;*      drawasm.nas
;*
;* Assembler optimized drawing routines for DOOM
;*
;* $Id: drawasm.nas,v 1.1 1998/01/06 16:37:11 pekangas Exp $
;*
;* Written by Petteri Kangaslampi <pekangas@sci.fi>
;* Donated to the public domain.

BITS 32

; Segment/section definition macros. 

%ifdef M_TARGET_WATCOM
  SEGMENT DATA PUBLIC ALIGN=16 CLASS=DATA USE32
  SEGMENT DATA
%else
  SECTION .data
%endif

; External variables:
EXTERN _ds_xstep
EXTERN _ds_ystep
EXTERN _ds_colormap
EXTERN _ds_source

; Local stuff:
lastAddress DD 0


%ifdef M_TARGET_WATCOM
  SEGMENT CODE PUBLIC ALIGN=16 CLASS=CODE USE32
  SEGMENT CODE
%else
  SECTION .text
%endif


GLOBAL _DrawSpan8Loop

;void DrawSpan8Loop(fixed_t xfrac, fixed_t yfrac, int count, byte *dest);

_DrawSpan8Loop:
	push	ebp
	push	edi
	push	esi
	push	ebx

	mov	ebx,[esp+20]	; xfrac
	mov	ecx,[esp+24]	; yfrac
	mov	eax,[esp+28]	; count
	mov	edi,[esp+32]	; dest
	add	eax,edi
	mov	[lastAddress],eax
	
	mov	ebp,ebx
	mov	edx,ecx
	shr	edx,10
	and	ebp,65536*63

	shr	ebp,16
	xor	eax,eax

.loo:
	and	edx,64*63
	mov	esi,[_ds_source]

	or	ebp,edx
	mov	edx,[_ds_xstep]

	add	ebx,edx
	mov	edx,[_ds_ystep]

	mov	al,[esi+ebp]
	mov	esi,[_ds_colormap]

	add	ecx,edx
	mov	ebp,ebx

	mov	edx,ecx
	mov	al,[esi+eax]

	shr	edx,10
	mov	[edi],al

	shr	ebp,16
	and	edx,64*63

	and	ebp,63
	mov	esi,[_ds_source]

	or	ebp,edx
	mov	edx,[_ds_xstep]

	add	ebx,edx
	mov	edx,[_ds_ystep]

	mov	al,[esi+ebp]
	mov	esi,[_ds_colormap]

	add	ecx,edx
	mov	ebp,ebx

	mov	edx,ecx
	mov	al,[esi+eax]

	shr	edx,10
	mov	[edi+1],al

	shr	ebp,16
	mov	esi,[lastAddress]

	and	ebp,63
	add	edi,2

	cmp	edi,esi
	jne	near .loo

	pop	ebx
	pop	esi
	pop	edi
	pop	ebp

	ret



;* $Log: drawasm.nas,v $
;* Revision 1.1  1998/01/06 16:37:11  pekangas
;* Initial revision
;*
