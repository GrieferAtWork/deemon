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
#ifndef GUARD_DEEMON_RUNTIME_SERIAL_C
#define GUARD_DEEMON_RUNTIME_SERIAL_C 1

#include <deemon/api.h>
#include <deemon/object.h>
#include <deemon/serial.h>
#include <deemon/system-features.h>
#include <deemon/types.h>

DECL_BEGIN

/* Serialize a `DREF DeeObject *' field at `addrof_object' as being populated with a reference to `ob'
 * @return: 0 : Success
 * @return: -1: Error */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeSerial_XPutObject)(DeeSerial *__restrict self,
                             Dee_seraddr_t addrof_object,
                             /*0..1*/ DeeObject *ob) {
	DREF DeeObject **p_out_ob;
	if (ob != NULL)
		return DeeSerial_PutObject(self, addrof_object, ob);
	p_out_ob = DeeSerial_Addr2Mem(self, addrof_object, DREF DeeObject *);
	*p_out_ob = NULL;
	return 0;
}

PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeSerial_PutObjectInherited)(DeeSerial *__restrict self, Dee_seraddr_t addrof_object,
                                     /*1..1*/ /*inherit(always)*/ DREF DeeObject *__restrict ob) {
	int result = DeeSerial_PutObject(self, addrof_object, ob);
	Dee_Decref(ob);
	return result;
}

PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeSerial_XPutObjectInherited)(DeeSerial *__restrict self, Dee_seraddr_t addrof_object,
                                      /*0..1*/ /*inherit(always)*/ DREF DeeObject *ob) {
	DREF DeeObject **p_out_ob;
	if (ob != NULL)
		return DeeSerial_PutObjectInherited(self, addrof_object, ob);
	p_out_ob = DeeSerial_Addr2Mem(self, addrof_object, DREF DeeObject *);
	*p_out_ob = NULL;
	return 0;
}


/* Serialize a `void *' field at `addrof_pointer' as being populated with the address of a static
 * object at `static_addr' ("static" here meaning that `DeeModule_FromStaticPointer()' will
 * return a non-NULL pointer for `static_addr'). Behavior is undefined if `static_addr' does
 * cannot be resolved using `DeeModule_FromStaticPointer()'.
 * @return: 0 : Success
 * @return: -1: Error */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeSerial_XPutStatic)(DeeSerial *__restrict self,
                             Dee_seraddr_t addrof_pointer,
                             void const *static_addr) {
	void **p_out_addr;
	if (static_addr != NULL)
		return DeeSerial_PutStatic(self, addrof_pointer, static_addr);
	p_out_addr = DeeSerial_Addr2Mem(self, addrof_pointer, void *);
	*p_out_addr = NULL;
	return 0;
}


/* Helper wrapper for encoding a pointer to the memdup of `data' at `addrof_pointer':
 * >> Dee_seraddr_t addrof_dup = DeeSerial_Malloc(self, num_bytes);
 * >> memcpy(DeeSerial_Addr2Mem(self, addrof_dup, void), data, num_bytes);
 * >> return DeeSerial_PutAddr(self, addrof_pointer, addrof_dup); */
PUBLIC WUNUSED NONNULL((1)) int
(DCALL DeeSerial_PutMemdup)(DeeSerial *__restrict self,
                            Dee_seraddr_t addrof_pointer,
                            void const *data, size_t num_bytes) {
	Dee_seraddr_t addrof_dup = DeeSerial_Malloc(self, num_bytes);
	if (!Dee_SERADDR_ISOK(addrof_dup))
		goto err;
	memcpy(DeeSerial_Addr2Mem(self, addrof_dup, void), data, num_bytes);
	return DeeSerial_PutAddr(self, addrof_pointer, addrof_dup);
err:
	return -1;
}



/* Inplace-serialize an object references:
 * >> DREF DeeObject *obj = *DeeSerial_Addr2Mem(self, addrof_object, DREF DeeObject *);
 * >> int result = DeeSerial_PutObject(self, addrof_object, obj);
 * >> Dee_Decref(obj);
 * >> return result; */
PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeSerial_InplacePutObject)(DeeSerial *__restrict self,
                                   Dee_seraddr_t addrof_object) {
	DREF DeeObject *obj = *DeeSerial_Addr2Mem(self, addrof_object, DREF DeeObject *);
	int result = DeeSerial_PutObject(self, addrof_object, obj);
	Dee_Decref(obj);
	return result;
}

PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeSerial_XInplacePutObject)(DeeSerial *__restrict self,
                                    Dee_seraddr_t addrof_object) {
	DREF DeeObject *obj = *DeeSerial_Addr2Mem(self, addrof_object, DREF DeeObject *);
	if (obj) {
		int result = DeeSerial_PutObject(self, addrof_object, obj);
		Dee_Decref(obj);
		return result;
	}
	/* No need to do anything in this case -- field is already set to "NULL" */
	return 0;
}


/* Inplace-serialize an array of object references. Said
 * array of object references is **ALWAYS** inherited:
 * >> while (objc) {
 * >>     int error = DeeSerial_InplacePutObject(self, addrof_objects);
 * >>     --objc;
 * >>     addrof_objects += sizeof(DREF DeeObject *);
 * >>     if unlikely(error) {
 * >>         Dee_Decrefv(DeeSerial_Addr2Mem(self, addrof_objects, DREF DeeObject *), objc);
 * >>         return error;
 * >>     }
 * >> }
 * >> return 0; */
PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeSerial_InplacePutObjectv)(DeeSerial *__restrict self,
                                    Dee_seraddr_t addrof_objects,
                                    size_t objc) {
	while (objc) {
		int error = DeeSerial_InplacePutObject(self, addrof_objects);
		--objc;
		addrof_objects += sizeof(DREF DeeObject *);
		if unlikely(error) {
			DREF DeeObject **objv;
			objv = DeeSerial_Addr2Mem(self, addrof_objects, DREF DeeObject *);
			Dee_Decrefv(objv, objc);
			return error;
		}
	}
	return 0;
}

PUBLIC WUNUSED NONNULL((1, 3)) int
(DCALL DeeSerial_XInplacePutObjectv)(DeeSerial *__restrict self,
                                     Dee_seraddr_t addrof_objects,
                                     size_t objc) {
	while (objc) {
		int error = DeeSerial_XInplacePutObject(self, addrof_objects);
		--objc;
		addrof_objects += sizeof(DREF DeeObject *);
		if unlikely(error) {
			DREF DeeObject **objv;
			objv = DeeSerial_Addr2Mem(self, addrof_objects, DREF DeeObject *);
			Dee_XDecrefv(objv, objc);
			return error;
		}
	}
	return 0;
}

DECL_END

#endif /* !GUARD_DEEMON_RUNTIME_SERIAL_C */
