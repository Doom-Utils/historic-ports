//  BOOM, a modified and improved DOOM engine
//  Copyright (C) 1999 by
//  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//  02111-1307, USA.
//
/////////////////////////////////////////////////////////////////////////////
// Doom screen copy, optimized for PPro and higher CPUs,
// where REP MOVSD is unoptimal. Uses 64-bit integers for
// the fastest data bandwidth. By Lee Killough 1/25/98

.text
.globl _ppro_blit
	.align 8
_ppro_blit:

	movl 8(%esp),%eax
	movl _screens,%ecx
	movl 4(%esp),%edx
	addl %eax,%ecx
	addl %eax,%edx
	shrl $3,%eax
	negl %eax
	.align 8,0x90
lp:	fildq   0(%ecx,%eax,8)
	fistpq  0(%edx,%eax,8)
	fildq   8(%ecx,%eax,8)
	fistpq  8(%edx,%eax,8)
	fildq  16(%ecx,%eax,8)
	fistpq 16(%edx,%eax,8)
	fildq  24(%ecx,%eax,8)
	fistpq 24(%edx,%eax,8)
	addl   $4,%eax
	js     lp
	ret

	.align 8
.globl _pent_blit
_pent_blit:
	pushl %esi
	pushl %edi
	movl 16(%esp),%ecx
	movl _screens,%esi
	movl 12(%esp),%edi
	shrl $4,%ecx
	.align 8,0x90
lp2:
	fildq   0(%esi)
	fildq   8(%esi)
        fxch
	fistpq  0(%edi)
	fistpq  8(%edi)
        addl    $16,%esi
        addl    $16,%edi
        decl    %ecx
        jne     lp2
	popl  %edi
	popl  %esi
	ret

//----------------------------------------------------------------------------
// $Id: pproblit.s,v 1.4 1998/02/23 04:53:25 killough Exp $
//----------------------------------------------------------------------------
//
// $Log: pproblit.s,v $
// Revision 1.4  1998/02/23  04:53:25  killough
// Performance tuning, add Pentium routine
//
// Revision 1.3  1998/02/09  03:12:04  killough
// Change blit to forward direction
//
// Revision 1.2  1998/01/26  19:31:17  phares
// First rev w/o ^Ms
//
// Revision 1.1  1998/01/26  05:51:52  killough
// PPro tuned blit
//
//
//----------------------------------------------------------------------------
