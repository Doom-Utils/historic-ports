BITS 32

; Segment/section definition macros. 

%ifdef M_TARGET_WATCOM
  SEGMENT DATA PUBLIC ALIGN=16 CLASS=DATA USE32
  SEGMENT DATA
%else
  SECTION .data
%endif

; External variables:
extern _ds_y
extern _ds_x1
extern _ds_x2
extern _ds_xstep
extern _ds_ystep
extern _ds_xfrac
extern _ds_yfrac
extern _ds_colormap
extern _ds_source
extern _ylookup
extern _columnofs
extern _dc_yh
extern _dc_yl
extern _dc_x
extern _dc_texheight
extern _dc_source
extern _dc_iscale
extern _dc_colormap
extern _dc_texturemid
extern _centery
extern _tranmap
extern _SCREENWIDTH


%ifdef M_TARGET_WATCOM
  SEGMENT CODE PUBLIC ALIGN=16 CLASS=CODE USE32
  SEGMENT CODE
%else
  SECTION .text
%endif


 ; //================
 ; //
 ; // R_DrawSpan
 ; //
 ; // Horizontal texture mapping
 ; //
 ; //================
 ; //
 ; // 2/14/98 Lee Killough
 ; //
 ; // Ported from the released linux source
 ; // Converted from Intel to AT&T syntax
 ; // Replaced self-modifying code with code that uses stack
 ; // Removed dependence on 256-byte-aligned colormaps
 ; //
 ; //================

;global _R_DrawSpan

global _R_DrawColumn

;global _R_DrawTLColumn

_R_DrawSpan: ; basic label
push     ebp
push     esi
push     edi
push     ebx

 ; //
 ; // find loop count
 ; //

 ; // count = ds_x2 - ds_x1 + 1; 

mov     eax ,    dword [_ds_x2]
inc     eax

 ; // pixel count

sub     eax,    dword [_ds_x1]

 ; // nothing to scale
jle     near hdone

 ; // pixel count

push     eax

 ; //
 ; // build composite position
 ; //

mov     ebp,    dword [_ds_xfrac]
shl     ebp,    10
and     ebp,    00ffff0000h
mov     eax,    dword [_ds_yfrac]
shr     eax,    6
and     eax,    0ffffh
or         ebp,    eax

 ; //
 ; // calculate screen dest
 ; //

 ; //  dest = ylookup[ds_y] + columnofs[ds_x1];

mov     ebx,    dword [_ds_y]
mov     eax,    dword [_ds_x1]
mov     esi,    dword [_ds_source]
mov     edi,    dword [_ylookup+ebx*4]
add     edi,    dword [_columnofs+eax*4]

 ; //
 ; // build composite step
 ; //

mov     ebx,    dword [_ds_xstep]
shl     ebx,    10
and     ebx,    0ffff0000h
mov     eax,    dword [_ds_ystep]
shr     eax,    6
and     eax,    0ffffh
or         ebx,    eax
push     ebx

 ; // %eax, %ebx, %ecx,%edx scratch
 ; // %esi  virtual source
 ; // %edi  moving destination pointer
 ; // %ebp  frac

 ; // begin calculating third pixel (y units)
shld     ecx,    ebp,    22

 ; // begin calculating third pixel (x units)
shld     ecx,    ebp,    6

 ; // advance frac pointer
add     ebp,    ebx

 ; // finish calculation for third pixel
and     ecx,    4095

 ; // begin calculating fourth pixel (y units)
shld     edx,    ebp,    22

 ; // begin calculating fourth pixel (x units)
shld     edx,    ebp,    6

 ; // advance frac pointer
add     ebp,    ebx

 ; // finish calculation for fourth pixel
and     edx,    4095

 ; // get first pixel
xor     eax,    eax
mov     al,    byte [esi+ecx]

 ; // get second pixel
xor     ebx,    ebx
mov     bl,    byte [esi+edx]

 ; // offset first pixel into colormap
add     eax,    dword [_ds_colormap]

 ; // offset second pixel into colormap
add     ebx,    dword [_ds_colormap]

sub     dword [4+esp],    2

 ; // color translate first pixel
mov     al,    byte [eax]

 ; // color translate second pixel
mov     bl,    byte [ebx]

jl     near hchecklast

 ; // at least two pixels to map


align 256

hdoubleloop: ; basic label
 ; // begin calculating third pixel (y units)
shld     ecx,    ebp,    22

 ; // begin calculating third pixel (x units)
shld     ecx,    ebp,    6

 ; // advance frac pointer
add     ebp,    dword [esp]

 ; // write first pixel
mov     byte [edi],    al

 ; // finish calculation for third pixel
and     ecx,    4095

 ; // begin calculating fourth pixel (y units)
shld     edx,    ebp,    22

 ; // begin calculating fourth pixel (x units)
shld     edx,    ebp,    6

 ; // advance frac pointer
add     ebp,    dword [esp]

 ; // finish calculation for fourth pixel
and     edx,    4095

 ; // write second pixel
mov     byte [1+edi],    bl

 ; // get third pixel
xor     eax,    eax
mov     al,    byte [esi+ecx]

 ; // get fourth pixel
xor     ebx,    ebx
mov     bl,    byte [esi+edx]

 ; // advance to third pixel destination
add     edi,    2

 ; // offset third pixel into colormap
add     eax,    [_ds_colormap]

 ; // offset fourth pixel into colormap
add     ebx,    [_ds_colormap]

 ; // done with loop?
sub     dword [4+esp],    2

 ; // color translate third pixel
mov     al,    byte [eax]

 ; // color translate fourth pixel
mov     bl,    byte [ebx]
jge     hdoubleloop

 ; // check for final pixel
hchecklast: ; basic label
pop     ecx
pop     edx
jnp     hdone

 ; // write final pixel
mov     byte [edi],    al
hdone: ; basic label
pop     ebx
pop     edi
pop     esi
pop     ebp
ret

 ; //================
 ; //
 ; // R_DrawColumn
 ; //
 ; //================
 ; //
 ; // 2/15/98 Lee Killough
 ; //
 ; // Converted C code with TFE fix to assembly and tuned
 ; //
 ; // 2/21/98 killough: added translucency support
 ; //
 ; //================

_R_DrawColumn: ; basic label

push     ebp
push     esi
push     edi
push     ebx
mov     esi,    dword [_dc_yh]
mov     edx,    dword [_dc_yl]
mov     eax,    dword [_dc_x]
inc     esi
mov     ebx,    dword [_ylookup+edx*4]
sub     esi,    edx
jle     near end
add     ebx,    dword [_columnofs+eax*4]
mov     eax,    dword [_dc_texheight]
sub     edx,    dword [_centery]
mov     ebp,    dword [_dc_source]
imul     edx,    dword [_dc_iscale]
lea     ecx,    [-1+eax]
;mov        ecx,    eax
;dec        ecx
mov     edi,    dword [_dc_colormap]
add     edx,    dword [_dc_texturemid]
test     ecx,    eax
je         near powerof2
sal     eax,    16

red1: ; basic label
sub     edx,    eax
jge     near red1

red2: ; basic label
add     edx,    eax
jl         near red2


nonp2loop: ; basic label
mov     ecx,    edx
sar     ecx,    16
add     edx,    dword [_dc_iscale]
movzx     ecx,    byte [ecx+ebp]
mov     cl,    byte [edi+ecx]
mov     byte [ebx],    cl
add     ebx,    dword [_SCREENWIDTH]
cmp     edx,    eax
jge     near wraparound
dec     esi
jg         near nonp2loop
pop     ebx
pop     edi
pop     esi
pop     ebp
ret


wraparound: ; basic label
sub     edx,    eax
dec     esi
jg         near nonp2loop
pop     ebx
pop     edi
pop     esi
pop     ebp
ret


end: ; basic label
pop     ebx
pop     edi
pop     esi
pop     ebp
ret


p2loop: ; basic label
mov     eax,    edx
add     edx,    dword [_dc_iscale]
sar     eax,    16
and     eax,    ecx
movzx     eax,    byte [eax+ebp]
mov     al,    byte [eax+edi]
mov     byte [ebx],    al
add     ebx,    dword [_SCREENWIDTH]
mov     eax,    edx
add     edx,    dword [_dc_iscale]
sar     eax,    16
and     eax,    ecx
movzx     eax,    byte [eax+ebp]
mov     al,    byte [eax+edi]
mov     byte [ebx],    al
add     ebx,    dword [_SCREENWIDTH]

powerof2: ; basic label
add     esi,    -2
jge     near p2loop
jnp     end
sar     edx,    16
and     edx,    ecx
xor     eax,    eax
mov     al,    byte [edx+ebp]
mov     al,    byte [eax+edi]
mov     byte [ebx],    al
pop     ebx
pop     edi
pop     esi
pop     ebp
ret

 ; //================
 ; //
 ; // R_DrawTLColumn
 ; //
 ; // Translucency support
 ; //
 ; //================


_R_DrawTLColumn: ; basic label

push     ebp
push     esi
push     edi
push     ebx
mov     esi,    dword [_dc_yh]
mov     edx,    dword [_dc_yl]
mov     eax,    dword [_dc_x]
inc     esi
mov     ebx,    dword [_ylookup+edx*4]
sub     esi,    edx
jle     near end_tl
add     ebx,    dword [_columnofs+eax*4]
mov     eax,    dword [_dc_texheight]
sub     edx,    dword [_centery]
mov     ebp,    dword [_dc_source]
imul     edx,    dword [_dc_iscale]
lea     ecx,    [-1+eax]
mov     edi,    dword [_dc_colormap]
add     edx,    dword [_dc_texturemid]
test     ecx,    eax
push     ecx
je         near powerof2_tl
sal     eax,    16

red1_tl: ; basic label
sub     edx,    eax
jge     near red1_tl

red2_tl: ; basic label
add     edx,    eax
jl         near red2_tl
push     esi


nonp2loop_tl: ; basic label
xor     ecx,    ecx
mov     esi,    edx
mov     cl,    byte [ebx]
shl     ecx,    8
sar     esi,    16
add     ecx,    [_tranmap]
add     edx,    dword [_dc_iscale]
movzx     esi,    byte [esi+ebp]
movzx     esi,    byte [edi+esi]
mov     cl,    byte [ecx+esi]
mov     byte [ebx],    cl
add     ebx,    dword [_SCREENWIDTH]
cmp     edx,    eax
jge     wraparound_tl
dec     dword [esp]
jg         near nonp2loop_tl
pop     eax
pop     ecx
pop     ebx
pop     edi
pop     esi
pop     ebp
ret


wraparound_tl: ; basic label
sub     edx,    eax
dec     dword [esp]
jg         near nonp2loop_tl
pop     eax
pop     ecx
pop     ebx
pop     edi
pop     esi
pop     ebp
ret


end_tl: ; basic label
pop     ecx
pop     ebx
pop     edi
pop     esi
pop     ebp
ret


p2loop_tl: ; basic label
mov     eax,    edx
xor     ecx,    ecx
add     edx,    dword [_dc_iscale]
mov     cl,    byte [ebx]
sar     eax,    16
shl     ecx,    8
and     eax,    dword [esp]
add     ecx,    dword [_tranmap]
movzx     eax,    byte [eax+ebp]
movzx     eax,    byte [edi+eax]
mov     al,    byte [ecx+eax]
xor     ecx,    ecx
mov     byte [ebx],    al
add     ebx, dword [_SCREENWIDTH]
mov     cl,    byte [ebx]
mov     eax,    edx
add     edx,    dword [_dc_iscale]
sar     eax,    16
shl     ecx,    8
and     eax,    dword [esp]
add     ecx,    dword [_tranmap]
movzx     eax,    byte [eax+ebp]
movzx     eax,    byte [edi+eax]
mov     al,    byte [ecx+eax]
mov     byte [ebx],    al
add     ebx, dword [_SCREENWIDTH]

powerof2_tl: ; basic label
add     esi,    -2
jge     near p2loop_tl
jnp     near end_tl
xor     ecx,    ecx
sar     edx,    16
mov     cl,    byte [ebx]
and     edx,    dword [esp]
shl     ecx,    8
xor     eax,    eax
add     ecx,    dword [_tranmap]
mov     al,    byte [edx+ebp]
movzx     eax,    byte [eax+edi]
mov     al ,    byte [ecx+eax]
mov     byte [ebx],    al
pop     ecx
pop     ebx
pop     edi
pop     esi
pop     ebp
ret
