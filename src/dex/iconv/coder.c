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

#include <deemon/alloc.h>           /* DeeObject_MALLOC */
#include <deemon/arg.h>             /* DeeArg_UnpackStruct*, UNPu32, UNPuSIZ */
#include <deemon/bool.h>             /* DeeArg_UnpackStruct*, UNPu32, UNPuSIZ */
#include <deemon/file.h>            /* return_bool */
#include <deemon/error.h>            /* return_bool */
#include <deemon/object.h>          /* DREF, DeeObject, DeeObject_*, Dee_AsObject, Dee_formatprinter_t, Dee_ssize_t */
#include <deemon/type.h>            /* DeeObject_InitStatic, METHOD_FNORMAL */

DECL_BEGIN

/*[[[deemon
(print_DEFINE_KWLIST from rt.gen.unpack)({
	"codec",
	"out",
	"errors",
});
]]]*/
#ifndef DEFINED_kwlist__codec_out_errors
#define DEFINED_kwlist__codec_out_errors
PRIVATE DEFINE_KWLIST(kwlist__codec_out_errors, { KEX("codec", 0x91dfc790, 0x678d4474a4f58564), KEX("out", 0x20bcdfe4, 0xfc801ac012e9f722), KEX("errors", 0xd327c5ea, 0x88b9782b6de95122), KEND });
#endif /* !DEFINED_kwlist__codec_out_errors */
/*[[[end]]]*/

/************************************************************************/
/* DECODER                                                              */
/************************************************************************/

#define decoder_release(self) Dee_nrshared_lock_release(&(self)->ivd_lock)
PRIVATE WUNUSED NONNULL((1)) int DCALL
decoder_acquire(IconvDecoder *__restrict self) {
	int error = Dee_nrshared_lock_acquire(&self->ivd_lock);
	if (error == Dee_NRLOCK_ALREADY) {
		DeeError_Throwf(&DeeError_RuntimeError,
		                "Reentrant calls to `%s.write' are not allowed",
		                DeeType_GetName(&Dee_TYPE(self)->ft_base));
		error = Dee_NRLOCK_ERR;
	}
	return error;
}

PRIVATE NONNULL((1)) void DCALL
ivd_fini(IconvDecoder *__restrict self) {
	Dee_Decref((DREF DeeFileObject *)self->ivd_decoder.icd_output.ii_arg);
}

PRIVATE NONNULL((1, 2)) void DCALL
ivd_visit(IconvDecoder *__restrict self, Dee_visit_t proc, void *arg) {
	Dee_Visit((DREF DeeFileObject *)self->ivd_decoder.icd_output.ii_arg);
}


PRIVATE WUNUSED NONNULL((1)) int DCALL
ivd_init(IconvDecoder *__restrict self, size_t argc,
         DeeObject *const *argv, DeeObject *kw) {
	iconv_codec_t codec_id;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("Decoder", params: """
	codec:?X2?Dstring?Dint,
	out:?DFile,
	errors:?X2?Dstring?Dint = NULL = !Pstrict
""", docStringPrefix: "ivd");]]]*/
#define ivd_Decoder_params "codec:?X2?Dstring?Dint,out:?DFile,errors:?X2?Dstring?Dint=!Pstrict"
	struct {
		DeeObject *codec;
		DeeObject *out;
		DeeObject *errors;
	} args;
	args.errors = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__codec_out_errors, "oo|o:Decoder", &args))
		goto err;
/*[[[end]]]*/
	codec_id = deemon_iconv_parse_codec_name_and_error_mode(args.codec, args.errors,
	                                                        &self->ivd_decoder.icd_flags);
	if unlikely(codec_id == ICONV_CODEC_UNKNOWN)
		goto err;
	self->ivd_decoder.icd_output.ii_printer = (Dee_formatprinter_t)&DeeFile_WriteAll;
	self->ivd_decoder.icd_output.ii_arg     = args.out;
	self->ivd_decoder.icd_codec = codec_id;
	if unlikely(libiconv_decode_init(&self->ivd_decoder, &self->ivd_input))
		goto no_such_codec;
	Dee_nrshared_lock_init(&self->ivd_lock);
	ASSERT(self->ivd_decoder.icd_output.ii_arg == args.out);
	Dee_Incref(args.out);
	return 0;
no_such_codec:
	err_unknown_codec(codec_id);
err:
	return -1;
}

PRIVATE WUNUSED NONNULL((1)) ATTR_INS(2, 3) size_t DCALL
ivd_write(IconvDecoder *self, void const *buffer,
          size_t bufsize, Dee_ioflag_t flags) {
	Dee_ssize_t result;
	(void)flags;
	if (decoder_acquire(self))
		goto err;
	result = (*self->ivd_input.ii_printer)(self->ivd_input.ii_arg, (char const *)buffer, bufsize);
	if (result < 0) {
		if (self->ivd_decoder.icd_flags & ICONV_HASERR) {
			size_t offset;
			iconv_codec_t codec = self->ivd_decoder.icd_codec;
			decoder_release(self);
			offset = (size_t)(bufsize + result);
			err_unicode_decode_error(codec, offset);
			goto err;
		}
		ASSERTF(result == -1, "Underlying printer should be `DeeFile_WriteAll', "
		                      "which should only ever return `-1' on error");
	}
	decoder_release(self);
	/* Return the full "bufsize" instead of "result": "result" is actual the #
	 * of produced **output** bytes, which can (easily) differ from "bufsize". */
	return bufsize;
err:
	return (size_t)-1;
}

PRIVATE WUNUSED NONNULL((1)) DREF DeeObject *DCALL
ivd_get_isshiftzero(IconvDecoder *__restrict self) {
	bool result;
	if (decoder_acquire(self))
		goto err;
	result = libiconv_decode_isshiftzero(&self->ivd_decoder);
	decoder_release(self);
	return_bool(result);
err:
	return NULL;
}

#define ivd_close ivd_sync
PRIVATE WUNUSED NONNULL((1)) int DCALL
ivd_sync(IconvDecoder *__restrict self) {
	(void)self;
	return 0;
}

PRIVATE struct type_getset tpconst ivd_getsets[] = {
	TYPE_GETTER_AB("isshiftzero", &ivd_get_isshiftzero,
	               "->?Dbool\n"
	               "Check if the given encoder is in its default (zero) shift state. If it isn't, "
	               /**/ "then that must mean that it's still waiting for more input data to arrive, and "
	               /**/ "that you should either feed it said data, or deal with the fact that there's "
	               /**/ "something missing in your input.\n"
	               "WARNING: This function #B{DOESN'T} work when the given decoder is used to parse "
	               /**/ "UTF-8 input! This is because special optimizations are performed when "
	               /**/ "decoding UTF-8 (since decoders also always output UTF-8). In this "
	               /**/ "case this function will always return !t"),
	TYPE_GETSET_END
};

PRIVATE struct type_member tpconst ivd_members[] = {
	TYPE_MEMBER_FIELD_DOC("out", STRUCT_OBJECT,
	                      offsetof(IconvDecoder, ivd_decoder.icd_output.ii_arg),
	                      "->?DFile"),
	TYPE_MEMBER_END
};

INTERN DeeFileTypeObject IconvDecoder_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "Decoder",
		/* .tp_doc      = */ NULL,
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
			/* .tp_str  = */ NULL,
			/* .tp_repr = */ NULL,
			/* .tp_bool = */ NULL
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
	/* .ft_read   = */ NULL,
	/* .ft_write  = */ (size_t (DCALL *)(DeeFileObject *, void const *, size_t, Dee_ioflag_t))&ivd_write,
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
/* ENCODER                                                              */
/************************************************************************/

STATIC_ASSERT(offsetof(IconvDecoder, ivd_lock) == offsetof(IconvEncoder, ive_lock));
#define encoder_acquire(self) decoder_acquire((IconvDecoder *)(self))
#define encoder_release(self) Dee_nrshared_lock_release(&(self)->ive_lock)

STATIC_ASSERT(offsetof(IconvDecoder, ivd_decoder.icd_output.ii_arg) == offsetof(IconvEncoder, ive_encoder.ice_output.ii_arg));
#define ive_fini    ivd_fini
#define ive_visit   ivd_visit
#define ive_members ivd_members

PRIVATE WUNUSED NONNULL((1)) int DCALL
ive_init(IconvEncoder *__restrict self, size_t argc,
         DeeObject *const *argv, DeeObject *kw) {
	iconv_codec_t codec_id;
/*[[[deemon (print_DeeArg_UnpackKw from rt.gen.unpack)("Encoder", params: """
	codec:?X2?Dstring?Dint,
	out:?DFile,
	errors:?X2?Dstring?Dint = NULL = !Pstrict
""", docStringPrefix: "ivd");]]]*/
#define ivd_Encoder_params "codec:?X2?Dstring?Dint,out:?DFile,errors:?X2?Dstring?Dint=!Pstrict"
	struct {
		DeeObject *codec;
		DeeObject *out;
		DeeObject *errors;
	} args;
	args.errors = NULL;
	if (DeeArg_UnpackStructKw(argc, argv, kw, kwlist__codec_out_errors, "oo|o:Encoder", &args))
		goto err;
/*[[[end]]]*/
	codec_id = deemon_iconv_parse_codec_name_and_error_mode(args.codec, args.errors,
	                                                        &self->ive_encoder.ice_flags);
	if unlikely(codec_id == ICONV_CODEC_UNKNOWN)
		goto err;
	self->ive_encoder.ice_output.ii_printer = (Dee_formatprinter_t)&DeeFile_WriteAll;
	self->ive_encoder.ice_output.ii_arg     = args.out;
	self->ive_encoder.ice_codec = codec_id;
	if unlikely(libiconv_encode_init(&self->ive_encoder, &self->ive_input))
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

#define ive_close ive_sync
PRIVATE WUNUSED NONNULL((1)) int DCALL
ive_sync(IconvEncoder *__restrict self) {
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

PRIVATE struct type_getset tpconst ive_getsets[] = {
	TYPE_GETTER_AB("isinputshiftzero", &ive_get_isinputshiftzero,
	               "->?Dbool\n"
	               "Check if UTF-8 input taken by the given encoder is in its default (zero) shift "
	               /**/ "state. If it isn't, then that must mean that it's still waiting for more UTF-8 "
	               /**/ "data to arrive, and that you should either feed it said data, or deal with the "
	               /**/ "fact that there's something missing in your input.\n"
	               "WARNING: This function #B{DOESN'T} work when the given encoder is targeting UTF-8. "
	               /**/ "This is because special optimizations are performed when encoding UTF-8 "
	               /**/ "(since encoder also always takes UTF-8 as input). In this case, this "
	               /**/ "function will always return !N"),
	TYPE_GETSET_END
};


INTERN DeeFileTypeObject IconvEncoder_Type = {
	/* .ft_base = */ {
		OBJECT_HEAD_INIT(&DeeFileType_Type),
		/* .tp_name     = */ "Encoder",
		/* .tp_doc      = */ NULL,
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
			/* .tp_str  = */ NULL,
			/* .tp_repr = */ NULL,
			/* .tp_bool = */ NULL
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






STATIC_ASSERT(offsetof(IconvDecoder, ivd_lock) == offsetof(IconvTranscoder, ivt_lock));
#define transcoder_acquire(self) decoder_acquire((IconvDecoder *)(self))
#define transcoder_release(self) Dee_nrshared_lock_release(&(self)->ivt_lock)
STATIC_ASSERT(offsetof(IconvDecoder, ivd_decoder.icd_output.ii_arg) == offsetof(IconvTranscoder, ivt_encoder.ice_output.ii_arg));
#define ivt_fini  ivd_fini
#define ivt_visit ivd_visit

DECL_END


#endif /* !GUARD_DEX_ICONV_CODER_C */
