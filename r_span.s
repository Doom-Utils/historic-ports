.comm _ds_y,4
.comm _ds_x1,4
.comm _ds_x2,4
.comm _ds_colormap,4
.comm _ds_xfrac,4
.comm _ds_yfrac,4
.comm _ds_xstep,4
.comm _ds_ystep,4
.comm _ylookup,4
.comm _columnofs,4
.comm _ds_source,4
.text

//
// id's code. Probably optimal for 486.
// By id software, ported by ES 1998-08-11
//
// -ES- 1998/08/25 Fixed alignment

.align	4
.globl _R_DrawSpan8_id
_R_DrawSpan8_id:
        pushl %esi
	pushl %edi
        pushl %ebp
	pushl %ebx

//
// find loop count
//	
	movl _ds_x2, %eax
	incl %eax
	subl _ds_x1, %eax          	        // pixel count
	movl %eax, _pixelcount			// save for final pixel
	js 0xDeadBeef				// nothing to scale
.globl rds8ioffs1
rds8ioffs1: // rds8idone
	shrl $1, %eax			        // double pixel count
	movl %eax, _loopcount

//
// build composite position
//
	movl	_ds_xfrac, %ebp
	shll	$10, %ebp
	andl	$0xffff0000, %ebp
	movl	_ds_yfrac, %eax
	shrl	$6, %eax
	andl	$0xFFFF, %eax
	orl	%eax,%ebp

	movl	_ds_source,%esi

//
// calculate screen dest
//
        movl _ylookup, %eax
	movl _ds_y,%edi
	movl (%eax,%edi,4), %edi
	movl _ds_x1,%eax
	movl _columnofs,%ebx
	addl (%ebx,%eax,4),%edi

//
// build composite step
//
	movl	_ds_xstep,%ebx
	shll	$10,%ebx
	andl	$0xffff0000,%ebx
	movl	_ds_ystep,%eax
	shrl	$6,%eax
	andl	$0xffff,%eax
	orl	%eax,%ebx

	movl %ebx, (rds8ipatch1-4) // patch imms, to free some regs
.globl rds8ipatcher1
rds8ipatcher1:
	movl %ebx, (rds8ipatch2-4)
.globl rds8ipatcher2
rds8ipatcher2:
	
// eax		aligned colormap
// ebx		aligned colormap
// ecx,edx	scratch
// esi		virtual source
// edi		moving destination pointer
// ebp		frac
	
	shldl $22,%ebp,%ecx				// begin calculating third pixel (y units)
	shldl $6,%ebp,%ecx				// begin calculating third pixel (x units)
	addl	%ebx,%ebp					// advance frac pointer
	andl $4095,%ecx				// finish calculation for third pixel
	shldl $22,%ebp,%edx				// begin calculating fourth pixel (y units)
	shldl $6,%ebp,%edx				// begin calculating fourth pixel (x units)
	addl	%ebx,%ebp					// advance frac pointer
	andl $4095,%edx				// finish calculation for fourth pixel
	movl (_ds_colormap),%eax
	movl %eax,%ebx
	movb	(%esi,%ecx),%al			// get first pixel
	movb	(%esi,%edx),%bl			// get second pixel
	movb	(%eax),%al				// color translate first pixel
	movb	(%ebx),%bl				// color translate second pixel

	testl	$0xfffffffe, _pixelcount
	jnz	0xDeadBeef				// at least two pixels to map
.globl rds8ioffs2
rds8ioffs2: // rds8iloop
	jmp	0xDeadBeef
.globl rds8ioffs3
rds8ioffs3: // rds8ichecklast
	

.globl rds8iloop
rds8iloop:
	shldl $22,%ebp,%ecx				// begin calculating third pixel (y units)
	shldl $6,%ebp,%ecx				// begin calculating third pixel (x units)
	addl  $0xDeadBeef,%ebp			// advance frac pointer
.globl rds8ipatch1
rds8ipatch1:
	movb %al, (%edi)				// write first pixel
	andl $4095,%ecx				// finish calculation for third pixel
	shldl $22,%ebp,%edx				// begin calculating fourth pixel (y units)
	shldl $6,%ebp,%edx				// begin calculating fourth pixel (x units)
	addl  $0xDeadBeef,%ebp			// advance frac pointer
.globl rds8ipatch2
rds8ipatch2:
	movb %bl, 1(%edi)				// write second pixel
	andl $4095,%edx				// finish calculation for fourth pixel
	movb (%esi,%ecx), %al			// get third pixel
	addl $2,%edi					// advance to third pixel destination
	movb (%esi,%edx),%bl			// get fourth pixel
	decl _loopcount				// done with loop?
	movb (%eax),%al				// color translate third pixel
	movb (%ebx),%bl				// color translate fourth pixel
	jnz	rds8iloop

// check for final pixel
.globl rds8ichecklast
rds8ichecklast:
	testl	$1,_pixelcount
	jz	rds8idone
	movb %al,(%edi)				// write final pixel
	
.globl rds8idone
rds8idone:
	popl %ebx
        popl %ebp
	popl %edi
        popl %esi
	ret

.globl R_DrawSpan8_id_end
R_DrawSpan8_id_end:
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;


//
// id's code, further optimised by ES 1998/08/11
//

.align	4
.globl _R_DrawSpan8_id_Erik
_R_DrawSpan8_id_Erik:
        pushl %esi
	pushl %edi
        pushl %ebp
	pushl %ebx

//
// find loop count
//	
	movl _ds_x2, %eax
	incl %eax
	subl _ds_x1, %eax          	        // pixel count
	movl %eax, _pixelcount			// save for final pixel
	js 0xDeadBeef				// nothing to scale
.globl rds8eoffs1
rds8eoffs1: // rds8edone
	shrl $1, %eax			        // double pixel count
	movl %eax, _loopcount

//
// build composite position
//
	movl	_ds_xfrac, %ebp
	shll	$10, %ebp
	andl	$0xffff0000, %ebp
	movl	_ds_yfrac, %eax
	shrl	$6, %eax
	andl	$0xFFFF, %eax
	orl	%eax,%ebp

	movl	_ds_source,%esi

//
// calculate screen dest
//
        movl _ylookup, %eax
	movl _ds_y,%edi
	movl (%eax,%edi,4), %edi
	movl _ds_x1,%eax
	movl _columnofs,%ebx
	addl (%ebx,%eax,4),%edi

//
// build composite step
//
	movl	_ds_xstep,%ebx
	shll	$10,%ebx
	andl	$0xffff0000,%ebx
	movl	_ds_ystep,%eax
	shrl	$6,%eax
	andl	$0xffff,%eax
	orl	%eax,%ebx
/* ---------------------------------------------------- */

	movl %ebx, (rds8epatch1-4) // patch imms, to free some regs
.globl rds8epatcher1
rds8epatcher1:
	movl %ebx, (rds8epatch2-4)
.globl rds8epatcher2
rds8epatcher2:
	movl %esi, (rds8epatch3-4)
.globl rds8epatcher3
rds8epatcher3:
	movl %esi, (rds8epatch4-4)
.globl rds8epatcher4
rds8epatcher4:
	
// eax		aligned colormap
// ebx		aligned colormap
// ecx,edx	scratch
// esi		virtual source
// edi		moving destination pointer
// ebp		frac
	
	shldl $22,%ebp,%ecx				// begin calculating third pixel (y units)
	shldl $6,%ebp,%ecx				// begin calculating third pixel (x units)
	addl	%ebx,%ebp					// advance frac pointer
	andl $4095,%ecx				// finish calculation for third pixel
	shldl $22,%ebp,%edx				// begin calculating fourth pixel (y units)
	shldl $6,%ebp,%edx				// begin calculating fourth pixel (x units)
	addl	%ebx,%ebp					// advance frac pointer
	andl $4095,%edx				// finish calculation for fourth pixel
	movl (_ds_colormap),%eax
	movl %eax,%ebx
	movb	(%esi,%ecx),%al			// get first pixel
	movb	(%esi,%edx),%bl			// get second pixel
	movb	(%eax),%al				// color translate first pixel
	movb	(%ebx),%bl				// color translate second pixel

	shldl $22,%ebp,%ecx				// begin calculating third pixel (y units)
        movl _loopcount,%esi

	testl	$0xfffffffe, _pixelcount
	jnz	0xDeadBeef 				// at least two pixels to map
.globl rds8eoffs2
rds8eoffs2: // rds8eloop
	jmp	0xDeadBeef
.globl rds8eoffs3
rds8eoffs3: // rds8echecklast
	
.globl rds8eloop
rds8eloop:
	shldl $6,%ebp,%ecx				// begin calculating third pixel (x units)
	addl  $0xDeadBeef,%ebp			// advance frac pointer
.globl rds8epatch1
rds8epatch1:
	andl $4095,%ecx				// finish calculation for third pixel
	shldl $22,%ebp,%edx				// begin calculating fourth pixel (y units)
	movb %al, (%edi)				// write first pixel
	movb %bl, 1(%edi)				// write second pixel
	addl $2,%edi					// advance to third pixel destination
	shldl $6,%ebp,%edx				// begin calculating fourth pixel (x units)
	andl $4095,%edx				// finish calculation for fourth pixel
	movb 0xDeadBeef(%ecx), %al			// get third pixel
.globl rds8epatch3
rds8epatch3:
	movb 0xDeadBeef(%edx),%bl			// get fourth pixel
.globl rds8epatch4
rds8epatch4:
	addl  $0xDeadBeef,%ebp			// advance frac pointer
.globl rds8epatch2
rds8epatch2:
	shldl $22,%ebp,%ecx				// begin calculating third pixel (y units)
	movb (%eax),%al				// color translate third pixel
	movb (%ebx),%bl				// color translate fourth pixel
	decl %esi				// done with loop?
	jnz	rds8eloop

// check for final pixel
.globl rds8echecklast
rds8echecklast:
	testl	$1,_pixelcount
	jz	rds8edone
	movb %al,(%edi)				// write final pixel

.globl rds8edone
rds8edone:
	popl %ebx
        popl %ebp
	popl %edi
        popl %esi
	ret

.globl R_DrawSpan8_id_Erik_end
R_DrawSpan8_id_Erik_end:
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;


        .align 4
.globl _R_DrawSpan8_Rasem
_R_DrawSpan8_Rasem:
        pushl %esi
	pushl %edi
        pushl %ebp
	pushl %ebx
	movl _ds_xfrac,%edi
        movl _ds_yfrac,%ebp
	movl _ds_y,%ebx
	movl _columnofs,%eax
	movl _ds_x1,%edx
        movl _ylookup,%esi
	movl (%eax,%edx,4),%eax
        movl (%esi,%ebx,4),%ebx
        movl _ds_x2,%esi
	addl %eax,%ebx
        subl %edx,%esi

        .align 2,0x90

L2:
        movl %ebp,%edx
	movl %edi,%eax
	sarl $10,%edx
	sarl $16,%eax
	andl $4032,%edx
	andl $63,%eax
	addl %eax,%edx
	movl _ds_source,%eax
        xorl %ecx,%ecx
	addl _ds_xstep,%edi
        movb (%edx,%eax),%cl
	movl _ds_colormap,%eax
        movb (%ecx,%eax),%al
        addl _ds_ystep,%ebp
	movb %al,(%ebx)
	incl %ebx
        subl $1,%esi
	jnc L2
	popl %ebx
        popl %ebp
	popl %edi
        popl %esi
        ret


// -ES- 1998/07/24 Wrote MMX version
.globl _R_DrawSpan8_MMX
_R_DrawSpan8_MMX:
    pushl %esi
    pushl %edi
    pushl %ebp
    pushl %ebx

    movl %esp, %eax // Push 8 or 12, so that (%esp) gets aligned by 8
    andl $7,%eax
    addl $8,%eax
    movl %eax, _mmxcomm // Temp storage in mmxcomm: (%esp) is used instead
    subl %eax,%esp

        movl _ds_source, %esi               // src in esi
        movl _ds_y, %eax
        movl _ylookup, %ebx
        movl (%ebx, %eax, 4), %edi
        movl _ds_x2, %eax
        movl _columnofs, %ebx
        addl (%ebx, %eax, 4), %edi        // dest in edi
        
        movl _ds_x1, %ecx
        subl %eax, %ecx                    // count in ecx
        
        movl _ds_xfrac, %eax                // convert to 6.26 fix
        movl _ds_yfrac, %ebx
        shll $10, %eax
        shll $10, %ebx
        shll $10, _ds_xstep
        shll $10, _ds_ystep
        
        leal -1(%edi, %ecx), %ebp         // Load dest to L1 cache
        andl $~31, %ebp                     // (speeds up Pentium MMX)
        subl %edi, %ebp
        jng rds8mload_done
rds8mload:
        movb (%edi, %ebp), %dl
        subl $32, %ebp
        jg rds8mload
rds8mload_done:
        movb (%edi, %ebp), %dl
        
        cmpl $-4, %ecx
        jl rds8mmany
        // Less than six pixels to draw, write one pixel at a time without MMX to
        // avoid MMX register setup and loop initialisation
rds8mfew:
        movl %ebx, %ebp
        shrl $20, %ebp
        andl $0xFC0, %ebp
        movl %eax, %edx
        shrl $26, %edx
        addl %edx, %ebp
        movzbl (%esi, %ebp), %edx
        movl _ds_colormap, %ebp
        movb (%ebp, %edx), %dl
        movb %dl, (%edi,%ecx)
        addl _ds_xstep, %eax
        addl _ds_ystep, %ebx
        incl %ecx
        jle rds8mfew ;
        jmp rds8mdone
.globl rds8moffs1
rds8moffs1:

rds8mmany:
        movl $0xFC000000, 0(%esp)
        movl $0xFC000000, 4(%esp)
        movq 0(%esp), %mm7
        
        movl %eax, 0(%esp)
        addl _ds_xstep, %eax
        movl %eax, 4(%esp)
        movq 0(%esp), %mm0                 // xfrac in mm0
        
        movl %ebx, 0(%esp)
        addl _ds_ystep, %ebx
        movl %ebx, 4(%esp)
        movq 0(%esp), %mm1                 // yfrac in mm1
        
        movl _ds_xstep, %eax
        addl %eax, %eax
        movl %eax, 0(%esp)
        movl %eax, 4(%esp)
        movq 0(%esp), %mm2                 // xstep in mm2
        
        movl _ds_ystep, %ebx
        addl %ebx, %ebx
        movl %ebx, 0(%esp)
        movl %ebx, 4(%esp)
        movq 0(%esp), %mm3                 // ystep in mm3
        
        movl _ds_colormap, %eax
        movl %eax, %ebx
        
// The loop handles 10 pixels at a time. We must now prepare the eight first
// ones. This is extremely messy, but fast :-)
        
        // Prepare pixels 0-1
        movq %mm0, %mm4
        paddd %mm2, %mm0
        psrld $26, %mm4
        movq %mm1, %mm5
        paddd %mm3, %mm1
        .byte 0x0F,0xDB,0xEF  // GCC thinks that pand %mm7, %mm5 is written 0x0FDAEF, but it isn't
        psrld $20, %mm5
        por %mm5, %mm4
        movq %mm4, 0(%esp)
        movl 0(%esp), %ebp
        movl 4(%esp), %edx
        movb (%esi, %ebp), %al
        movb (%esi, %edx), %bl
        
        // Prepare pixels 2-3
        movq %mm0, %mm4
        paddd %mm2, %mm0
        psrld $26, %mm4
        movq %mm1, %mm5
        paddd %mm3, %mm1
        .byte 0x0F,0xDB,0xEF  // GCC thinks that pand %mm7, %mm5 is written 0x0FDAEF, but it isn't
        psrld $20, %mm5
        por %mm5, %mm4
        movq %mm4, 0(%esp)
        movl 0(%esp), %ebp
        movl 4(%esp), %edx
        
        // Prepare pixels 4-5
        movq %mm0, %mm4
        psrld $26, %mm4
        movq %mm1, %mm5
        .byte 0x0F,0xDB,0xEF  // GCC thinks that pand %mm7, %mm5 is written 0x0FDAEF, but it isn't
        psrld $20, %mm5
        
        // Prepare pixels 6-7
        paddd %mm3, %mm1
        
        incl %ecx
        
.globl rds8mloop
rds8mloop:
        paddd %mm2, %mm0                   // 3
        movb 0(%eax), %al                  // 3
        
        por %mm5, %mm4                     // 3
        movb %al, 1-2(%edi, %ecx)         // 4
        
        movq %mm1, %mm5                    // 3
        .byte 0x8A,0x5C,0x23,0x00            // 4-byte version of movb (%ebx), %bl;
        
        movq %mm4, 0(%esp)                 // 7
        movb 0(%esi, %ebp), %al           // 4
        
        paddd %mm3, %mm1                   // 3
        movb %bl, 2-2(%edi, %ecx)         // 4
        
        movq %mm0, %mm4                    // 3
        movb 0(%esi, %edx), %bl            // 4
        
        .byte 0x0F,0xDB,0xEF  // GCC thinks that pand %mm7, %mm5 is written 0x0FDAEF, but it isn't
        movl 0(%esp), %ebp                 // 6
        
        psrld $26, %mm4                     // 4
        movl 4(%esp), %edx               // 6
        
        psrld $20, %mm5                     // 4
        
        addl $2, %ecx                           // 1 // I would prefer to use LOOP instead, but PMMX wouldn't...
        jl rds8mloop                         // 2
        movb (%eax), %al                  // Write last 1 or 2 pixel(s)
        movb %al, 1-2(%edi, %ecx)         // 4
        jg rds8mlastodd
        movb (%ebx), %ah                  // 3
        movb %ah, 2-2(%edi, %ecx)         // 4
rds8mlastodd:
        
.globl rds8mdone;
rds8mdone:
    emms  // fixme: To end of rendering

    addl _mmxcomm, %esp

    popl %ebx
    popl %ebp
    popl %edi
    popl %esi
    ret
        
.globl R_DrawSpan8MMX_end
R_DrawSpan8MMX_end:
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;nop;


.globl _R_DrawSpan16_Rasem
_R_DrawSpan16_Rasem:
	pushl %ebp
	pushl %edi
	pushl %esi
	pushl %ebx
	movl _ds_xfrac,%edi
	movl _ds_yfrac,%esi
	movl _ds_y,%ebx
	movl _ylookup,%ecx
	movl _ds_x1,%edx
	movl _columnofs,%eax
	movl (%eax,%edx,4),%eax
	movl (%ecx,%ebx,4),%ebx
	addl %eax,%ebx
	movl _ds_x2,%ecx
	subl %edx,%ecx
	movl _ds_colormap,%ebp

        .align 2,0x90

L3:
	movl %esi,%edx
	movl %edi,%eax
	sarl $10,%edx
	sarl $16,%eax
	andl $4032,%edx
	andl $63,%eax
	addl %eax,%edx
	movl _ds_source,%eax
	addl _ds_xstep,%edi
	addl _ds_ystep,%esi
        movb (%edx,%eax),%al
        andl $255,%eax
	movw (%ebp,%eax,2),%ax
	movw %ax,(%ebx)
	addl $2,%ebx
	subl $1,%ecx
        jnc L3
	popl %ebx
	popl %esi
	popl %edi
	popl %ebp
	ret
