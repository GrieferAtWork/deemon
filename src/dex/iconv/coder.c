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
#ifndef GUARD_DEX_ICONV_CODER_C
#define GUARD_DEX_ICONV_CODER_C 1
#define CONFIG_BUILDING_LIBICONV
#define DEE_SOURCE

#include "libiconv.h"
/**/

#include <deemon/api.h>

#include <deemon/alloc.h>       /* Dee_TYPE_CONSTRUCTOR_INIT_FIXED */
#include <deemon/arg.h>         /* DeeArg_UnpackStructKw */
#include <deemon/bool.h>        /* return_bool */
#include <deemon/error.h>       /* DeeError_RuntimeError, DeeError_Throwf */
#include <deemon/file.h>        /* DeeFileObject, DeeFileTypeObject, DeeFileType_Type, DeeFile_Type, DeeFile_WriteAll, Dee_ioflag_t */
#include <deemon/format.h>      /* DeeFormat_PRINT, DeeFormat_Printf, PRFxN */
#include <deemon/int.h>         /* DeeInt_NEWU */
#include <deemon/object.h>      /* DREF, DeeObject, Dee_Decref, Dee_Incref, Dee_TYPE, Dee_formatprinter_t, Dee_ssize_t, OBJECT_HEAD_INIT */
#include <deemon/string.h>      /* DeeString_New */
#include <deemon/type.h>        /* DeeType_GetName, Dee_TYPE_CONSTRUCTOR_INIT_FIXED, Dee_Visit, Dee_visit_t, STRUCT_OBJECT, TF_NONE, TP_FNORMAL, TYPE_*, type_getset, type_member */
#include <deemon/util/nrlock.h> /* Dee_NRLOCK_ALREADY, Dee_NRLOCK_ERR, Dee_nrshared_lock_* */

#include <hybrid/typecore.h> /* __SIZEOF_INTPTR_HALF_T__ */

#include <stdbool.h> /* bool */
#include <stddef.h>  /* NULL, offsetof, size_t */

DECL_BEGIN

#define DOC_param_errors     "#perrors{One of $\"strict\", $\"replace\", $\"ignore\" or $\"discard\"}"
#define DOC_param_closeout   "#pcloseout{When ?t, propagate ${operator sync} and ${operator close} to @out}"
#define DOC_param_closeinput "#pcloseinput{When ?t, propagate ${operator close} to @input}"
#define DOC_param_chunksize  "#pchunksize{Hint for how many bytes to read from @input at a time}"

/*[[[deemon
(print_DEFINE_KWLIST from rt.gen.unpack)({
	"codec",
	"out",
	"errors",
	"closeout",
});
(print_DEFINE_KWLIST from rt.gen.unpack)({
	"codec",
	"input",
	"errors",
	"closeinput",
	"chunksize",
});
]]]*/
#ifndef DEFINED_kwlist__codec_out_errors_closeout
#define DEFINED_kwlist__codec_out_errors_closeout
PRIVATE DEFINE_KWLIST(kwlist__codec_out_errors_closeout, { KEX("codec", 0x91dfc790, 0x678d4474a4f58564), KEX("out", 0x20bcdfe4, 0xfc801ac012e9f722), KEX("errors", 0xd327c5ea, 0x88b9782b6de95122), KEX("closeout", 0xa788672d, 0x6a93c4deffab5326), KEND });
#endif /* !DEFINED_kwlist__codec_out_errors_closeout */
#ifndef DEFINED_kwlist__codec_input_errors_closeinput_chunksize
#define DEFINED_kwlist__codec_input_errors_closeinput_chunksize
PRIVATE DEFINE_KWLIST(kwlist__codec_input_errors_closeinput_chunksize, { KEX("codec", 0x91dfc790, 0x678d4474a4f58564), KEX("input", 0xa0556672, 0x9532499416effacb), KEX("errors", 0xd327c5ea, 0x88b9782b6de95122), KEX("closeinput", 0x90ae92c4, 0xfc11aaae8d08ecc9), KEX("chunksize", 0xd5f3e634, 0x45395a4186f3ceef), KEND });
#endif /* !DEFINED_kwlist__codec_input_errors_closeinput_chunksize */
/*[[[end]]]*/

#define EDO(err, expr)                   \
	do {                                 \
		if unlikely((temp = (expr)) < 0) \
			goto err;                    \
		result += temp;                  \
	}	__WHILE0


PRIVATE WUNUSED NONNULL((3)) Dee_ssize_t DCALL
printcodecname(iconv_codec_t codec, uintptr_half_t flags,
               Dee_formatprinter_t printer, void *arg) {
	Dee_ssize_t temp, result;
	char const *name = libiconv_getcodecnames(codec);
	if unlikely(!name)
		return DeeFormat_Printf(printer, arg, "%u", (unsigned int)codec);
	result = DeeFormat_Printf(printer, arg, "\"%#q", name);
	if unlikely(result < 0)
		goto done;
	if (flags & ICONV_ERR_TRANSLIT)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "//TRANSLIT"));
	if ((flags & ICONV_ERRMASK) == ICONV_ERR_IGNORE)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, "//IGNORE"));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, "\""));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((3)) Dee_ssize_t DCALL
printcodecerrors(uintptr_half_t flags, Dee_formatprinter_t printer, void *arg) {
	switch (flags) {
	case ICONV_ERR_ERROR:
		return DeeFormat_PRINT(printer, arg, "\"strict\"");
	case ICONV_ERR_DISCARD:
		return DeeFormat_PRINT(printer, arg, "\"discard\"");
	case ICONV_ERR_REPLACE:
		return DeeFormat_PRINT(printer, arg, "\"replace\"");
	case ICONV_ERR_IGNORE:
		return DeeFormat_PRINT(printer, arg, "\"ignore\"");
	default: break;
	}
	return DeeFormat_Printf(printer, arg, "%#" PRFxN(__SIZEOF_INTPTR_HALF_T__), flags);
}

PRIVATE WUNUSED DREF DeeObject *DCALL
get_codec_name(iconv_codec_t codec) {
	char const *name = libiconv_getcodecnames(codec);
	if (name)
		return DeeString_New(name);
	return DeeInt_NEWU(codec);
}


/************************************************************************/
/* ENCODER                                                              */
/************************************************************************/

#define encoder_release(self) Dee_nrshared_lock_release(&(self)->ive_lock)
PRIVATE WUNUSED NONNULL((1)) int DCALL
encoder_acquire(IconvEncoder *__restrict self) {
	int error = Dee_nrshared_lock_acquire(&self->ive_lock);
	if (error == Dee_NRLOCK_ALREADY) {
		DeeError_Throwf(&DeeError_RuntimeError,
		                "Reentrant calls to `%s.write' are not allowed",
		                DeeType_GetName(&Dee_TYPE(self)->ft_base));
		error = Dee_NRLOCK_ERR;
	}
	return error;
}

PRIVATE NONNULL((1)) void DCALL
ive_fini(IconvEncoder *__restrict self) {
	Dee_Decref((DREF DeeFileObject *)self->ive_encoder.ice_output.ii_arg);
}

PRIVATE NONNULL((1, 2)) void DCALL
ive_visit(IconvEncoder *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit((DREF DeeFileObject *)self->ive_encoder.ice_output.ii_arg);
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ive_init(IconvEncoder *__restrict self, size_t argc,
         DeeObject *const *argv, DeeObject *kw) {
	iconv_codec_t codec_id;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("Encoder", params: """
	codec:?X2?Dstring?Dint,
	out:?DFile,
	errors:?X2?Dstring?Dint = NULL = !Pstrict,
	bool closeout = false
""", docStringPrefix: "ivd");]]]*/
#define ivd_Encoder_params "codec:?X2?Dstring?Dint,out:?DFile,errors:?X2?Dstring?Dint=!Pstrict,closeout=!f"
	struct {
		DeeObject *codec;
		DeeObject *out;
		DeeObject *errors;
		bool closeout;
	} args;
	args.errors = NULL;
	args.closeout = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__codec_out_errors_closeout, "oo|ob:Encoder", &args))
		goto err;
/*[[[end]]]*/
	self->ive_closeout = args.closeout;
	codec_id = deemon_iconv_parse_codec_name_and_error_mode(args.codec, args.errors,
	                                                        &self->ive_encoder.ice_flags);
	if unlikely(codec_id == ICONV_CODEC_UNKNOWN)
		goto err;
	self->ive_encoder.ice_output.ii_printer = (Dee_formatprinter_t)&DeeFile_WriteAll;
	self->ive_encoder.ice_output.ii_arg     = args.out;
	self->ive_encoder.ice_codec = codec_id;
	if unlikely(do_libiconv_encode_init(&self->ive_encoder, &self->ive_input))
		goto no_such_codec;
	Dee_nrshared_lock_init(&self->ive_lock);
	ASSERT(self->ive_encoder.ice_output.ii_arg == args.out);
	Dee_Incref(args.out);
	return 0;
no_such_codec:
	err_unknown_codec(codec_id);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
ive_printrepr(IconvEncoder *__restrict self,
              Dee_formatprinter_t printer, void *arg) {
	uintptr_half_t flags;
	Dee_ssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "Encoder(codec: ");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, printcodecname(self->ive_encoder.ice_codec, self->ive_encoder.ice_flags, printer, arg));
	EDO(err_temp, DeeFormat_Printf(printer, arg, ", out: %r", self->ive_encoder.ice_output.ii_arg));
	flags = self->ive_encoder.ice_flags;
	flags &= ~ICONV_ERR_TRANSLIT; /* Already printed as part of codec name */
	if (flags != ICONV_ERR_ERROR) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ", errors: "));
		EDO(err_temp, printcodecerrors(flags, printer, arg));
	}
	if (self->ive_closeout)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ", closeout: true"));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL
ive_write(IconvEncoder *self, void const *buffer,
          size_t bufsize, Dee_ioflag_t flags) {
	Dee_ssize_t result;
	(void)flags;
	if (encoder_acquire(self))
		goto err;
	result = (*self->ive_input.ii_printer)(self->ive_input.ii_arg, (char const *)buffer, bufsize);
	if (result < 0) {
		if (self->ive_encoder.ice_flags & ICONV_HASERR) {
			size_t offset;
			iconv_codec_t codec = self->ive_encoder.ice_codec;
			encoder_release(self);
			offset = (size_t)(bufsize + result);
			err_unicode_encode_error(codec, offset);
			goto err;
		}
		ASSERTF(result == -1, "Underlying printer should be `DeeFile_WriteAll', "
		                      "which should only ever return `-1' on error");
	}
	encoder_release(self);
	/* Return the full "bufsize" instead of "result": "result" is actual the #
	 * of produced **output** bytes, which can (easily) differ from "bufsize". */
	return bufsize;
err:
	return (size_t)-1;
}

DOC_DEF(ive_get_isinputshiftzero_doc,
        "->?Dbool\n"
        "Check if UTF-8 input taken by the given encoder is in its default (zero) shift "
        /**/ "state. If it isn't, then that must mean that it's still waiting for more UTF-8 "
        /**/ "data to arrive, and that you should either feed it said data, or deal with the "
        /**/ "fact that there's something missing in your input.\n"
        "WARNING: This function #B{DOESN'T} work when the given encoder is targeting UTF-8. "
        /**/ "This is because special optimizations are performed when encoding UTF-8 "
        /**/ "(since encoder also always takes UTF-8 as input). In this case, this "
        /**/ "function will always return !N");

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ive_get_isinputshiftzero(IconvEncoder *__restrict self) {
	bool result;
	if (encoder_acquire(self))
		goto err;
	result = libiconv_encode_isinputshiftzero(&self->ive_encoder);
	encoder_release(self);
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ive_sync_impl(IconvEncoder *__restrict self) {
	Dee_ssize_t status;
	if unlikely(encoder_acquire(self))
		goto err;
	status = libiconv_encode_flush(&self->ive_encoder);
	encoder_release(self);
	ASSERTF(status == -1 || status >= 0,
	        "The underlying printer is `DeeFile_WriteAll', "
	        "which should only ever return `-1' on error");
	if unlikely(status < 0)
		goto err;
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ive_close(IconvEncoder *__restrict self) {
	if (ive_sync_impl(self))
		goto err;
	if (self->ive_closeout)
		return DeeFile_Close(Dee_AsObject(IconvEncoder_GetOut(self)));
	return 0;
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ive_sync(IconvEncoder *__restrict self) {
	if (ive_sync_impl(self))
		goto err;
	if (self->ive_closeout)
		return DeeFile_Sync(Dee_AsObject(IconvEncoder_GetOut(self)));
	return 0;
err:
	return -1;
}


PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ive_getcodec(IconvEncoder *__restrict self) {
	iconv_codec_t codec = self->ive_encoder.ice_codec;
	return get_codec_name(codec);
}

PRIVATE struct type_getset tpconst ive_getsets[] = {
	TYPE_GETTER_AB("isinputshiftzero", &ive_get_isinputshiftzero,
	               DOC_GET(ive_get_isinputshiftzero_doc)),
	TYPE_GETTER_AB("codec", &ive_getcodec, "->?X2?Dstring?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst ive_members[] = {
	TYPE_MEMBER_FIELD_DOC("out", STRUCT_OBJECT,
	                      offsetof(IconvEncoder, ive_encoder.ice_output.ii_arg),
	                      "->?DFile"),
	TYPE_MEMBER_FIELD("closeout", STRUCT_ATOMIC | STRUCT_BOOL(__SIZEOF_BOOL__),
	                  offsetof(IconvEncoder, ive_closeout)),
	TYPE_MEMBER_END
};

INTERN DeeFileTypeObject IconvEncoder_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "Encoder",
		/* .tp_doc      = */ DOC("(" ivd_Encoder_params ")"
		                     "#pcodec{Name of the codec that should be used to encode @data. May "
		                     /*   */ "also contain some additional flags (s.a. ?Gparsecodecname)}"
		                     DOC_param_errors
		                     DOC_param_closeout
		                     ""),
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ &DeeFile_Type.ft_base,
		/* .tp_init = */ {
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
				/* T:              */ IconvEncoder,
				/* tp_ctor:        */ NULL,
				/* tp_copy_ctor:   */ NULL,
				/* tp_any_ctor:    */ NULL,
				/* tp_any_ctor_kw: */ &ive_init,
				/* tp_serialize:   */ NULL /* Would be possible, but would be super-complicated since
				                            * it'd require per-codec handling not provided by `libiconv' */
			),
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ive_fini,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str       = */ NULL,
			/* .tp_repr      = */ NULL,
			/* .tp_bool      = */ NULL,
			/* .tp_print     = */ NULL,
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&ive_printrepr,
		},
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ive_visit,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_iterator      = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */ NULL,
		/* .tp_getsets       = */ ive_getsets,
		/* .tp_members       = */ ive_members,
		/* .tp_class_methods = */ NULL,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ NULL
	},
	/* .ft_read   = */ NULL,
	/* .ft_write  = */ (size_t (DCALL *)(DeeFileObject *, void const *, size_t, Dee_ioflag_t))&ive_write,
	/* .ft_seek   = */ NULL,
	/* .ft_sync   = */ (int (DCALL *)(DeeFileObject *))&ive_sync,
	/* .ft_trunc  = */ NULL,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *))&ive_close,
	/* .ft_pread  = */ NULL,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ NULL,
	/* .ft_ungetc = */ NULL,
	/* .ft_putc   = */ NULL
};






/************************************************************************/
/* DECODE-WRITER                                                        */
/************************************************************************/

STATIC_ASSERT(offsetof(IconvDecodeWriter, ivdw_lock) == offsetof(IconvEncoder, ive_lock));
#define ivdw_acquire(self) encoder_acquire((IconvEncoder *)(self))
#define ivdw_release(self) Dee_nrshared_lock_release(&(self)->ivdw_lock)

STATIC_ASSERT(offsetof(IconvDecodeWriter, ivdw_decoder.icd_output.ii_arg) ==
              offsetof(IconvEncoder, ive_encoder.ice_output.ii_arg));
#define ivdw_fini    ive_fini
#define ivdw_visit   ive_visit
#define ivdw_members ive_members

PRIVATE WUNUSED NONNULL((1)) int DCALL
ivdw_init(IconvDecodeWriter *__restrict self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	iconv_codec_t codec_id;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("DecodeWriter", params: """
	codec:?X2?Dstring?Dint,
	out:?DFile,
	errors:?X2?Dstring?Dint = NULL = !Pstrict,
	bool closeout = false
""", docStringPrefix: "ivd");]]]*/
#define ivd_DecodeWriter_params "codec:?X2?Dstring?Dint,out:?DFile,errors:?X2?Dstring?Dint=!Pstrict,closeout=!f"
	struct {
		DeeObject *codec;
		DeeObject *out;
		DeeObject *errors;
		bool closeout;
	} args;
	args.errors = NULL;
	args.closeout = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__codec_out_errors_closeout, "oo|ob:DecodeWriter", &args))
		goto err;
/*[[[end]]]*/
	self->ivdw_closeout = args.closeout;
	codec_id = deemon_iconv_parse_codec_name_and_error_mode(args.codec, args.errors,
	                                                        &self->ivdw_decoder.icd_flags);
	if unlikely(codec_id == ICONV_CODEC_UNKNOWN)
		goto err;
	self->ivdw_decoder.icd_output.ii_printer = (Dee_formatprinter_t)&DeeFile_WriteAll;
	self->ivdw_decoder.icd_output.ii_arg     = args.out;
	self->ivdw_decoder.icd_codec = codec_id;
	if unlikely(do_libiconv_decode_init(&self->ivdw_decoder, &self->ivdw_input))
		goto no_such_codec;
	Dee_nrshared_lock_init(&self->ivdw_lock);
	ASSERT(self->ivdw_decoder.icd_output.ii_arg == args.out);
	Dee_Incref(args.out);
	return 0;
no_such_codec:
	err_unknown_codec(codec_id);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
ivdw_printrepr(IconvDecodeWriter *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
	uintptr_half_t flags;
	Dee_ssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "DecodeWriter(codec: ");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, printcodecname(self->ivdw_decoder.icd_codec, self->ivdw_decoder.icd_flags, printer, arg));
	EDO(err_temp, DeeFormat_Printf(printer, arg, ", out: %r", self->ivdw_decoder.icd_output.ii_arg));
	flags = self->ivdw_decoder.icd_flags;
	flags &= ~ICONV_ERR_TRANSLIT; /* Already printed as part of codec name */
	if (flags != ICONV_ERR_ERROR) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ", errors: "));
		EDO(err_temp, printcodecerrors(flags, printer, arg));
	}
	if (self->ivdw_closeout)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ", closeout: true"));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL
ivdw_write(IconvDecodeWriter *self, void const *buffer,
           size_t bufsize, Dee_ioflag_t flags) {
	Dee_ssize_t result;
	(void)flags;
	if (ivdw_acquire(self))
		goto err;
	result = (*self->ivdw_input.ii_printer)(self->ivdw_input.ii_arg, (char const *)buffer, bufsize);
	if (result < 0) {
		if (self->ivdw_decoder.icd_flags & ICONV_HASERR) {
			size_t offset;
			iconv_codec_t codec = self->ivdw_decoder.icd_codec;
			ivdw_release(self);
			offset = (size_t)(bufsize + result);
			err_unicode_decode_error(codec, offset);
			goto err;
		}
		ASSERTF(result == -1, "Underlying printer should be `DeeFile_WriteAll', "
		                      "which should only ever return `-1' on error");
	}
	ivdw_release(self);
	/* Return the full "bufsize" instead of "result": "result" is actual the #
	 * of produced **output** bytes, which can (easily) differ from "bufsize". */
	return bufsize;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ivdw_get_isshiftzero(IconvDecodeWriter *__restrict self) {
	bool result;
	if (ivdw_acquire(self))
		goto err;
	result = libiconv_decode_isshiftzero(&self->ivdw_decoder);
	ivdw_release(self);
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ivdw_close(IconvDecodeWriter *__restrict self) {
	if (self->ivdw_closeout)
		return DeeFile_Close(Dee_AsObject(IconvDecodeWriter_GetOut(self)));
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ivdw_sync(IconvDecodeWriter *__restrict self) {
	if (self->ivdw_closeout)
		return DeeFile_Sync(Dee_AsObject(IconvDecodeWriter_GetOut(self)));
	return 0;
}

STATIC_ASSERT(offsetof(IconvDecodeWriter, ivdw_decoder.icd_codec) ==
              offsetof(IconvEncoder, ive_encoder.ice_codec));
#define ivdw_getcodec ive_getcodec

DOC_DEF(ivdw_get_isshiftzero_doc,
        "->?Dbool\n"
        "Check if the given encoder is in its default (zero) shift state. If it isn't, "
        /**/ "then that must mean that it's still waiting for more input data to arrive, and "
        /**/ "that you should either feed it said data, or deal with the fact that there's "
        /**/ "something missing in your input.\n"
        "WARNING: This function #B{DOESN'T} work when the given decoder is used to parse "
        /**/ "UTF-8 input! This is because special optimizations are performed when "
        /**/ "decoding UTF-8 (since decoders also always output UTF-8). In this "
        /**/ "case this function will always return ?t");

PRIVATE struct type_getset tpconst ivdw_getsets[] = {
	TYPE_GETTER_AB("isshiftzero", &ivdw_get_isshiftzero, DOC_GET(ivdw_get_isshiftzero_doc)),
	TYPE_GETTER_AB("codec", &ivdw_getcodec, "->?X2?Dstring?Dint"),
	TYPE_GETSET_END
};

INTERN DeeFileTypeObject IconvDecodeWriter_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "DecodeWriter",
		/* .tp_doc      = */ DOC("(" ivd_DecodeWriter_params ")\n"
		                     "#pcodec{Name of the codec that was used to encode @data. May also "
		                     /*   */ "contain some additional flags (s.a. ?Gparsecodecname)}"
		                     DOC_param_errors
		                     DOC_param_closeout
		                     ""),
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ &DeeFile_Type.ft_base,
		/* .tp_init = */ {
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
				/* T:              */ IconvDecodeWriter,
				/* tp_ctor:        */ NULL,
				/* tp_copy_ctor:   */ NULL,
				/* tp_any_ctor:    */ NULL,
				/* tp_any_ctor_kw: */ &ivdw_init,
				/* tp_serialize:   */ NULL /* Would be possible, but would be super-complicated since
				                            * it'd require per-codec handling not provided by `libiconv' */
			),
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ivdw_fini,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str       = */ NULL,
			/* .tp_repr      = */ NULL,
			/* .tp_bool      = */ NULL,
			/* .tp_print     = */ NULL,
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&ivdw_printrepr,
		},
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ivdw_visit,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_iterator      = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */ NULL,
		/* .tp_getsets       = */ ivdw_getsets,
		/* .tp_members       = */ ivdw_members,
		/* .tp_class_methods = */ NULL,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ NULL
	},
	/* .ft_read   = */ NULL,
	/* .ft_write  = */ (size_t (DCALL *)(DeeFileObject *, void const *, size_t, Dee_ioflag_t))&ivdw_write,
	/* .ft_seek   = */ NULL,
	/* .ft_sync   = */ (int (DCALL *)(DeeFileObject *))&ivdw_sync,
	/* .ft_trunc  = */ NULL,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *))&ivdw_close,
	/* .ft_pread  = */ NULL,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ NULL,
	/* .ft_ungetc = */ NULL,
	/* .ft_putc   = */ NULL
};







/************************************************************************/
/* TRANSCODER                                                           */
/************************************************************************/

STATIC_ASSERT(offsetof(IconvTranscodeWriter, ivtw_lock) == offsetof(IconvEncoder, ive_lock));
#define ivtw_acquire(self) encoder_acquire((IconvEncoder *)(self))
#define ivtw_release(self) Dee_nrshared_lock_release(&(self)->ivtw_lock)
STATIC_ASSERT(offsetof(IconvTranscodeWriter, ivtw_encoder.ice_output.ii_arg) ==
              offsetof(IconvEncoder, ive_encoder.ice_output.ii_arg));
#define ivtw_fini    ive_fini
#define ivtw_visit   ive_visit
#define ivtw_members ive_members

STATIC_ASSERT(offsetof(IconvTranscodeWriter, ivtw_encoder) == offsetof(IconvEncoder, ive_encoder));
STATIC_ASSERT(offsetof(IconvTranscodeWriter, ivtw_closeout) == offsetof(IconvEncoder, ive_closeout));
#define ivtw_close       ive_close
#define ivtw_sync        ive_sync
#define ivtw_getoutcodec ive_getcodec

PRIVATE WUNUSED NONNULL((1)) int DCALL
ivtw_init(IconvTranscodeWriter *__restrict self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	iconv_codec_t in_codec_id;
	iconv_codec_t out_codec_id;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("TranscodeWriter", params: """
	incodec:?X2?Dstring?Dint,
	outcodec:?X2?Dstring?Dint,
	out:?DFile,
	errors:?X2?Dstring?Dint = NULL = !Pstrict,
	bool closeout = false
""", docStringPrefix: "ivd", defineKwList: true);]]]*/
	static DEFINE_KWLIST(TranscodeWriter_kwlist, { KEX("incodec", 0x556c3790, 0x67f1690ff39bd316), KEX("outcodec", 0x2f41262c, 0xfb14bcb3b213f6d6), KEX("out", 0x20bcdfe4, 0xfc801ac012e9f722), KEX("errors", 0xd327c5ea, 0x88b9782b6de95122), KEX("closeout", 0xa788672d, 0x6a93c4deffab5326), KEND });
#define ivd_TranscodeWriter_params "incodec:?X2?Dstring?Dint,outcodec:?X2?Dstring?Dint,out:?DFile,errors:?X2?Dstring?Dint=!Pstrict,closeout=!f"
	struct {
		DeeObject *incodec;
		DeeObject *outcodec;
		DeeObject *out;
		DeeObject *errors;
		bool closeout;
	} args;
	args.errors = NULL;
	args.closeout = false;
	if (DeeArg_UnpackStructKw(argc, argv, kw, TranscodeWriter_kwlist, "ooo|ob:TranscodeWriter", &args))
		goto err;
/*[[[end]]]*/
	self->ivtw_closeout = args.closeout;
	in_codec_id = deemon_iconv_parse_codec_name_and_error_mode(args.incodec, args.errors,
	                                                           &self->ivtw_decoder.icd_flags);
	if unlikely(in_codec_id == ICONV_CODEC_UNKNOWN)
		goto err;
	out_codec_id = deemon_iconv_parse_codec_name_and_error_mode(args.outcodec, args.errors,
	                                                            &self->ivtw_encoder.ice_flags);
	if unlikely(out_codec_id == ICONV_CODEC_UNKNOWN)
		goto err;
	self->ivtw_encoder.ice_output.ii_printer = (Dee_formatprinter_t)&DeeFile_WriteAll;
	self->ivtw_encoder.ice_output.ii_arg     = args.out;
	self->ivtw_encoder.ice_codec = out_codec_id;
	self->ivtw_decoder.icd_codec = in_codec_id;

	/* Check for special case: when input and output  codecs are the same,  then
	 *                         it really shouldn't matter if we don't know them! */
	if (self->ivtw_decoder.icd_codec == self->ivtw_encoder.ice_codec) {
		self->ivtw_input = self->ivtw_encoder.ice_output;
		self->ivtw_encoder.ice_codec = CODEC_UNKNOWN;
	} else {
		/* Initialize the encoder and set-up its input pipe for use as output by the decoder. */
		if unlikely(do_libiconv_encode_init(&self->ivtw_encoder, &self->ivtw_decoder.icd_output))
			goto no_such_out_codec;

		/* Initialize the decoder (note that it's output printer was already set-up
		 * as the input descriptor for the  encode function in the previous  step!) */
		if (do_libiconv_decode_init(&self->ivtw_decoder, &self->ivtw_input))
			goto no_such_in_codec;

		/* And that's already it! */
	}

	Dee_nrshared_lock_init(&self->ivtw_lock);
	ASSERT(self->ivtw_encoder.ice_output.ii_arg == args.out);
	Dee_Incref(args.out);
	return 0;
no_such_out_codec:
	err_unknown_codec(out_codec_id);
	goto err;
no_such_in_codec:
	err_unknown_codec(in_codec_id);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
ivtw_printrepr(IconvTranscodeWriter *__restrict self,
               Dee_formatprinter_t printer, void *arg) {
	uintptr_half_t flags;
	Dee_ssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "TranscodeWriter(incodec: ");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, printcodecname(self->ivtw_decoder.icd_codec, self->ivtw_decoder.icd_flags, printer, arg));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ", outcodec: "));
	EDO(err_temp, printcodecname(self->ivtw_encoder.ice_codec, self->ivtw_encoder.ice_flags, printer, arg));
	EDO(err_temp, DeeFormat_Printf(printer, arg, ", out: %r", self->ivtw_encoder.ice_output.ii_arg));
	flags = self->ivtw_decoder.icd_flags | self->ivtw_encoder.ice_flags;
	flags &= ~ICONV_ERR_TRANSLIT; /* Already printed as part of codec name */
	if (flags != ICONV_ERR_ERROR) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ", errors: "));
		EDO(err_temp, printcodecerrors(flags, printer, arg));
	}
	if (self->ivtw_closeout)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ", closeout: true"));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err_temp:
	return temp;
}

PRIVATE WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL
ivtw_write(IconvTranscodeWriter *self, void const *buffer,
           size_t bufsize, Dee_ioflag_t flags) {
	Dee_ssize_t result;
	(void)flags;
	if (ivtw_acquire(self))
		goto err;
	result = (*self->ivtw_input.ii_printer)(self->ivtw_input.ii_arg, (char const *)buffer, bufsize);
	if (result < 0) {
		if (self->ivtw_decoder.icd_flags & ICONV_HASERR) {
			size_t offset;
			iconv_codec_t incodec = self->ivtw_decoder.icd_codec;
			ivtw_release(self);
			offset = (size_t)(bufsize + result);
			err_unicode_decode_error(incodec, offset);
			goto err;
		} else if (self->ivtw_encoder.ice_flags & ICONV_HASERR) {
			iconv_codec_t outcodec = self->ivtw_decoder.icd_codec;
			ivtw_release(self);
			err_unicode_reencode_error(outcodec);
			goto err;
		}
		ASSERTF(result == -1, "Underlying printer should be `DeeFile_WriteAll', "
		                      "which should only ever return `-1' on error");
	}
	ivtw_release(self);
	/* Return the full "bufsize" instead of "result": "result" is actual the #
	 * of produced **output** bytes, which can (easily) differ from "bufsize". */
	return bufsize;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ivtw_get_isshiftzero(IconvTranscodeWriter *__restrict self) {
	bool result;
	if (ivtw_acquire(self))
		goto err;
	result = libiconv_decode_isshiftzero(&self->ivtw_decoder);
	ivtw_release(self);
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ivtw_getincodec(IconvTranscodeWriter *__restrict self) {
	iconv_codec_t codec = self->ivtw_encoder.ice_codec;
	return get_codec_name(codec);
}


#define ivtw_get_isshiftzero_doc ivdw_get_isshiftzero_doc
PRIVATE struct type_getset tpconst ivtw_getsets[] = {
	TYPE_GETTER_AB("isshiftzero", &ivtw_get_isshiftzero, DOC_GET(ivtw_get_isshiftzero_doc)),
	TYPE_GETTER_AB("incodec", &ivtw_getincodec, "->?X2?Dstring?Dint"),
	TYPE_GETTER_AB("outcodec", &ivtw_getoutcodec, "->?X2?Dstring?Dint"),
	TYPE_GETSET_END
};

INTERN DeeFileTypeObject IconvTranscodeWriter_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "TranscodeWriter",
		/* .tp_doc      = */ DOC("(" ivd_TranscodeWriter_params ")\n"
		                         "#pincodec{Name of the codec that was used to encode @data. May also "
		                         /*     */ "contain some additional flags (s.a. ?Gparsecodecname)}"
		                         "#poutcodec{Name of the codec that should be used to re-encode @data "
		                         /*      */ "after it was internally decoded using @incodec. May also "
		                         /*      */ "contain some additional flags (s.a. ?Gparsecodecname)}"
		                         DOC_param_errors
		                         DOC_param_closeout
		                         ""),
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ &DeeFile_Type.ft_base,
		/* .tp_init = */ {
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
				/* T:              */ IconvTranscodeWriter,
				/* tp_ctor:        */ NULL,
				/* tp_copy_ctor:   */ NULL,
				/* tp_any_ctor:    */ NULL,
				/* tp_any_ctor_kw: */ &ivtw_init,
				/* tp_serialize:   */ NULL /* Would be possible, but would be super-complicated since
				                            * it'd require per-codec handling not provided by `libiconv' */
			),
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ivtw_fini,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str       = */ NULL,
			/* .tp_repr      = */ NULL,
			/* .tp_bool      = */ NULL,
			/* .tp_print     = */ NULL,
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&ivtw_printrepr,
		},
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ivtw_visit,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_iterator      = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */ NULL,
		/* .tp_getsets       = */ ivtw_getsets,
		/* .tp_members       = */ ivtw_members,
		/* .tp_class_methods = */ NULL,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ NULL
	},
	/* .ft_read   = */ NULL,
	/* .ft_write  = */ (size_t (DCALL *)(DeeFileObject *, void const *, size_t, Dee_ioflag_t))&ivtw_write,
	/* .ft_seek   = */ NULL,
	/* .ft_sync   = */ (int (DCALL *)(DeeFileObject *))&ivtw_sync,
	/* .ft_trunc  = */ NULL,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *))&ivtw_close,
	/* .ft_pread  = */ NULL,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ NULL,
	/* .ft_ungetc = */ NULL,
	/* .ft_putc   = */ NULL
};





/************************************************************************/
/* DECODER                                                              */
/************************************************************************/

#define IVD_DEFAULT_CHUNKSIZE 4096

STATIC_ASSERT(offsetof(IconvDecoder, ivd_lock) == offsetof(IconvEncoder, ive_lock));
#define ivd_acquire(self) encoder_acquire((IconvEncoder *)(self))
#define ivd_release(self) Dee_nrshared_lock_release(&(self)->ivd_lock)

PRIVATE WUNUSED NONNULL((1)) Dee_ssize_t DCALL
ivd_buffer_printer(void *arg, char const *__restrict data, size_t datalen) {
	IconvDecoder *me = (IconvDecoder *)arg;
	byte_t *pnd_end = me->ivd_pndbase + me->ivd_pndsize;
	byte_t *buf_end = me->ivd_bufbase + me->ivd_bufsize;
	size_t avail = (size_t)(buf_end - pnd_end);
	ASSERT(buf_end >= pnd_end);
	ASSERT(me->ivd_pndbase >= me->ivd_bufbase);
	if (avail < datalen) {
		/* Remaining buffer space too small -> try to reclaim leading space */
		if (me->ivd_pndbase > me->ivd_bufbase) {
			me->ivd_pndbase = (byte_t *)memmovedown(me->ivd_bufbase, me->ivd_pndbase, me->ivd_pndsize);
			pnd_end = me->ivd_pndbase + me->ivd_pndsize;
			ASSERT(buf_end >= pnd_end);
			avail = (size_t)(buf_end - pnd_end);
		}

		/* Buffer is just too small; period. -> Must resize to make it larger */
		if (avail < datalen) {
			byte_t *new_buffer;
			size_t min_alloc = me->ivd_pndsize + datalen;
			size_t new_alloc = me->ivd_bufsize << 1;
			if (new_alloc < 512)
				new_alloc = 512;
			if (new_alloc < min_alloc)
				new_alloc = min_alloc;
			new_buffer = (byte_t *)Dee_TryRealloc(me->ivd_bufbase, new_alloc);
			if unlikely(!new_buffer) {
				new_alloc = min_alloc;
				new_buffer = (byte_t *)Dee_Realloc(me->ivd_bufbase, new_alloc);
				if unlikely(!new_buffer)
					goto err;
			}
			me->ivd_bufbase = new_buffer;
			me->ivd_pndbase = new_buffer;
			me->ivd_bufsize = new_alloc;
			pnd_end = me->ivd_pndbase + me->ivd_pndsize;
			buf_end = me->ivd_bufbase + me->ivd_bufsize;
			avail = (size_t)(buf_end - pnd_end);
		}
	}
	ASSERT(avail >= datalen);
	ASSERT(pnd_end == me->ivd_pndbase + me->ivd_pndsize);

	/* Copy data into buffer */
	memcpy(pnd_end, data, datalen);
	me->ivd_pndsize += datalen;

	return (Dee_ssize_t)datalen;
err:
	return -1;
}

/* Fill decoded data buffer by reading from input file.
 * NOTE: Caller must be holding a lock to `self->ivd_lock'
 *
 * @param: hint: Hint for how many bytes to read from input file
 * @return: 0 :  Input file indicates EOF (no further data was decoded)
 * @return: * :  This many bytes were read from input file
 * @return: (size_t)-1: An error was thrown. */
PRIVATE WUNUSED NONNULL((1)) size_t DCALL
ivd_fillbuffer_locked(IconvDecoder *__restrict self,
                      size_t hint, Dee_ioflag_t flags) {
	size_t result;
	byte_t *temp;
	if (hint > self->ivd_chnksiz) {
		hint = self->ivd_chnksiz;
		if unlikely(!hint)
			hint = 1;
	}
	temp = (byte_t *)Dee_Malloca(hint);
	if unlikely(!temp)
		goto err;

	/* Read data into "temp" buffer */
	result = DeeFile_Readf(self->ivd_in, temp, hint, flags);
	if likely(result != (size_t)-1 &&
	          result != (size_t)0) {
		/* Feed data into input printer. */
		Dee_ssize_t status;
		status = (*self->ivd_input.ii_printer)(self->ivd_input.ii_arg,
		                                       (char const *)temp, result);
		if (status < 0) {
			if (self->ivd_decoder.icd_flags & ICONV_HASERR) {
				iconv_codec_t codec = self->ivd_decoder.icd_codec;
				Dee_pos_t fpos = DeeFile_Tell(self->ivd_in);
				if unlikely(fpos == (Dee_pos_t)-1)
					goto err_temp;
				fpos += (Dee_off_t)status;
				if (Dee_TYPE(self) == &IconvDecoder_Type) {
					err_unicode_decode_error(codec, fpos);
				} else {
					err_unicode_encode_error(codec, fpos);
				}
				goto err_temp;
			}
			ASSERTF(result == -1,
			        "Underlying printer should be `ivd_buffer_printer', "
			        "which only return `-1' on error");
		}
	}
	Dee_Freea(temp);
	return result;
err_temp:
	Dee_Freea(temp);
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) size_t DCALL
ivd_fillbuffer_any_locked(IconvDecoder *__restrict self,
                          size_t hint, Dee_ioflag_t flags) {
	size_t result, temp;
	result = ivd_fillbuffer_locked(self, hint, flags);
	if (result == 0) {
maybe_sync_on_eof:
		if (Dee_TYPE(self) == &IconvEncodeReader_Type) {
			IconvEncodeReader *me = (IconvEncodeReader *)self;
			if (libiconv_encode_flush(&me->iver_encoder) == -1)
				goto err;
#if 0 /* TODO */
		} else if (Dee_TYPE(self) == &IconvTranscodeReader_Type) {
			IconvTranscodeReader *me = (IconvTranscodeReader *)self;
			/* TODO */
#endif
		}
	} else if likely(result != (size_t)-1) {
		/* If the underlying file-read didn't do anything (i.e.:
		 * all read data was simply discarded by the decoder),
		 * try to read some more data... */
		while unlikely(self->ivd_pndsize == 0) {
			hint *= 2;
			if unlikely(!hint)
				hint = IVD_DEFAULT_CHUNKSIZE;
			temp = ivd_fillbuffer_locked(self, hint, flags);
			if unlikely(temp == (size_t)-1) {
				result = (size_t)-1;
				break;
			}
			if (temp == 0) {
				/* EOF (but check if we're an "EncodeReader" or "TranscodeReader",
				 *      in which case we may still be able to produce some more
				 *      output by flushing the encoder so it resets itself) */
				goto maybe_sync_on_eof;
			}
			result += temp;
		}
	}
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ivd_init(IconvDecoder *__restrict self, size_t argc,
         DeeObject *const *argv, DeeObject *kw) {
	iconv_codec_t codec_id;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("Decoder", params: """
	codec:?X2?Dstring?Dint,
	input:?DFile,
	errors:?X2?Dstring?Dint = NULL = !Pstrict,
	bool closeinput = false,
	size_t chunksize = IVD_DEFAULT_CHUNKSIZE = !4096
""", docStringPrefix: "ivd");]]]*/
#define ivd_Decoder_params "codec:?X2?Dstring?Dint,input:?DFile,errors:?X2?Dstring?Dint=!Pstrict,closeinput=!f,chunksize=!4096"
	struct {
		DeeObject *codec;
		DeeObject *input;
		DeeObject *errors;
		bool closeinput;
		size_t chunksize;
	} args;
	args.errors = NULL;
	args.closeinput = false;
	args.chunksize = IVD_DEFAULT_CHUNKSIZE;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__codec_input_errors_closeinput_chunksize, "oo|ob" UNPuSIZ ":Decoder", &args))
		goto err;
/*[[[end]]]*/
	self->ivd_closein = args.closeinput;
	self->ivd_chnksiz = args.chunksize;
	codec_id = deemon_iconv_parse_codec_name_and_error_mode(args.codec, args.errors,
	                                                        &self->ivd_decoder.icd_flags);
	if unlikely(codec_id == ICONV_CODEC_UNKNOWN)
		goto err;
	self->ivd_decoder.icd_output.ii_printer = &ivd_buffer_printer;
	self->ivd_decoder.icd_output.ii_arg     = self;
	self->ivd_decoder.icd_codec = codec_id;
	if unlikely(do_libiconv_decode_init(&self->ivd_decoder, &self->ivd_input))
		goto no_such_codec;
	Dee_nrshared_lock_init(&self->ivd_lock);
	ASSERT(self->ivd_decoder.icd_output.ii_arg == self);
	Dee_Incref(args.input);
	self->ivd_in = args.input;
	self->ivd_bufbase = NULL;
	self->ivd_bufsize = 0;
	self->ivd_pndbase = NULL;
	self->ivd_pndsize = 0;
	return 0;
no_such_codec:
	err_unknown_codec(codec_id);
err:
	return -1;
}

PRIVATE NONNULL((1)) void DCALL
ivd_fini(IconvDecoder *__restrict self) {
	Dee_Free(self->ivd_bufbase);
	Dee_Decref(self->ivd_in);
}

PRIVATE NONNULL((1, 2)) void DCALL
ivd_visit(IconvDecoder *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit(self->ivd_in);
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
ivd_printrepr(IconvDecoder *__restrict self, Dee_formatprinter_t printer, void *arg) {
	uintptr_half_t flags;
	Dee_ssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "Decoder(codec: ");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, printcodecname(self->ivd_decoder.icd_codec, self->ivd_decoder.icd_flags, printer, arg));
	EDO(err_temp, DeeFormat_Printf(printer, arg, ", input: %r", self->ivd_in));
	flags = self->ivd_decoder.icd_flags;
	flags &= ~ICONV_ERR_TRANSLIT; /* Already printed as part of codec name */
	if (flags != ICONV_ERR_ERROR) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ", errors: "));
		EDO(err_temp, printcodecerrors(flags, printer, arg));
	}
	if (self->ivd_closein)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ", closeinput: true"));
	if (self->ivd_chnksiz != IVD_DEFAULT_CHUNKSIZE)
		EDO(err_temp, DeeFormat_Printf(printer, arg, ", chunksize: %" PRFuSIZ, self->ivd_chnksiz));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err_temp:
	return temp;
}

#define ivd_get_isshiftzero_doc ivdw_get_isshiftzero_doc
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ivd_get_isshiftzero(IconvDecoder *__restrict self) {
	bool result;
	if (ivd_acquire(self))
		goto err;
	result = libiconv_decode_isshiftzero(&self->ivd_decoder);
	ivd_release(self);
	return_bool(result);
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL
ivd_read(IconvDecoder *self, void *buffer,
         size_t bufsize, Dee_ioflag_t flags) {
	size_t result = 0;
	(void)flags;
	if (ivd_acquire(self))
		goto err;
	while (bufsize) {
		size_t temp = self->ivd_pndsize;
		if (temp > bufsize)
			temp = bufsize;
		buffer = mempcpy(buffer, self->ivd_pndbase, temp);
		result += temp;
		bufsize -= temp;
		self->ivd_pndbase += temp;
		self->ivd_pndsize -= temp;
		if (result != 0)
			break; /* Got something! */
		ASSERT(!self->ivd_pndsize);
		temp = ivd_fillbuffer_any_locked(self, bufsize, flags);
		if unlikely(temp == (size_t)-1) {
			result = (size_t)-1;
			break;
		}
		if (temp == 0 && self->ivd_pndsize == 0)
			break; /* EOF */
	}
	ivd_release(self);
	return result;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ivd_close(IconvDecoder *__restrict self) {
	if (self->ivd_closein)
		return DeeFile_Close(self->ivd_in);
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) int DCALL
ivd_sync(IconvDecoder *__restrict self) {
	(void)self;
#if 0 /* Nope: not propagated! */
	if (self->ivd_closein)
		return DeeFile_Sync(self->ivd_in);
#endif
	return 0;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ivd_getcodec(IconvDecoder *__restrict self) {
	iconv_codec_t codec = self->ivd_decoder.icd_codec;
	return get_codec_name(codec);
}


PRIVATE struct type_getset tpconst ivd_getsets[] = {
	TYPE_GETTER_AB("isshiftzero", &ivd_get_isshiftzero, DOC_GET(ivd_get_isshiftzero_doc)),
	TYPE_GETTER_AB("codec", &ivd_getcodec, "->?X2?Dstring?Dint"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst ivd_members[] = {
	TYPE_MEMBER_FIELD_DOC("input", STRUCT_OBJECT, offsetof(IconvDecoder, ivd_in), "->?DFile"),
	TYPE_MEMBER_FIELD("closeinput", STRUCT_ATOMIC | STRUCT_BOOL(__SIZEOF_BOOL__),
	                  offsetof(IconvDecoder, ivd_closein)),
	TYPE_MEMBER_FIELD("chunksize", STRUCT_ATOMIC | STRUCT_SIZE_T,
	                  offsetof(IconvDecoder, ivd_chnksiz)),
	TYPE_MEMBER_FIELD_DOC("__pending_bytes__", STRUCT_CONST | STRUCT_SIZE_T,
	                      offsetof(IconvDecoder, ivd_pndsize),
	                      "Number of decoded, but not-yet-read bytes"),
	TYPE_MEMBER_FIELD_DOC("__buffer_size__", STRUCT_CONST | STRUCT_SIZE_T,
	                      offsetof(IconvDecoder, ivd_bufsize),
	                      "Size of the internal decode buffer"),
	TYPE_MEMBER_END
};



INTERN DeeFileTypeObject IconvDecoder_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "Decoder",
		/* .tp_doc      = */ DOC("(" ivd_Decoder_params ")\n"
		                         "#pcodec{Name of the codec that was used to encode @fp. May also "
		                         /*   */ "contain some additional flags (s.a. ?Gparsecodecname)}"
		                         "#pinput{The ?DFile that encoded data is read from and subsequently decoded}"
		                         DOC_param_errors
		                         DOC_param_closeinput
		                         DOC_param_chunksize
		                         ""),
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ &DeeFile_Type.ft_base,
		/* .tp_init = */ {
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
				/* T:              */ IconvDecoder,
				/* tp_ctor:        */ NULL,
				/* tp_copy_ctor:   */ NULL,
				/* tp_any_ctor:    */ NULL,
				/* tp_any_ctor_kw: */ &ivd_init,
				/* tp_serialize:   */ NULL /* Would be possible, but would be super-complicated since
				                            * it'd require per-codec handling not provided by `libiconv' */
			),
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ivd_fini,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str       = */ NULL,
			/* .tp_repr      = */ NULL,
			/* .tp_bool      = */ NULL,
			/* .tp_print     = */ NULL,
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&ivd_printrepr,
		},
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&ivd_visit,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_iterator      = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */ NULL,
		/* .tp_getsets       = */ ivd_getsets,
		/* .tp_members       = */ ivd_members,
		/* .tp_class_methods = */ NULL,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ NULL
	},
	/* .ft_read   = */ (size_t (DCALL *)(DeeFileObject *, void *, size_t, Dee_ioflag_t))&ivd_read,
	/* .ft_write  = */ NULL,
	/* .ft_seek   = */ NULL,
	/* .ft_sync   = */ (int (DCALL *)(DeeFileObject *))&ivd_sync,
	/* .ft_trunc  = */ NULL,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *))&ivd_close,
	/* .ft_pread  = */ NULL,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ NULL,
	/* .ft_ungetc = */ NULL,
	/* .ft_putc   = */ NULL
};






/************************************************************************/
/* ENCODE-READER                                                        */
/************************************************************************/

#define IVD_DEFAULT_CHUNKSIZE 4096

STATIC_ASSERT(offsetof(IconvEncodeReader, iver_lock) == offsetof(IconvEncoder, ive_lock));
#define iver_acquire(self) encoder_acquire((IconvEncoder *)(self))
#define iver_release(self) Dee_nrshared_lock_release(&(self)->iver_lock)

STATIC_ASSERT(offsetof(IconvEncodeReader, iver_bufbase) == offsetof(IconvDecoder, ivd_bufbase));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_bufsize) == offsetof(IconvDecoder, ivd_bufsize));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_pndbase) == offsetof(IconvDecoder, ivd_pndbase));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_pndsize) == offsetof(IconvDecoder, ivd_pndsize));
#define iver_buffer_printer ivd_buffer_printer

STATIC_ASSERT(offsetof(IconvEncodeReader, iver_in) == offsetof(IconvDecoder, ivd_in));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_bufbase) == offsetof(IconvDecoder, ivd_bufbase));
#define iver_fini  ivd_fini
#define iver_visit ivd_visit

STATIC_ASSERT(offsetof(IconvEncodeReader, iver_lock) == offsetof(IconvDecoder, ivd_lock));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_in) == offsetof(IconvDecoder, ivd_in));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_chnksiz) == offsetof(IconvDecoder, ivd_chnksiz));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_input) == offsetof(IconvDecoder, ivd_input));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_encoder.ice_codec) == offsetof(IconvDecoder, ivd_decoder.icd_codec));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_encoder.ice_flags) == offsetof(IconvDecoder, ivd_decoder.icd_flags));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_bufbase) == offsetof(IconvDecoder, ivd_bufbase));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_bufsize) == offsetof(IconvDecoder, ivd_bufsize));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_pndbase) == offsetof(IconvDecoder, ivd_pndbase));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_pndsize) == offsetof(IconvDecoder, ivd_pndsize));
#define iver_fillbuffer_locked     ivd_fillbuffer_locked
#define iver_fillbuffer_any_locked ivd_fillbuffer_any_locked
#define iver_read                  ivd_read

PRIVATE WUNUSED NONNULL((1)) int DCALL
iver_init(IconvEncodeReader *__restrict self, size_t argc,
          DeeObject *const *argv, DeeObject *kw) {
	iconv_codec_t codec_id;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("EncodeReader", params: """
	codec:?X2?Dstring?Dint,
	input:?DFile,
	errors:?X2?Dstring?Dint = NULL = !Pstrict,
	bool closeinput = false,
	size_t chunksize = IVD_DEFAULT_CHUNKSIZE = !4096
""", docStringPrefix: "ivd");]]]*/
#define ivd_EncodeReader_params "codec:?X2?Dstring?Dint,input:?DFile,errors:?X2?Dstring?Dint=!Pstrict,closeinput=!f,chunksize=!4096"
	struct {
		DeeObject *codec;
		DeeObject *input;
		DeeObject *errors;
		bool closeinput;
		size_t chunksize;
	} args;
	args.errors = NULL;
	args.closeinput = false;
	args.chunksize = IVD_DEFAULT_CHUNKSIZE;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__codec_input_errors_closeinput_chunksize, "oo|ob" UNPuSIZ ":EncodeReader", &args))
		goto err;
/*[[[end]]]*/
	self->iver_closein = args.closeinput;
	self->iver_chnksiz = args.chunksize;
	codec_id = deemon_iconv_parse_codec_name_and_error_mode(args.codec, args.errors,
	                                                        &self->iver_encoder.ice_flags);
	if unlikely(codec_id == ICONV_CODEC_UNKNOWN)
		goto err;
	self->iver_encoder.ice_output.ii_printer = &iver_buffer_printer;
	self->iver_encoder.ice_output.ii_arg     = self;
	self->iver_encoder.ice_codec = codec_id;
	if unlikely(do_libiconv_encode_init(&self->iver_encoder, &self->iver_input))
		goto no_such_codec;
	Dee_nrshared_lock_init(&self->iver_lock);
	ASSERT(self->iver_encoder.ice_output.ii_arg == self);
	Dee_Incref(args.input);
	self->iver_in = args.input;
	self->iver_bufbase = NULL;
	self->iver_bufsize = 0;
	self->iver_pndbase = NULL;
	self->iver_pndsize = 0;
	return 0;
no_such_codec:
	err_unknown_codec(codec_id);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1, 2)) Dee_ssize_t DCALL
iver_printrepr(IconvEncodeReader *__restrict self, Dee_formatprinter_t printer, void *arg) {
	uintptr_half_t flags;
	Dee_ssize_t temp, result;
	result = DeeFormat_PRINT(printer, arg, "EncodeReader(codec: ");
	if unlikely(result < 0)
		goto done;
	EDO(err_temp, printcodecname(self->iver_encoder.ice_codec, self->iver_encoder.ice_flags, printer, arg));
	EDO(err_temp, DeeFormat_Printf(printer, arg, ", input: %r", self->iver_in));
	flags = self->iver_encoder.ice_flags;
	flags &= ~ICONV_ERR_TRANSLIT; /* Already printed as part of codec name */
	if (flags != ICONV_ERR_ERROR) {
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ", errors: "));
		EDO(err_temp, printcodecerrors(flags, printer, arg));
	}
	if (self->iver_closein)
		EDO(err_temp, DeeFormat_PRINT(printer, arg, ", closeinput: true"));
	if (self->iver_chnksiz != IVD_DEFAULT_CHUNKSIZE)
		EDO(err_temp, DeeFormat_Printf(printer, arg, ", chunksize: %" PRFuSIZ, self->iver_chnksiz));
	EDO(err_temp, DeeFormat_PRINT(printer, arg, ")"));
done:
	return result;
err_temp:
	return temp;
}

#define iver_get_isinputshiftzero_doc ive_get_isinputshiftzero_doc
PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
iver_get_isinputshiftzero(IconvEncodeReader *__restrict self) {
	bool result;
	if (iver_acquire(self))
		goto err;
	result = libiconv_encode_isinputshiftzero(&self->iver_encoder);
	iver_release(self);
	return_bool(result);
err:
	return NULL;
}


STATIC_ASSERT(offsetof(IconvEncodeReader, iver_closein) == offsetof(IconvDecoder, ivd_closein));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_in) == offsetof(IconvDecoder, ivd_in));
#define iver_close ivd_close

PRIVATE WUNUSED NONNULL((1)) int DCALL
iver_sync(IconvEncodeReader *__restrict self) {
	Dee_ssize_t status;
	if unlikely(iver_acquire(self))
		goto err;
	status = libiconv_encode_flush(&self->iver_encoder);
	iver_release(self);
	ASSERTF(status == -1 || status >= 0,
	        "The underlying printer is `iver_buffer_printer', "
	        "which should only ever return `-1' on error");
	if unlikely(status < 0)
		goto err;
#if 0 /* Nope: not propagated! */
	if (self->ivd_closein)
		return DeeFile_Sync(self->ivd_in);
#endif
	return 0;
err:
	return -1;
}



STATIC_ASSERT(offsetof(IconvEncodeReader, iver_encoder.ice_codec) == offsetof(IconvDecoder, ivd_decoder.icd_codec));
#define iver_getcodec ivd_getcodec

PRIVATE struct type_getset tpconst iver_getsets[] = {
	TYPE_GETTER_AB("isinputshiftzero", &iver_get_isinputshiftzero,
	               DOC_GET(iver_get_isinputshiftzero_doc)),
	TYPE_GETTER_AB("codec", &iver_getcodec, "->?X2?Dstring?Dint"),
	TYPE_GETSET_END
};

STATIC_ASSERT(offsetof(IconvEncodeReader, iver_in) == offsetof(IconvDecoder, ivd_in));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_closein) == offsetof(IconvDecoder, ivd_closein));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_chnksiz) == offsetof(IconvDecoder, ivd_chnksiz));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_pndsize) == offsetof(IconvDecoder, ivd_pndsize));
STATIC_ASSERT(offsetof(IconvEncodeReader, iver_bufsize) == offsetof(IconvDecoder, ivd_bufsize));
#define iver_members ivd_members

INTDEF DeeFileTypeObject IconvEncodeReader_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "EncodeReader",
		/* .tp_doc      = */ DOC("(" ivd_EncodeReader_params ")\n"
		                         "#pcodec{Name of the codec that should be used to encode @input. May "
		                         /*   */ "also contain some additional flags (s.a. ?Gparsecodecname)}"
		                         "#pinput{The ?DFile that UTF-8 data should be read from and subsequently encoded}"
		                         DOC_param_errors
		                         DOC_param_closeinput
		                         DOC_param_chunksize
		                         ""),
		/* .tp_flags    = */ TP_FNORMAL,
		/* .tp_weakrefs = */ 0,
		/* .tp_features = */ TF_NONE,
		/* .tp_base     = */ &DeeFile_Type.ft_base,
		/* .tp_init = */ {
			Dee_TYPE_CONSTRUCTOR_INIT_FIXED(
				/* T:              */ IconvEncodeReader,
				/* tp_ctor:        */ NULL,
				/* tp_copy_ctor:   */ NULL,
				/* tp_any_ctor:    */ NULL,
				/* tp_any_ctor_kw: */ &iver_init,
				/* tp_serialize:   */ NULL /* Would be possible, but would be super-complicated since
				                            * it'd require per-codec handling not provided by `libiconv' */
			),
			/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&iver_fini,
			/* .tp_assign      = */ NULL,
			/* .tp_move_assign = */ NULL
		},
		/* .tp_cast = */ {
			/* .tp_str       = */ NULL,
			/* .tp_repr      = */ NULL,
			/* .tp_bool      = */ NULL,
			/* .tp_print     = */ NULL,
			/* .tp_printrepr = */ (Dee_ssize_t (DCALL *)(DeeObject *__restrict, Dee_formatprinter_t, void *))&iver_printrepr,
		},
		/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, Dee_visit_t, void *))&iver_visit,
		/* .tp_gc            = */ NULL,
		/* .tp_math          = */ NULL,
		/* .tp_cmp           = */ NULL,
		/* .tp_seq           = */ NULL,
		/* .tp_iter_next     = */ NULL,
		/* .tp_iterator      = */ NULL,
		/* .tp_attr          = */ NULL,
		/* .tp_with          = */ NULL,
		/* .tp_buffer        = */ NULL,
		/* .tp_methods       = */ NULL,
		/* .tp_getsets       = */ iver_getsets,
		/* .tp_members       = */ iver_members,
		/* .tp_class_methods = */ NULL,
		/* .tp_class_getsets = */ NULL,
		/* .tp_class_members = */ NULL
	},
	/* .ft_read   = */ (size_t (DCALL *)(DeeFileObject *, void *, size_t, Dee_ioflag_t))&iver_read,
	/* .ft_write  = */ NULL,
	/* .ft_seek   = */ NULL,
	/* .ft_sync   = */ (int (DCALL *)(DeeFileObject *))&iver_sync,
	/* .ft_trunc  = */ NULL,
	/* .ft_close  = */ (int (DCALL *)(DeeFileObject *))&iver_close,
	/* .ft_pread  = */ NULL,
	/* .ft_pwrite = */ NULL,
	/* .ft_getc   = */ NULL,
	/* .ft_ungetc = */ NULL,
	/* .ft_putc   = */ NULL
};



DECL_END


#endif /* !GUARD_DEX_ICONV_CODER_C */
