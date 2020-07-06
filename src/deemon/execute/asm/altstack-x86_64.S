
#define __SIZEOF_POINTER__        8
#define DEE_EXEC_ALTSTACK_SIZE   (__SIZEOF_POINTER__*1024*512)

#ifndef _MSC_VER
#error "Only meant for MSVC version"
#endif /* !_MSC_VER */

PUBLIC  DeeCode_ExecFrameSafeAltStack
PUBLIC  DeeCode_ExecFrameFastAltStack
EXTRN   free_altstack:NEAR
EXTRN   alloc_altstack:NEAR
EXTRN   DeeCode_ExecFrameSafe:NEAR
EXTRN   DeeCode_ExecFrameFast:NEAR

_TEXT   SEGMENT

/* NOTE: The +/- 32 is required because it is part
 *       of Microsoft's x86_64 calling convention:
 * https://en.wikipedia.org/wiki/X86_calling_conventions#Microsoft_x64_calling_convention
 */

DeeCode_ExecFrameSafeAltStack PROC
	push  RCX
	sub   RSP, 32
	call  alloc_altstack
	add   RSP, 32
	pop   RCX
	test  RAX, RAX
	jz    Lexit
	push  RAX /* Preserve the alt-stack base-pointer */
	push  RBX
	mov   RBX, RSP
	lea   RSP, [RAX + DEE_EXEC_ALTSTACK_SIZE - 32]
	call  DeeCode_ExecFrameSafe
	mov   RSP, RBX
	pop   RBX
	mov   RCX, RAX   /* result */
	xchg  [RSP], RCX /* Put the result onto the stack & get the alt-stack pointer. */
	sub   RSP, 32
	call  free_altstack
	add   RSP, 32
	pop   RAX        /* pop the result */
Lexit:
	ret
DeeCode_ExecFrameSafeAltStack ENDP

DeeCode_ExecFrameFastAltStack PROC
	push  RCX
	sub   RSP, 32
	call  alloc_altstack
	add   RSP, 32
	pop   RCX
	test  RAX, RAX
	jz    Lexit
	push  RAX /* Preserve the alt-stack base-pointer */
	push  RBX
	mov   RBX, RSP
	lea   RSP, [RAX + DEE_EXEC_ALTSTACK_SIZE - 32]
	call  DeeCode_ExecFrameFast
	mov   RSP, RBX
	pop   RBX
	mov   RCX, RAX   /* result */
	xchg  [RSP], RCX /* Put the result onto the stack & get the alt-stack pointer. */
	sub   RSP, 32
	call  free_altstack
	add   RSP, 32
	pop   RAX        /* pop the result */
Lexit:
	ret
DeeCode_ExecFrameFastAltStack ENDP

_TEXT   ENDS

END

