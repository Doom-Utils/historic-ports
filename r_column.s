.comm _dc_colormap,4
.comm _dc_x,4
.comm _dc_yl,4
.comm _dc_yh,4
.comm _dc_iscale,4
.comm _dc_texturemid,4
.comm _SCREENWIDTH,4
.comm _SCREENHEIGHT,4
.comm _dc_source,4
.comm _ylookup,4
.comm _columnofs,4
.comm _loopcount,4
.comm _pixelcount,4
.data
_pixelcount:
.long 0x00000000
_loopcount:
.long 0x00000000
.text

// -ES- 1999/03/19 Removed centery subtractions

//
// id's code. Probably optimal for 486.
// By id software, ported by ES 1998-08-04
//
// -ES- 1998/08/25 Fixed alignment

        .align 4
.globl _R_DrawColumn8_id
_R_DrawColumn8_id:
   pushl %ebp
   pushl %esi
   pushl %edi
   pushl %ebx
        movl _dc_yl,%ebp
        movl %ebp,%ebx
        movl _ylookup,%edi
        movl (%edi,%ebx,4),%edi
        movl _dc_x,%ebx
        movl _columnofs,%eax
        addl (%eax,%ebx,4),%edi
        
        movl _dc_yh,%eax
        incl %eax
        subl %ebp,%eax
        movl %eax,_pixelcount
        js 0xDeadBeef
.globl rdc8ioffs1
rdc8ioffs1: // rdc8idone
        shrl $1,%eax
        movl %eax,_loopcount
        
        movl _dc_iscale,%ecx
        
        xorl %eax,%eax
        subl %ebp,%eax
        imull	%ecx
        movl _dc_texturemid,%ebp
        subl %eax,%ebp
        shll $9,%ebp

        movl _dc_source,%esi	
        
        movl _dc_iscale,%ebx
        shll $9,%ebx
        movl $rdc8ipatch1-4,%eax
.globl rdc8ipatcher1
rdc8ipatcher1:
        movl %ebx,(%eax)
        movl $rdc8ipatch2-4,%eax
.globl rdc8ipatcher2
rdc8ipatcher2:
        movl %ebx,(%eax)
        
        movl %ebp,%ecx
        addl %ebx,%ebp
        shrl $25,%ecx
        movl %ebp,%edx
        addl %ebx,%ebp
        shrl $25,%edx
        movl _dc_colormap,%eax
        movl %eax,%ebx
        movb (%esi,%ecx),%al
        movb (%esi,%edx),%bl
        movb (%eax),%al
        movb (%ebx),%bl
        
        testl $0xfffffffe,_pixelcount
        jnz 0xDeadBeef
.globl rdc8ioffs2
rdc8ioffs2: // rdc8iloop
        jmp 0xDeadBeef
.globl rdc8ioffs3
rdc8ioffs3: // rdc8ichecklast

.globl rdc8iloop
rdc8iloop:
        movl %ebp,%ecx
        addl $0xDeadBeef,%ebp
.globl rdc8ipatch1
rdc8ipatch1:
        movb %al,(%edi)
        shrl $25,%ecx
        movl %ebp,%edx
        addl $0xDeadBeef,%ebp
.globl rdc8ipatch2
rdc8ipatch2:
        movb %bl,0xDeadBeef(%edi)
.globl rdc8iwidth1
rdc8iwidth1:
        shrl $25,%edx
        movb (%esi,%ecx),%al
        addl $0xDeadBeef,%edi
.globl rdc8iwidth2
rdc8iwidth2:
        movb (%esi,%edx),%bl
        decl _loopcount
        movb (%eax),%al
        movb (%ebx),%bl
        jnz rdc8iloop
        
.globl rdc8ichecklast
rdc8ichecklast:
        testl $1,_pixelcount
        jz rdc8idone
        movb %al,(%edi)
.globl rdc8idone
rdc8idone:
   popl %ebx
   popl %edi
   popl %esi
   popl %ebp
   ret

.globl R_DrawColumn8_id_end
R_DrawColumn8_id_end:
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;

//
// id's code, further optimised by Erik. (-ES- 1998-08-04)
//

        .align 4
.globl _R_DrawColumn8_id_Erik
_R_DrawColumn8_id_Erik:
   pushl %ebp
   pushl %esi
   pushl %edi
   pushl %ebx
        movl _dc_yl,%ebp
        movl %ebp,%ebx
        movl _ylookup,%edi
        movl (%edi,%ebx,4),%edi
        movl _dc_x,%ebx
        movl _columnofs,%eax
        addl (%eax,%ebx,4),%edi
        
        movl _dc_yh,%eax
        incl %eax
        subl %ebp,%eax
        movl %eax,_pixelcount
        js 0xDeadBeef
.globl rdc8eoffs1
rdc8eoffs1: // rdc8edone
        shrl $1,%eax
        movl %eax,_loopcount
        
        movl _dc_iscale,%ecx
        
        xorl %eax,%eax
        subl %ebp,%eax
        imull	%ecx
        movl _dc_texturemid,%ebp
        subl %eax,%ebp
        shll $9,%ebp

        movl _dc_source,%esi	
        
        movl _dc_iscale,%ebx
        shll $9,%ebx
        movl %ebx,(rdc8epatch1-4)
.globl rdc8epatcher1
rdc8epatcher1:
        movl %ebx,%ecx
        addl %ecx,%ecx
        movl %ecx,(rdc8epatch2-4)
.globl rdc8epatcher2
rdc8epatcher2:
        movl %esi,(rdc8epatch3-4)
.globl rdc8epatcher3
rdc8epatcher3:
        movl %esi,(rdc8epatch4-4)
.globl rdc8epatcher4
rdc8epatcher4:
        
        movl %ebp,%ecx
        addl %ebx,%ebp
        shrl $25,%ecx
        movl %ebp,%edx
        addl %ebx,%ebp
        shrl $25,%edx
        movl _dc_colormap,%eax
        movl %eax,%ebx
        movb (%esi,%ecx),%al
        movb (%esi,%edx),%bl
        movb (%eax),%al
        movb (%ebx),%bl
        movl _loopcount, %esi
        movl %ebp,%ecx
        
        testl $0xfffffffe,_pixelcount
        jnz 0xDeadBeef
.globl rdc8eoffs2
rdc8eoffs2: // rdc8eloop
        jmp 0xDeadBeef
.globl rdc8eoffs3
rdc8eoffs3: // rdc8echecklast

.globl rdc8eloop
rdc8eloop:
        movb %al,(%edi)
        leal 0xDeadBeef(%ebp),%edx
.globl rdc8epatch1
rdc8epatch1:
        shrl $25,%ecx
        addl $0xDeadBeef,%ebp
.globl rdc8epatch2
rdc8epatch2:
        movb %bl,0xDeadBeef(%edi)
.globl rdc8ewidth1
rdc8ewidth1:
        shrl $25,%edx
        addl $0xDeadBeef,%edi
.globl rdc8ewidth2
rdc8ewidth2:
        movb 0xDeadBeef(%ecx),%al
.globl rdc8epatch3
rdc8epatch3:
        movb 0xDeadBeef(%edx),%bl
.globl rdc8epatch4
rdc8epatch4:
        movl %ebp,%ecx
        decl %esi
        movb (%eax),%al
        movb (%ebx),%bl
        jnz rdc8eloop
        
.globl rdc8echecklast
rdc8echecklast:
        testl $1,_pixelcount
        jz rdc8edone
        movb %al,(%edi)
.globl rdc8edone
rdc8edone:

   popl %ebx
   popl %edi
   popl %esi
   popl %ebp
   ret

.globl R_DrawColumn8_id_Erik_end
R_DrawColumn8_id_Erik_end:
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;

        .align 4
.globl _R_DrawColumn8_NOMMX
_R_DrawColumn8_NOMMX:
   pushl %ebp
   pushl %esi
   pushl %edi
   pushl %ebx
	movl _dc_yl,%edx
	movl _dc_yh,%eax
	subl %edx,%eax
	leal 1(%eax),%ebx
	testl %ebx,%ebx
	jle rdc8ndone
	movl _dc_x,%eax
        movl _ylookup, %edi
	movl (%edi,%edx,4),%esi
	movl _columnofs, %edi
	addl (%edi,%eax,4),%esi
	movl _dc_iscale,%edi
	movl %edx,%eax
	imull %edi,%eax
	movl _dc_texturemid,%ecx
	addl %eax,%ecx

	movl _dc_source,%ebp
   xorl %edx, %edx
   subl $0xDeadBeef, %esi
.globl rdc8nwidth1
rdc8nwidth1:
	.align 4,0x90
rdc8nloop:
	movl %ecx,%eax
	shrl $16,%eax
	addl %edi,%ecx
	andl $127,%eax
	addl $0xDeadBeef,%esi
.globl rdc8nwidth2
rdc8nwidth2:
	movb (%eax,%ebp),%dl
	movl _dc_colormap,%eax
	movb (%eax,%edx),%al
	movb %al,(%esi)
	decl %ebx
	jne rdc8nloop
rdc8ndone:
   popl %ebx
   popl %edi
   popl %esi
   popl %ebp
   ret

        .align 4
.globl _R_DrawColumn8_Rasem
_R_DrawColumn8_Rasem:
	pushl %ebp
        pushl %edi
        pushl %esi
        pushl %ebx
	movl _dc_yl,%edi
	movl _dc_yh,%eax
	subl %edi,%eax
        movl _ylookup,%ebx
        leal 1(%eax),%ecx
        movl _columnofs,%eax
        testl %ecx,%ecx
        jle  L1
	movl _dc_x,%edx
        movl (%ebx,%edi,4),%esi
	movl (%eax,%edx,4),%eax
	movl _dc_iscale,%ebp
	addl %eax,%esi
        imull %ebp,%edi
        addl _dc_texturemid,%edi
        .align 2,0x90

L3:

        movl %edi,%ebx
        addl %ebp,%edi
        sarl $16,%ebx
	movl _dc_source,%edx
        andl $127,%ebx
        xorl %eax,%eax
        movb (%ebx,%edx),%al
        movl _dc_colormap,%ebx
        movb (%eax,%ebx),%dl
        movb %dl,(%esi)
	addl _SCREENWIDTH,%esi
        decl %ecx
	jnz L3

L1:
        popl %ebx
        popl %esi
        popl %edi
	popl %ebp
	ret
        .align 2, 0x90

//
// Optimised specifically for P54C/P55C (aka Pentium with/without MMX)
// By ES 1998/08/01
//

.globl _R_DrawColumn8_Pentium
_R_DrawColumn8_Pentium:
	pushl %ebp
        pushl %ebx
	pushl %esi
        pushl %edi
	movl _dc_yl,%eax        // Top pixel
	movl _dc_yh,%ebx        // Bottom pixel
        movl _ylookup, %edi
	movl (%edi,%ebx,4),%ecx
	subl %eax,%ebx          // ebx=number of pixels-1
	jl rdc8pdone            // no pixel to draw, done
	jnz rdc8pmany
	movl _dc_x,%edx         // Special case: only one pixel
        movl _columnofs, %edi
	addl (%edi,%edx,4),%ecx // dest pixel at (%ecx)
	movl _dc_iscale,%esi
	imull %esi,%eax
	movl _dc_texturemid,%edi
	addl %eax,%edi          // texture index in edi
	movl _dc_colormap,%edx
   	shrl $16, %edi
   	movl _dc_source,%ebp
	andl $127,%edi
	movb (%edi,%ebp),%dl    // read texture pixel
	movb (%edx),%al	        // lookup for light
	movb %al,0(%ecx) 	// write it
	jmp rdc8pdone		// done!
.align 4, 0x90
rdc8pmany:			// draw >1 pixel
	movl _dc_x,%edx
        movl _columnofs, %edi
	movl (%edi,%edx,4),%edx
	leal 0xDeadBeef(%edx, %ecx), %edi  // edi = two pixels above bottom
.globl rdc8pwidth5
rdc8pwidth5:  // DeadBeef = -2*SCREENWIDTH
        movl _dc_iscale,%edx	// edx = fracstep
	imull %edx,%eax
   	shll $9, %edx           // fixme: Should get 7.25 fix as input
	movl _dc_texturemid,%ecx
	addl %eax,%ecx          // ecx = frac
	movl _dc_colormap,%eax  // eax = lighting/special effects LUT
   	shll $9, %ecx
   	movl _dc_source,%esi    // esi = source ptr

	imull $0xDeadBeef, %ebx // ebx = negative offset to pixel
.globl rdc8pwidth6
rdc8pwidth6:  // DeadBeef = -SCREENWIDTH

// Begin the calculation of the two first pixels
        leal (%ecx, %edx), %ebp
	shrl $25, %ecx
	movb (%esi, %ecx), %al
	leal (%edx, %ebp), %ecx
	shrl $25, %ebp
        movb (%eax), %dl

// The main loop
rdc8ploop:
	movb (%esi,%ebp), %al		// load 1
        leal (%ecx, %edx), %ebp         // calc frac 3

	shrl $25, %ecx                  // shift frac 2
        movb %dl, 0xDeadBeef(%edi, %ebx)// store 0
.globl rdc8pwidth1
rdc8pwidth1:  // DeadBeef = 2*SCREENWIDTH

        movb (%eax), %al                // lookup 1

        movb %al, 0xDeadBeef(%edi, %ebx)// store 1
.globl rdc8pwidth2
rdc8pwidth2:  // DeadBeef = 3*SCREENWIDTH
        movb (%esi, %ecx), %al          // load 2

        leal (%ebp, %edx), %ecx         // calc frac 4

        shrl $25, %ebp                  // shift frac 3
        movb (%eax), %dl                // lookup 2

        addl $0xDeadBeef, %ebx          // counter
.globl rdc8pwidth3
rdc8pwidth3:  // DeadBeef = 2*SCREENWIDTH
        jl rdc8ploop                    // loop

// End of loop. Write extra pixel or just exit.
        jnz rdc8pdone
        movb %dl, 0xDeadBeef(%edi, %ebx)// Write odd pixel
.globl rdc8pwidth4
rdc8pwidth4:  // DeadBeef = 2*SCREENWIDTH

rdc8pdone:

        popl %edi
	popl %esi
        popl %ebx
	popl %ebp
        ret

//
// MMX asm version, optimised for K6
// By ES 1998/07/05
//

.globl _R_DrawColumn8_K6_MMX
_R_DrawColumn8_K6_MMX:
	pushl %ebp
        pushl %ebx
	pushl %esi
        pushl %edi

        movl %esp, %eax // Push 8 or 12, so that (%esp) gets aligned by 8
        andl $7,%eax
        addl $8,%eax
        movl %eax, _mmxcomm // Temp storage in mmxcomm: (%esp) is used instead
        subl %eax,%esp

	movl _dc_yl,%edx        // Top pixel
	movl _dc_yh,%ebx        // Bottom pixel
        movl _ylookup, %edi
	movl (%edi,%ebx,4),%ecx
	subl %edx,%ebx         // ebx=number of pixels-1
	jl 0xDeadBeef            // no pixel to draw, done
.globl rdc8moffs1
rdc8moffs1:
	jnz rdc8mmany
	movl _dc_x,%eax         // Special case: only one pixel
        movl _columnofs, %edi
	addl (%edi,%eax,4),%ecx  // dest pixel at (%ecx)
	movl _dc_iscale,%esi
	imull %esi,%edx
	movl _dc_texturemid,%edi
	addl %edx,%edi         // texture index in edi
	movl _dc_colormap,%edx
   	shrl $16, %edi
   	movl _dc_source,%ebp
	andl $127,%edi
	movb (%edi,%ebp),%dl  // read texture pixel
	movb (%edx),%al	 // lookup for light
	movb %al,0(%ecx) 	 // write it
	jmp 0xDeadBeef		 // done!
.globl rdc8moffs2
rdc8moffs2:
.align 4, 0x90
rdc8mmany:			 // draw >1 pixel
	movl _dc_x,%eax
        movl _columnofs, %edi
	movl (%edi,%eax,4),%eax
	leal 0xDeadBeef(%eax, %ecx), %esi  // esi = two pixels above bottom
.globl rdc8mwidth3
rdc8mwidth3:  // DeadBeef = -2*SCREENWIDTH
        movl _dc_iscale,%ecx	 // ecx = fracstep
	imull %ecx,%edx
   	shll $9, %ecx           // fixme: Should get 7.25 fix as input
	movl _dc_texturemid,%eax
	addl %edx,%eax         // eax = frac
	movl _dc_colormap,%edx  // edx = lighting/special effects LUT
   	shll $9, %eax
	leal (%ecx, %ecx), %edi
   	movl _dc_source,%ebp    // ebp = source ptr
	movl %edi, 0(%esp)     // Start moving frac and fracstep to MMX regs

	imull $0xDeadBeef, %ebx  // ebx = negative offset to pixel
.globl rdc8mwidth5
rdc8mwidth5:  // DeadBeef = -SCREENWIDTH

	movl %edi, 4(%esp)
	leal (%eax, %ecx), %edi
	movq 0(%esp), %mm1     // fracstep:fracstep in mm1
	movl %eax, 0(%esp)
	shrl $25, %eax
	movl %edi, 4(%esp)
	movzbl (%ebp, %eax), %eax
	movq 0(%esp), %mm0     // frac:frac in mm0

	paddd %mm1, %mm0
	shrl $25, %edi
	movq %mm0, %mm2
	psrld $25, %mm2         // texture index in mm2
	paddd %mm1, %mm0
	movq %mm2, 0(%esp)

.globl rdc8mloop
rdc8mloop:                      		// The main loop
	movq %mm0, %mm2                    // move 4-5 to temp reg
	movzbl (%ebp, %edi), %edi 		// read 1

	psrld $25, %mm2 			// shift 4-5
	movb (%edx,%eax), %cl 		// lookup 0

	movl 0(%esp), %eax 			// load 2
	addl $0xDeadBeef, %ebx 		// counter
.globl rdc8mwidth2
rdc8mwidth2:  // DeadBeef = 2*SCREENWIDTH

	movb %cl, (%esi, %ebx)		// write 0
	movb (%edx,%edi), %ch 		// lookup 1

	movb %ch, 0xDeadBeef(%esi, %ebx) 	// write 1
.globl rdc8mwidth1
rdc8mwidth1:  // DeadBeef = SCREENWIDTH
	movl 4(%esp), %edi			// load 3

	paddd %mm1, %mm0 			// frac 6-7
	movzbl (%ebp, %eax), %eax 		// lookup 2

	movq %mm2, 0(%esp) 		     // store texture index 4-5
	jl rdc8mloop

	jnz rdc8mno_odd
	movb (%edx,%eax), %cl  // write the last odd pixel
	movb %cl, 0xDeadBeef(%esi)
.globl rdc8mwidth4
rdc8mwidth4:  // DeadBeef = 2*SCREENWIDTH
rdc8mno_odd:

.globl rdc8mdone
rdc8mdone:
        emms

        addl _mmxcomm, %esp
        popl %edi
	popl %esi
        popl %ebx
	popl %ebp
        ret

// Need some extra space to align run-time
.globl R_DrawColumn8_K6_MMX_end
R_DrawColumn8_K6_MMX_end:
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;nop;
nop;nop;nop;nop;nop;nop;nop;

//
// DOSDoom's original asm version (by Chi Hoang, I guess)
// Moved here by -ES- 1998/08/05
//
.globl _R_DrawColumn16_Old
_R_DrawColumn16_Old:
    pushl %ebp
    pushl %ebx
    pushl %esi
    pushl %edi
        movl _dc_yl,%edx
	movl _dc_yh,%ebx
	subl %edx,%ebx
	incl %ebx
	jle rdc16odone
	movl _ylookup,%esi
	movl _dc_x,%eax
        movl _columnofs,%edi
	movl (%esi,%edx,4),%esi
	addl (%edi,%eax,4),%esi
	movl _dc_iscale,%edi
	movl %edx,%eax
	imull %edi,%eax
	movl _dc_texturemid,%ecx
	addl %eax,%ecx

	movl _dc_source,%ebp
        subl $0xDeadBeef, %esi
.globl rdc16owidth1
rdc16owidth1:
        xorl %edx, %edx

	.align 4,0x90
rdc16oloop:
	movl %ecx,%eax
	shrl $16,%eax
	addl %edi,%ecx
	andl $127,%eax
	addl $0xDeadBeef,%esi
.globl rdc16owidth2
rdc16owidth2:
	movb (%eax,%ebp),%dl
	movl _dc_colormap,%eax
	movw (%eax,%edx,2),%ax
	movw %ax,(%esi)
	decl %ebx
	jne rdc16oloop
rdc16odone:
    popl %edi
    popl %esi
    popl %ebx
    popl %ebp
    ret

.globl _R_DrawColumn16_Rasem
_R_DrawColumn16_Rasem:
	pushl %ebp
        pushl %ebx
	pushl %esi
        pushl %edi
        movl _dc_yl,%ebx
	movl _dc_yh,%eax
        subl %ebx,%eax
	movl _ylookup,%ecx
        leal 1(%eax),%edi
	movl _dc_x,%edx
        testl %edi,%edi
	jle L7
	movl _columnofs,%eax
        movl (%ecx,%ebx,4),%esi
	addl (%eax,%edx,4),%esi
	movl _dc_iscale,%ebp
        movl _dc_colormap,%eax
        imull %ebp,%ebx
        .byte 0x2e
        movl %esi,%esi
        addl _dc_texturemid,%ebx

L9:

        movl %ebx,%ecx
        addl %ebp,%ebx
        sarl $16,%ecx
	movl _dc_source,%edx
        andl $127,%ecx
        movb (%ecx,%edx),%dl
        movl _SCREENWIDTH,%ecx
        andl $255,%edx
        movw (%eax,%edx,2),%dx
        movw %dx,(%esi)
        addl %ecx,%esi
        addl %ecx,%esi
        decl %edi
	jne L9
L7:
        popl %edi
	popl %esi
        popl %ebx
	popl %ebp
	ret
