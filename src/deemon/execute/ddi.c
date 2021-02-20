/* Copyright (c) 2018-2021 Griefer@Work                                       *
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
 *    Portions Copyright (c) 2018-2021 Griefer@Work                           *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef GUARD_DEEMON_EXECUTE_DDI_C
#define GUARD_DEEMON_EXECUTE_DDI_C 1

#include <deemon/alloc.h>
#include <deemon/api.h>
#include <deemon/asm.h>
#include <deemon/bool.h>
#include <deemon/code.h>
#include <deemon/object.h>
#include <deemon/string.h>
#include <deemon/system-features.h> /* memcpyc(), ... */

#include <hybrid/minmax.h>

DECL_BEGIN


PRIVATE NONNULL((1)) int DCALL
get_sleb(uint8_t **__restrict pip) {
	int result, is_neg;
	uint8_t *ip, byte, num_bits;
	ip       = *pip;
	byte     = *ip++;
	num_bits = 6;
	is_neg   = (byte & 0x40);
	result   = byte & 0x3f;
	while (byte & 0x80) {
		byte = *ip++;
		result |= (byte & 0x7f) << num_bits;
		num_bits += 7;
	}
	if (is_neg)
		result = -result;
	*pip = ip;
	return result;
}

PRIVATE NONNULL((1)) unsigned int DCALL
get_uleb(uint8_t **__restrict pip) {
	unsigned int result;
	uint8_t *ip, byte, num_bits;
	result   = 0;
	ip       = *pip;
	num_bits = 0;
	do {
		byte = *ip++;
		result |= (byte & 0x7f) << num_bits;
		num_bits += 7;
	} while (byte & 0x80);
	*pip = ip;
	return result;
}

PUBLIC WUNUSED NONNULL((1, 2)) uint8_t *DCALL
Dee_ddi_next_simple(uint8_t *__restrict ip,
                    code_addr_t *__restrict puip) {
	code_addr_t uip = *puip;
	for (;;) {
		uint8_t op = *ip++;
		switch (op) {

		case DDI_STOP:
			*puip = uip;
			return DDI_NEXT_DONE; /* End of DDI stream. */

		case DDI_ADDUIP:
			++uip;
			uip += get_uleb((uint8_t **)&ip);
			*puip = uip;
			return ip; /* Checkpoint. */

		case DDI_ADDLNO:
		case DDI_SETCOL:
		case DDI_ADDUSP:
			get_sleb((uint8_t **)&ip);
			break;

		case DDI_DEFLCNAME:
			get_uleb((uint8_t **)&ip);
			ATTR_FALLTHROUGH
		case DDI_SETPATH:
		case DDI_SETFILE:
		case DDI_SETNAME:
		case DDI_DELLCNAME:
		case DDI_DEFSPNAME:
		case DDI_EXTENDED:
			get_uleb((uint8_t **)&ip);
			break;

		case DDI_INCUSP:
		case DDI_DECUSP:
			break;

		default:
			uip += DDI_GENERIC_IP(op);
			*puip = uip;
			return ip; /* Checkpoint. */
		}
	}
}

PUBLIC WUNUSED NONNULL((1, 2)) uint8_t *DCALL
Dee_ddi_next_regs(uint8_t *__restrict ip,
                  struct ddi_regs *__restrict regs) {
	/* This algorithm is heavily documented and explained in `<deemon/asm.h>' */
	for (;;) {
		uint8_t op = *ip++;
		switch (op) {

		case DDI_STOP:
			return DDI_NEXT_DONE; /* End of DDI stream. */

		case DDI_ADDUIP:
			++regs->dr_uip;
			regs->dr_uip += get_uleb((uint8_t **)&ip);
			return ip; /* Checkpoint. */

		case DDI_ADDLNO: {
			int temp;
			temp = get_sleb((uint8_t **)&ip);
			if (!(regs->dr_flags & DDI_REGS_FSECOND))
				regs->dr_lno += temp;
		}	break;

		case DDI_SETCOL: {
			int temp;
			temp = get_sleb((uint8_t **)&ip);
			if (!(regs->dr_flags & DDI_REGS_FSECOND))
				regs->dr_col = temp;
		}	break;

		case DDI_SETPATH: {
			uint16_t temp;
			temp = (uint16_t)get_uleb((uint8_t **)&ip);
			if (!(regs->dr_flags & DDI_REGS_FSECOND))
				regs->dr_path = temp;
		}	break;

		case DDI_SETFILE: {
			uint16_t temp;
			temp = (uint16_t)get_uleb((uint8_t **)&ip);
			if (!(regs->dr_flags & DDI_REGS_FSECOND))
				regs->dr_file = temp;
		}	break;

		case DDI_SETNAME: {
			uint16_t temp;
			temp = (uint16_t)get_uleb((uint8_t **)&ip);
			if (!(regs->dr_flags & DDI_REGS_FSECOND))
				regs->dr_name = temp;
		}	break;

		case DDI_INCUSP:
			++regs->dr_usp;
			break;

		case DDI_DECUSP:
			--regs->dr_usp;
			break;

		case DDI_ADDUSP:
			regs->dr_usp += (int16_t)get_sleb((uint8_t **)&ip);
			break;

			/* Ignore symbol-name instructions. */
		case DDI_DEFLCNAME:
			get_uleb((uint8_t **)&ip);
			ATTR_FALLTHROUGH
		case DDI_DELLCNAME:
		case DDI_DEFSPNAME:
			get_uleb((uint8_t **)&ip);
			break;

		case DDI_EXTENDED: {
			unsigned int cmd;
			cmd = get_uleb((uint8_t **)&ip);
			switch (cmd & DDI_X_CMDMASK) {

			case DDI_X_TOGGLESTMT:
				if (!(regs->dr_flags & DDI_REGS_FSECOND))
					regs->dr_flags ^= DDI_REGS_FISSTMT;
				break;

			case DDI_X_PUSHSTATE:
				regs->dr_flags += DDI_REGS_FSECONE;
				break;

			case DDI_X_POPSTATE:
				if (regs->dr_flags & DDI_REGS_FSECOND)
					regs->dr_flags -= DDI_REGS_FSECONE;
				break;

			default: break;
			}
		}	break;

		default:
			regs->dr_uip += DDI_GENERIC_IP(op);
			if (!(regs->dr_flags & DDI_REGS_FSECOND))
				regs->dr_lno += DDI_GENERIC_LN(op);
			return ip; /* Checkpoint. */
		}
	}
}



LOCAL NONNULL((1)) int DCALL
ddi_xrealloc_sp(struct ddi_xregs *__restrict regs,
                uint16_t min_size, unsigned int flags) {
	uint16_t new_alloc, *new_vec;
	new_alloc = regs->dx_spnama * 2;
	if (new_alloc < min_size)
		new_alloc = min_size;
	new_vec = (uint16_t *)Dee_TryRealloc(regs->dx_spnamv,
	                                     new_alloc *
	                                     sizeof(uint16_t));
	if (!new_vec) {
		new_alloc = min_size;
		if (flags & DDI_STATE_FNOTHROW) {
			new_vec = (uint16_t *)Dee_TryRealloc(regs->dx_spnamv,
			                                     new_alloc *
			                                     sizeof(uint16_t));
		} else {
			new_vec = (uint16_t *)Dee_Realloc(regs->dx_spnamv,
			                                  new_alloc *
			                                  sizeof(uint16_t));
		}
		if unlikely(!new_vec)
			goto err;
	}
	regs->dx_spnamv = new_vec;
	regs->dx_spnama = new_alloc;
	return 0;
err:
	return -1;
}


PUBLIC WUNUSED NONNULL((1, 2)) uint8_t *DCALL
Dee_ddi_next_state(uint8_t *__restrict ip,
                   struct ddi_state *__restrict self,
                   unsigned int flags) {
	/* This algorithm is heavily documented and explained in `<deemon/asm.h>' */
next_ip:
	for (;;) {
		uint8_t op = *ip++;
		switch (op) {

		case DDI_STOP:
			return DDI_NEXT_DONE; /* End of DDI stream. */

		case DDI_ADDUIP:
			++self->rs_regs.dr_uip;
			self->rs_regs.dr_uip += get_uleb((uint8_t **)&ip);
			return ip; /* Checkpoint. */

		case DDI_ADDLNO:
			self->rs_regs.dr_lno += get_sleb((uint8_t **)&ip);
			break;

		case DDI_SETCOL:
			self->rs_regs.dr_col = get_sleb((uint8_t **)&ip);
			break;

		case DDI_SETPATH:
			self->rs_regs.dr_path = (uint16_t)get_uleb((uint8_t **)&ip);
			break;

		case DDI_SETFILE:
			self->rs_regs.dr_file = (uint16_t)get_uleb((uint8_t **)&ip);
			break;

		case DDI_SETNAME:
			self->rs_regs.dr_name = (uint16_t)get_uleb((uint8_t **)&ip);
			break;

		case DDI_INCUSP:
			++self->rs_regs.dr_usp;
			break;

		case DDI_DECUSP:
			--self->rs_regs.dr_usp;
			/* Unbind the stack-symbols. */
			if (self->rs_regs.dr_usp < self->rs_xregs.dx_spnama)
				self->rs_xregs.dx_spnamv[self->rs_regs.dr_usp] = DDI_REGS_UNBOUND_NAME;
			break;

		case DDI_ADDUSP: {
			int16_t offset;
			offset = (int16_t)get_sleb((uint8_t **)&ip);
			self->rs_regs.dr_usp += offset;
			if (offset < 0 && !(flags & DDI_STATE_FNONAMES) &&
			    self->rs_regs.dr_usp < self->rs_xregs.dx_spnama) {
				/* Unbind all stack-symbols as they fall out of view. */
				uint16_t i, clr_cnt;
				clr_cnt = MIN(self->rs_xregs.dx_spnama - self->rs_regs.dr_usp,
				              (uint16_t)-offset);
				for (i = 0; i < clr_cnt; ++i)
					self->rs_xregs.dx_spnamv[self->rs_regs.dr_usp + i] = DDI_REGS_UNBOUND_NAME;
			}
		}	break;

		case DDI_DEFSPNAME: {
			uint16_t address;
			/* Define the name of a stack-symbol. */
			address = (uint16_t)get_uleb((uint8_t **)&ip);
			if (flags & DDI_STATE_FNONAMES)
				break;
			if unlikely(!self->rs_regs.dr_usp)
				break; /* Shouldn't happen... */
			if (self->rs_regs.dr_usp >= self->rs_xregs.dx_spnama &&
			    ddi_xrealloc_sp(&self->rs_xregs, self->rs_regs.dr_usp, flags))
				goto err;
			self->rs_xregs.dx_spnamv[self->rs_regs.dr_usp - 1] = address;
		}	break;

		case DDI_DEFLCNAME: {
			uint16_t index, address;
			index   = (uint16_t)get_uleb((uint8_t **)&ip);
			address = (uint16_t)get_uleb((uint8_t **)&ip);
			if (flags & DDI_STATE_FNONAMES)
				break;
			if (index >= self->rs_xregs.dx_lcnamc) {
				/* Bind a stack-symbol. */
				index -= self->rs_xregs.dx_lcnamc;
				if (index >= self->rs_xregs.dx_spnama &&
				    ddi_xrealloc_sp(&self->rs_xregs, index + 1, flags))
					goto err;
				/* Bind the stack-symbol */
				self->rs_xregs.dx_spnamv[index] = address;
			} else {
				/* Bind a local-symbol. */
				self->rs_xregs.dx_lcnamv[index] = address;
			}
		}	break;

		case DDI_DELLCNAME: {
			unsigned int index;
			index = get_uleb((uint8_t **)&ip);
			if (flags & DDI_STATE_FNONAMES)
				break;
			if (index >= self->rs_xregs.dx_lcnamc) {
				/* Unbind a stack-symbol. */
				index -= self->rs_xregs.dx_lcnamc;
				if (index < self->rs_xregs.dx_spnama)
					self->rs_xregs.dx_spnamv[index] = DDI_REGS_UNBOUND_NAME;
			} else {
				/* Unbind a local-symbol. */
				self->rs_xregs.dx_lcnamv[index] = DDI_REGS_UNBOUND_NAME;
			}
		}	break;

		case DDI_EXTENDED: {
			unsigned int cmd;
			cmd = get_uleb((uint8_t **)&ip);
			switch (cmd & DDI_X_CMDMASK) {

			case DDI_X_TOGGLESTMT:
				self->rs_regs.dr_flags ^= DDI_REGS_FISSTMT;
				break;

			case DDI_X_PUSHSTATE: {
				struct ddi_saved *save;
				/* Save the current register state. */
				save = (struct ddi_saved *)Dee_TryMalloc(sizeof(struct ddi_saved));
				if unlikely(!save) {
					if (flags & DDI_STATE_FNOTHROW)
						goto next_ip;
					save = (struct ddi_saved *)Dee_Malloc(sizeof(struct ddi_saved));
					if unlikely(!save)
						goto err;
				}
				memcpy(&save->s_save.dx_base.dr_usp, &self->rs_xregs.dx_base.dr_usp,
				       sizeof(struct ddi_xregs) - offsetof(struct ddi_xregs, dx_spnama));
				/* Copy bound local-symbol names */
				save->s_save.dx_lcnamc = self->rs_xregs.dx_lcnamc;
				if (flags & DDI_STATE_FNONAMES)
					save->s_save.dx_lcnamc = 0;
				if (!save->s_save.dx_lcnamc)
					save->s_save.dx_lcnamv = NULL;
				else {
					save->s_save.dx_lcnamv = (uint16_t *)Dee_TryMalloc(self->rs_xregs.dx_lcnamc *
					                                                   sizeof(uint16_t));
					if unlikely(!save->s_save.dx_lcnamv) {
						if (flags & DDI_STATE_FNOTHROW) {
							Dee_Free(save);
							goto next_ip;
						}
						save->s_save.dx_lcnamv = (uint16_t *)Dee_Malloc(self->rs_xregs.dx_lcnamc *
						                                                sizeof(uint16_t));
						if unlikely(!save->s_save.dx_lcnamv)
							goto err_save;
					}
					memcpyc(save->s_save.dx_lcnamv, self->rs_xregs.dx_lcnamv,
					        self->rs_xregs.dx_lcnamc, sizeof(uint16_t));
				}
				/* Copy bound stack-symbol names */
				save->s_save.dx_spnama = self->rs_xregs.dx_spnama;
				if (save->s_save.dx_spnama > self->rs_xregs.dx_base.dr_usp)
					save->s_save.dx_spnama = self->rs_xregs.dx_base.dr_usp;
				while (save->s_save.dx_spnama &&
				       self->rs_xregs.dx_spnamv[save->s_save.dx_spnama - 1] == DDI_REGS_UNBOUND_NAME)
					--save->s_save.dx_spnama;
				if (flags & DDI_STATE_FNONAMES)
					save->s_save.dx_spnama = 0;
				if (!save->s_save.dx_spnama)
					save->s_save.dx_spnamv = NULL;
				else {
					save->s_save.dx_spnamv = (uint16_t *)Dee_TryMalloc(save->s_save.dx_spnama *
					                                                   sizeof(uint16_t));
					if unlikely(!save->s_save.dx_spnamv) {
						if (flags & DDI_STATE_FNOTHROW) {
							Dee_Free(save->s_save.dx_lcnamv);
							Dee_Free(save);
							goto next_ip;
						}
						save->s_save.dx_spnamv = (uint16_t *)Dee_Malloc(save->s_save.dx_spnama *
						                                                sizeof(uint16_t));
						if unlikely(!save->s_save.dx_spnamv)
							goto err_save_lc;
					}
					memcpyc(save->s_save.dx_spnamv, self->rs_xregs.dx_spnamv,
					        save->s_save.dx_spnama, sizeof(uint16_t));
				}
				/* Append the saved register state in the chain of states saved. */
				save->s_prev  = self->rs_save;
				self->rs_save = save;
				break;
err_save_lc:
				Dee_Free(save->s_save.dx_lcnamv);
err_save:
				Dee_Free(save);
				goto err;
			}	break;

			case DDI_X_POPSTATE: {
				struct ddi_saved *save;
				if ((save = self->rs_save) == NULL)
					break;
				/* Restore the saved register state (but don't modify the UIP/USP registers) */
				Dee_Free(self->rs_xregs.dx_spnamv);
				Dee_Free(self->rs_xregs.dx_lcnamv);
				memcpy(&self->rs_xregs.dx_base.dr_flags, &save->s_save.dx_base.dr_flags,
				       sizeof(struct ddi_xregs) - offsetof(struct ddi_xregs, dx_base.dr_flags));
				self->rs_save = save->s_prev;
				Dee_Free(save);
			}	break;

			default: break;
			}
		}	break;

		default:
			self->rs_regs.dr_uip += DDI_GENERIC_IP(op);
			self->rs_regs.dr_lno += DDI_GENERIC_LN(op);
			return ip; /* Checkpoint. */
		}
	}
err:
	if (flags & DDI_STATE_FNOTHROW)
		goto next_ip;
	return DDI_NEXT_ERR;
}

/* Initialize the given DDI register state from `code'.
 * @param: flags:          Set of `DDI_STATE_F*'
 * @return: * :            Successfully initialized the register state.
 *                         A pointer to the next DDI instruction.
 *                         This pointer can be used to enumerate DDI information.
 * @return: DDI_NEXT_ERR:  [!DDI_STATE_FNOTHROW] An error occurred.
 * NOTE: Upon error (return == DDI_NEXT_DONE || return == DDI_NEXT_ERR),
 *       the given ddi-state `self' is initialized to a no-op state that
 *       can still be used in a call to `Dee_ddi_state_fini()'! */
PUBLIC WUNUSED NONNULL((1, 2)) uint8_t *DCALL
Dee_ddi_state_init(struct ddi_state *__restrict self,
                   DeeObject *__restrict code,
                   unsigned int flags) {
	DeeDDIObject *ddi;
	uint16_t i;
	ASSERT_OBJECT_TYPE_EXACT(code, &DeeCode_Type);
	ddi = ((DeeCodeObject *)code)->co_ddi;
	ASSERT_OBJECT_TYPE_EXACT(ddi, &DeeDDI_Type);
	memcpy(&self->rs_regs, &ddi->d_start, sizeof(struct ddi_regs));
	self->rs_xregs.dx_lcnamc = ((DeeCodeObject *)code)->co_localc;
	self->rs_xregs.dx_spnama = (uint16_t)DeeCode_StackDepth((DeeCodeObject *)code);
	if (flags & DDI_STATE_FNONAMES) {
		self->rs_xregs.dx_lcnamc = 0;
		self->rs_xregs.dx_spnama = 0;
	}
	/* Allocate the local-name buffer. */
	if (!self->rs_xregs.dx_lcnamc)
		self->rs_xregs.dx_lcnamv = NULL;
	else {
		self->rs_xregs.dx_lcnamv = (uint16_t *)Dee_TryMalloc(self->rs_xregs.dx_lcnamc *
		                                                     sizeof(uint16_t));
		if unlikely(!self->rs_xregs.dx_lcnamv) {
			if (flags & DDI_STATE_FNOTHROW)
				self->rs_xregs.dx_lcnamc = 0;
			else {
				self->rs_xregs.dx_lcnamv = (uint16_t *)Dee_Malloc(self->rs_xregs.dx_lcnamc *
				                                                  sizeof(uint16_t));
				if unlikely(!self->rs_xregs.dx_lcnamv)
					return DDI_NEXT_ERR;
			}
		}
		for (i = 0; i < self->rs_xregs.dx_lcnamc; ++i)
			self->rs_xregs.dx_lcnamv[i] = DDI_REGS_UNBOUND_NAME;
	}
	/* Allocate an initial stack-name buffer. */
	if (!self->rs_xregs.dx_spnama)
		self->rs_xregs.dx_spnamv = NULL;
	else {
		self->rs_xregs.dx_spnamv = (uint16_t *)Dee_TryMalloc(self->rs_xregs.dx_spnama *
		                                                     sizeof(uint16_t));
		if unlikely(!self->rs_xregs.dx_spnamv)
			self->rs_xregs.dx_spnama = 0; /* The SP-buffer is optional, so don't sweat it if this failed. */
		else {
			for (i = 0; i < self->rs_xregs.dx_spnama; ++i) {
				self->rs_xregs.dx_spnamv[i] = DDI_REGS_UNBOUND_NAME;
			}
		}
	}
	self->rs_save = NULL;

	/* Execute the initialization bootstrap DDI block. */
	if (ddi->d_ddiinit != 0) {
		uint8_t *result, *end;
		end    = ddi->d_ddi + ddi->d_ddiinit;
		result = ddi->d_ddi;
		do {
			result = Dee_ddi_next_state(result, self, flags);
		} while (DDI_ISOK(result) && result < end);
		return result;
	}
	return ddi->d_ddi;
}

PUBLIC NONNULL((1)) void DCALL
Dee_ddi_state_fini(struct ddi_state *__restrict self) {
	struct ddi_saved *iter, *next;
	iter = self->rs_save;
	while (iter) {
		next = iter->s_prev;
		Dee_Free(iter->s_save.dx_lcnamv);
		Dee_Free(iter->s_save.dx_spnamv);
		Dee_Free(iter);
		iter = next;
	}
	Dee_Free(self->rs_xregs.dx_lcnamv);
	Dee_Free(self->rs_xregs.dx_spnamv);
}



/* Query DDI information for a given code address.
 * @param: self:            The code object for which DDI information should be queried.
 * @param: state:     [out] DDI information for the closest checkpoint below `uip'
 * @param: opt_endip: [out] When non-NULL, filled with the UIP of the closest checkpoint above `uip'
 * @param: flags:           Set of `DDI_STATE_F*'
 * @return: * :             Successfully found the DDI state describing `uip'
 * @return: DDI_NEXT_ERR:   [!DDI_STATE_FNOTHROW] An error occurred.
 * @return: DDI_NEXT_DONE:  The DDI information stream has ended after `DDI_STOP' was read. */
PUBLIC WUNUSED NONNULL((1, 2)) uint8_t *DCALL
DeeCode_FindDDI(DeeObject *__restrict self,
                struct ddi_state *__restrict start_state,
                code_addr_t *opt_endip, code_addr_t uip,
                unsigned int flags) {
	uint8_t *ip, *end_ip;
	ip = Dee_ddi_state_init(start_state, self, flags);
	while (DDI_ISOK(ip)) {
		code_addr_t end_uip;
		end_uip = start_state->rs_regs.dr_uip;
		end_ip  = Dee_ddi_next_simple(ip, &end_uip);
		if (!DDI_ISOK(end_ip)) {
			ip = end_ip;
			break;
		}
		if (uip >= start_state->rs_regs.dr_uip && uip < end_uip) {
			if (opt_endip)
				*opt_endip = end_uip;
			return ip; /* Found it! */
		}
		ip = Dee_ddi_next_state(ip, start_state, flags);
	}
	Dee_ddi_state_fini(start_state);
	return ip;
}


INTERN DEFINE_DDI(empty_ddi,
                  /* d_strings: */ NULL,
                  /* d_strtab:  */ (DeeStringObject *)Dee_EmptyString,
                  /* d_exdat:   */ NULL,
                  /* d_ddisize: */ 1,
                  /* d_nstring: */ 0,
                  /* d_ddiinit: */ 0,
                  /* d_start:   */ {
                      /* .dr_uip   = */ 0,
                      /* .dr_usp   = */ 0,
                      /* .dr_flags = */ 0,
                      /* .dr_path  = */ 0,
                      /* .dr_file  = */ 0,
                      /* .dr_name  = */ 0,
                      /* ._dr_pad  = */ { 0 },
                      /* .dr_col   = */ 0,
                      /* .dr_lno   = */ 0
                  },
                  /* d_ddi:     */ { DDI_STOP });

PRIVATE NONNULL((1)) void DCALL
ddi_fini(DeeDDIObject *__restrict self) {
	ASSERT(self != (DeeDDIObject *)&empty_ddi);
	Dee_Free((void *)self->d_strings);
	Dee_Free((void *)self->d_exdat);
	Dee_Decref(self->d_strtab);
}

PRIVATE NONNULL((1, 2)) void DCALL
ddi_visit(DeeDDIObject *__restrict self, dvisit_t proc, void *arg) {
	Dee_Visit(self->d_strtab);
}

PRIVATE struct type_member tpconst ddi_members[] = {
	TYPE_MEMBER_FIELD_DOC("__strtab__", STRUCT_OBJECT, offsetof(DeeDDIObject, d_strtab), "->?Dstring"),
	TYPE_MEMBER_END
};

PRIVATE WUNUSED DREF DeeDDIObject *DCALL ddi_ctor(void) {
	return_reference_((DREF DeeDDIObject *)&empty_ddi);
}

PRIVATE WUNUSED NONNULL((1)) dhash_t DCALL
ddi_hash(DeeDDIObject *__restrict self) {
	dhash_t result;
	uint16_t i;
	result = self->d_ddisize;
	result ^= *(uint32_t *)&self->d_nstring;
	for (i = 0; i < self->d_nstring; ++i)
		result ^= self->d_strings[i];
	result ^= DeeString_Hash((DeeObject *)self->d_strtab);
	result ^= Dee_HashPtr(&self->d_start, self->d_ddisize + sizeof(struct ddi_regs));
	if (self->d_exdat)
		result ^= Dee_HashPtr(self->d_exdat->dx_data, self->d_exdat->dx_size);
	return result;
}

PRIVATE WUNUSED NONNULL((1, 2)) bool DCALL
ddi_eq_impl(DeeDDIObject *__restrict self,
            DeeDDIObject *__restrict other) {
	if (self == other)
		return true;
	if (self->d_ddisize != other->d_ddisize)
		goto nope;
	if (self->d_nstring != other->d_nstring)
		goto nope;
	if (DeeString_SIZE(self->d_strtab) != DeeString_SIZE(other->d_strtab))
		goto nope;
	if ((self->d_exdat != NULL) != (other->d_exdat != NULL))
		goto nope;
	if (self->d_exdat &&
	    (self->d_exdat->dx_size != other->d_exdat->dx_size ||
	     memcmp(self->d_exdat->dx_data, other->d_exdat->dx_data,
	            self->d_exdat->dx_size) != 0))
		goto nope;
	if (memcmp(DeeString_STR(self->d_strtab),
	           DeeString_STR(other->d_strtab),
	           DeeString_SIZE(self->d_strtab)) != 0)
		goto nope;
	if (memcmp(self->d_strings, other->d_strings, self->d_nstring * sizeof(*self->d_strings)) != 0)
		goto nope;
	if (memcmp(&self->d_start, &other->d_start, self->d_ddisize + sizeof(struct ddi_regs)) != 0)
		goto nope;
	return true;
nope:
	return false;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ddi_eq(DeeDDIObject *self,
       DeeDDIObject *other) {
	if (DeeObject_AssertTypeExact(other, &DeeDDI_Type))
		goto err;
	return_bool(ddi_eq_impl(self, other));
err:
	return NULL;
}

PRIVATE WUNUSED NONNULL((1, 2)) DREF DeeObject *DCALL
ddi_ne(DeeDDIObject *self,
       DeeDDIObject *other) {
	if (DeeObject_AssertTypeExact(other, &DeeDDI_Type))
		goto err;
	return_bool(!ddi_eq_impl(self, other));
err:
	return NULL;
}

PRIVATE struct type_cmp ddi_cmp = {
	/* .tp_hash = */ (dhash_t (DCALL *)(DeeObject *__restrict))&ddi_hash,
	/* .tp_eq   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ddi_eq,
	/* .tp_ne   = */ (DREF DeeObject *(DCALL *)(DeeObject *, DeeObject *))&ddi_ne
};

PUBLIC DeeTypeObject DeeDDI_Type = {
	OBJECT_HEAD_INIT(&DeeType_Type),
	/* .tp_name     = */ "_Ddi",
	/* .tp_doc      = */ NULL,
	/* .tp_flags    = */ TP_FNORMAL | TP_FVARIABLE | TP_FFINAL,
	/* .tp_weakrefs = */ 0,
	/* .tp_features = */ TF_NONE,
	/* .tp_base     = */ &DeeObject_Type,
	/* .tp_init = */ {
		{
			/* .tp_var = */ {
				/* .tp_ctor      = */ (dfunptr_t)&ddi_ctor,
				/* .tp_copy_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_deep_ctor = */ (dfunptr_t)&DeeObject_NewRef,
				/* .tp_any_ctor  = */ (dfunptr_t)NULL, /* TODO */
				/* .tp_free      = */ (dfunptr_t)NULL
			}
		},
		/* .tp_dtor        = */ (void (DCALL *)(DeeObject *__restrict))&ddi_fini,
		/* .tp_assign      = */ NULL,
		/* .tp_move_assign = */ NULL
	},
	/* .tp_cast = */ {
		/* .tp_str  = */ NULL,
		/* .tp_repr = */ NULL,
		/* .tp_bool = */ NULL
	},
	/* .tp_call          = */ NULL,
	/* .tp_visit         = */ (void (DCALL *)(DeeObject *__restrict, dvisit_t, void *))&ddi_visit,
	/* .tp_gc            = */ NULL,
	/* .tp_math          = */ NULL,
	/* .tp_cmp           = */ &ddi_cmp,
	/* .tp_seq           = */ NULL,
	/* .tp_iter_next     = */ NULL,
	/* .tp_attr          = */ NULL,
	/* .tp_with          = */ NULL,
	/* .tp_buffer        = */ NULL,
	/* .tp_methods       = */ NULL, /* TODO: __sizeof__(), <helpers for addr2line, etc.> */
	/* .tp_getsets       = */ NULL,
	/* .tp_members       = */ ddi_members,
	/* .tp_class_methods = */ NULL,
	/* .tp_class_getsets = */ NULL,
	/* .tp_class_members = */ NULL
};

DECL_END

#endif /* !GUARD_DEEMON_EXECUTE_DDI_C */
