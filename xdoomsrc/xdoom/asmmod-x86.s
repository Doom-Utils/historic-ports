//
// vi:set tabstop=8:
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Copyright (C) 1998 by Lee Killough
// Copyright (C) 1998-1999 by Udo Munk
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
//
// $Log:$
//
// DESCRIPTION:
//	Assembler routines for generic x86 CPU's, not specially optimized
//	for any CPU, should work OK on 386, 486, Pentium and Pentium Pro
//	and the clones from third party manufacturers. Alignment is optimized
//	for Pentium though, but that won't hurt on [34]86 CPU's.
//

//================
//
// R_DrawSpan
//
// Horizontal texture mapping
//
//================
//
// 2/14/98 Lee Killough
//
// Ported from the released linux source
// Converted from Intel to AT&T syntax
// Replaced self-modifying code with code that uses stack
// Removed dependence on 256-byte-aligned colormaps
//
// 12/20/98 Udo Munk
// Modified, so that it will assemble with UNIX assembler too
//================

.text

.align 8
.globl R_DrawSpan

R_DrawSpan:
	pushl %ebp
	pushl %esi
	pushl %edi
	pushl %ebx

//
// find loop count
//

// count = ds_x2 - ds_x1 + 1;
	movl ds_x2,%eax
	incl  %eax

// pixel count
	subl ds_x1,%eax

// nothing to scale
	jle  RDS9

// pixel count
	pushl %eax

//
// build composite position
//
	movl ds_xfrac,%ebp
	shll $10,%ebp
	andl $0x0ffff0000,%ebp
	movl ds_yfrac,%eax
	shrl $6,%eax
	andl $0xffff,%eax
	orl  %eax,%ebp

//
// calculate screen dest
//

//  dest = ylookup[ds_y] + columnofs[ds_x1];
	movl ds_y,%ebx
	movl ds_x1,%eax
	movl ds_source,%esi
	movl ylookup(,%ebx,4),%edi
	addl columnofs(,%eax,4),%edi

//
// build composite step
//
	movl ds_xstep,%ebx
	shll $10,%ebx
	andl $0xffff0000,%ebx
	movl ds_ystep,%eax
	shrl $6,%eax
	andl $0xffff,%eax
	orl  %eax,%ebx
	pushl %ebx

// %eax, %ebx, %ecx,%edx scratch
// %esi  virtual source
// %edi  moving destination pointer
// %ebp  frac

// begin calculating third pixel (y units)
	shldl $22,%ebp,%ecx

// begin calculating third pixel (x units)
	shldl $6,%ebp,%ecx

// advance frac pointer
	addl  %ebx,%ebp

// finish calculation for third pixel
	andl  $4095,%ecx

// begin calculating fourth pixel (y units)
	shldl $22,%ebp,%edx

// begin calculating fourth pixel (x units)
	shldl $6,%ebp,%edx

// advance frac pointer
	addl %ebx,%ebp

// finish calculation for fourth pixel
	andl $4095,%edx

// get first pixel
	xorl %eax,%eax
	movb (%esi,%ecx),%al

// get second pixel
	xorl %ebx,%ebx
	movb (%esi,%edx),%bl

// offset first pixel into colormap
	addl ds_colormap,%eax

// offset second pixel into colormap
	addl ds_colormap,%ebx
	subl $2,4(%esp)

// color translate first pixel
	movb (%eax),%al

// color translate second pixel
	movb (%ebx),%bl
	jl RDS8

// at least two pixels to map
.align 8
RDS1:

// begin calculating third pixel (y units)
	shldl $22,%ebp,%ecx

// begin calculating third pixel (x units)
	shldl $6,%ebp,%ecx

// advance frac pointer
	addl (%esp),%ebp

// write first pixel
	movb %al,(%edi)

// finish calculation for third pixel
	andl $4095,%ecx

// begin calculating fourth pixel (y units)
	shldl $22,%ebp,%edx

// begin calculating fourth pixel (x units)
	shldl $6,%ebp,%edx

// advance frac pointer
	addl (%esp),%ebp

// finish calculation for fourth pixel
	andl $4095,%edx

// write second pixel
	movb %bl,1(%edi)

// get third pixel
	xorl %eax,%eax
	movb (%esi,%ecx),%al

// get fourth pixel
	xorl %ebx,%ebx
	movb (%esi,%edx),%bl

// advance to third pixel destination
	addl $2,%edi

// offset third pixel into colormap
	addl ds_colormap,%eax

// offset fourth pixel into colormap
	addl ds_colormap,%ebx

// done with loop?
	subl $2,4(%esp)

// color translate third pixel
	movb (%eax),%al

// color translate fourth pixel
	movb (%ebx),%bl
	jge RDS1

// check for final pixel
.align 8
RDS8:
	popl %ecx
	popl %edx
	jnp RDS9

// write final pixel
	movb %al,(%edi)
.align 8
RDS9:
	popl %ebx
	popl %edi
	popl %esi
	popl %ebp
	ret

//================
//
// R_DrawColumn
//
//================
//
// 2/15/98 Lee Killough
//
// Converted C code with TFE fix to assembly and tuned
//
// 12/20/98 Udo Munk
// Modified, so that it will assemble with UNIX assembler too
//================

.align 8
.globl R_DrawColumn

R_DrawColumn:
	pushl %ebp
	pushl %esi
	pushl %edi
	pushl %ebx
	movl dc_yh,%esi
	movl dc_yl,%edx
	movl dc_x,%eax
	incl %esi
	movl ylookup(,%edx,4),%ebx
	subl %edx,%esi
	jle RDC9
	addl columnofs(,%eax,4),%ebx
	movl dc_texheight,%eax
	subl centery,%edx
	movl dc_source,%ebp
	imull dc_iscale,%edx
	leal -1(%eax),%ecx
	movl dc_colormap,%edi
	addl dc_texturemid,%edx
	cmpl $128,%eax
	je RDC7
	testl %eax,%ecx
	je RDC5
	sall $16,%eax

RDC0:
	subl %eax,%edx
	jge RDC0

RDC1:
	addl %eax,%edx
	jl RDC1

.align 8
RDC2:
	movl %edx,%ecx
	sarl $16,%ecx
	addl dc_iscale,%edx
	movzbl (%ecx,%ebp),%ecx
	movb (%edi,%ecx),%cl
	movb %cl,(%ebx)
	addl $320,%ebx
	cmpl %eax,%edx
	jge RDC3
	decl %esi
	jg RDC2
	popl %ebx
	popl %edi
	popl %esi
	popl %ebp
	ret

.align 8
RDC3:
	subl %eax,%edx
	decl %esi
	jg RDC2
	popl %ebx
	popl %edi
	popl %esi
	popl %ebp
	ret

.align 8
RDC9:
	popl %ebx
	popl %edi
	popl %esi
	popl %ebp
	ret

.align 8
RDC4:
	movl %edx,%eax
	addl dc_iscale,%edx
	sarl $16,%eax
	andl %ecx,%eax
	movzbl (%eax,%ebp),%eax
	movb (%eax,%edi),%al
	movb %al,(%ebx)
	movl %edx,%eax
	addl dc_iscale,%edx
	sarl $16,%eax
	andl %ecx,%eax
	movzbl (%eax,%ebp),%eax
	movb (%eax,%edi),%al
	movb %al,320(%ebx)
	addl $640,%ebx

.align 8
RDC5:
	addl $-2,%esi
	jge RDC4
	jnp RDC9
	sarl $16,%edx
	andl %ecx,%edx
	xorl %eax,%eax
	movb (%edx,%ebp),%al
	movb (%eax,%edi),%al
	movb %al,(%ebx)
	popl %ebx
	popl %edi
	popl %esi
	popl %ebp
	ret

.align 8
RDC7:
	; movl dc_iscale, %ecx
	andl $0x007fffff,%edx
	xorl %ecx,%ecx
	addl $-2, %esi
	jl RDC8

.align 8
RDC6:
	movl %edx,%eax
	sarl $16,%edx
	addl dc_iscale,%eax
	andl $0x007fffff,%eax
	movb (%edx,%ebp),%cl
	movl %eax,%edx
	sarl $16,%eax
	movb (%ecx,%edi),%cl
	addl dc_iscale,%edx
	movb %cl,(%ebx)
	movb (%eax,%ebp),%cl
	addl $640,%ebx
	andl $0x007fffff,%edx
	movb (%ecx,%edi),%cl
	addl $-2,%esi
	movb %cl,-320(%ebx)
	jge RDC6

RDC8:
.align 8
	jnp RDC9
	sarl $16,%edx
	andl $127,%edx
	xorl %eax,%eax
	movb (%edx,%ebp),%al
	movb (%eax,%edi),%al
	movb %al,(%ebx)
	popl %ebx
	popl %edi
	popl %esi
	popl %ebp
	ret

//================
//
// R_DrawTranslucentColumn
//
//================
//
// 2/15/98 Lee Killough
//
// Converted C code with TFE fix to assembly and tuned
//
// 12/20/98 Udo Munk
// Modified, so that it will assemble with UNIX assembler too
//================

.align 8
.globl R_DrawTranslucentColumn

R_DrawTranslucentColumn:
	pushl %ebp
	pushl %esi
	pushl %edi
	pushl %ebx
	movl dc_yh,%esi
	movl dc_yl,%edx
	movl dc_x,%eax
	incl %esi
	movl ylookup(,%edx,4),%ebx
	subl %edx,%esi
	jle RDTC9
	addl columnofs(,%eax,4),%ebx
	movl dc_texheight,%eax
	subl centery,%edx
	movl dc_source,%ebp
	imull dc_iscale,%edx
	leal -1(%eax),%ecx
	movl dc_colormap,%edi
	addl dc_texturemid,%edx
	testl %eax,%ecx
	pushl %ecx
	je RDTC5
	sall $16,%eax

RDTC0:
	subl %eax,%edx
	jge RDTC0

RDTC1:
	addl %eax,%edx
	jl RDTC1
	pushl %esi

.align 8
RDTC2:
	xorl %ecx,%ecx
	movl %edx,%esi
	movb (%ebx),%cl
	shll $8,%ecx
	sarl $16,%esi
	addl tranmap,%ecx
	addl dc_iscale,%edx
	movzbl (%esi,%ebp),%esi
	movzbl (%edi,%esi),%esi
	movb (%ecx,%esi),%cl
	movb %cl,(%ebx)
	addl $320,%ebx
	cmpl %eax,%edx
	jge RDTC3
	decl (%esp)
	jg RDTC2
	popl %eax
	popl %ecx
	popl %ebx
	popl %edi
	popl %esi
	popl %ebp
	ret

.align 8
RDTC3:
	subl %eax,%edx
	decl (%esp)
	jg RDTC2
	popl %eax
	popl %ecx
	popl %ebx
	popl %edi
	popl %esi
	popl %ebp
	ret

.align 8
RDTC9:
	popl %ecx
	popl %ebx
	popl %edi
	popl %esi
	popl %ebp
	ret

.align 8
RDTC4:
	movl %edx,%eax
	xorl %ecx,%ecx
	addl dc_iscale,%edx
	movb (%ebx),%cl
	sarl $16,%eax
	shll $8,%ecx
	andl (%esp),%eax
	addl tranmap,%ecx
	movzbl (%eax,%ebp),%eax
	movzbl (%edi,%eax),%eax
	movb   (%ecx,%eax),%al
	xorl %ecx,%ecx
	movb %al,(%ebx)
	movb 320(%ebx),%cl
	movl %edx,%eax
	addl dc_iscale,%edx
	sarl $16,%eax
	shll $8,%ecx
	andl (%esp),%eax
	addl tranmap,%ecx
	movzbl (%eax,%ebp),%eax
	movzbl (%edi,%eax),%eax
	movb   (%ecx,%eax),%al
	movb %al,320(%ebx)
	addl $640,%ebx

.align 8
RDTC5:
	addl $-2,%esi
	jge RDTC4
	jnp RDTC9
	xorl %ecx,%ecx
	sarl $16,%edx
	movb (%ebx),%cl
	andl (%esp),%edx
	shll $8,%ecx
	xorl %eax,%eax
	addl tranmap,%ecx
	movb (%edx,%ebp),%al
	movzbl (%eax,%edi),%eax
	movb (%ecx,%eax),%al
	movb %al,(%ebx)
	popl %ecx
	popl %ebx
	popl %edi
	popl %esi
	popl %ebp
	ret

//================
// void port_out(int value, int port)
//
// UM 1999-02-17
// For Unixware, VGAIO is to slow
//================
.align 8
port_out:
.globl port_out
	push	%ebp
	mov	%esp,%ebp
	push	%edi
	push	%ebx

	movl	8(%ebp),%eax
	movl	12(%ebp),%edx
	outb	(%dx)

	pop	%ebx
	pop	%edi
	pop	%ebp
	ret

//================
// int port_in(int port)
//
// UM 1999-02-17
// For Unixware, VGAIO is to slow
//================
.align 8
port_in:
.globl port_in
	push	%ebp
	mov	%esp,%ebp
	push	%edi
	push	%ebx

	movl	8(%ebp),%edx
	sub	%eax,%eax
	inb	(%dx)

	pop	%ebx
	pop	%edi
	pop	%ebp
	ret
