#line 1 ".\\vu1\\mcode\\sprite_transform.vu"










#line 1 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"





























































                                    













                                    



































































































#line 177 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"











#line 189 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































#line 314 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































































































































































































































































































































































































#line 791 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"

#line 12 ".\\vu1\\mcode\\sprite_transform.vu"

.vu 
.org    0x3000
.align  4 
.global VU1_SPRITE_XFORM_CODE_START
.global VU1_SPRITE_XFORM_CODE_END

.global     VU1_ENTRY_SPRITE_XFORM
.global     VU1_ENTRY_VEL_SPRITE_XFORM
.global     VU1_ENTRY_DISTORT_SPRITE_XFORM
.equ        VU1_ENTRY_SPRITE_XFORM,         ((VU1_SPRITE_XFORM        -VU1_SPRITE_XFORM_CODE_START+0x3000)/8)
.equ        VU1_ENTRY_VEL_SPRITE_XFORM,     ((VU1_VEL_SPRITE_XFORM    -VU1_SPRITE_XFORM_CODE_START+0x3000)/8)
.equ        VU1_ENTRY_DISTORT_SPRITE_XFORM, ((VU1_DISTORT_SPRITE_XFORM-VU1_SPRITE_XFORM_CODE_START+0x3000)/8)



VU1_SPRITE_XFORM_CODE_START:




























































































VU1_SPRITE_XFORM:
    sub.xyz         VF16xyz,  vf00xyz,    vf00xyz     xtop        VI05
    sub.xyz         VF18xyz,  vf00xyz,    vf00xyz     lq.xyzw     VF08xyzw, 1+3(VI05)
    sub.xyz         VF15xyz,  vf00xyz,    vf00xyz     lq.xyzw     VF22xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 33 )(vi00)
    addw.xyz        VF17xyz,  vf00xyz,    vf00w       lq.xyzw     VF21xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 32 )(vi00)
    addw.yz         VF16yz,   VF16yz,   vf00w       lq.xyzw     VF20xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 31 )(vi00)
    addw.xz         VF18xz,   VF18xz,   vf00w       lq.xyzw     VF19xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 30 )(vi00)
    mulaw.xyzw      acc,        VF22xyzw, VF08w     lq.xyzw     VF07xyzw, 1+2(VI05)
    maddaz.xyzw     acc,        VF21xyzw, VF08z     lq.xyzw     VF06xyzw, 1+1(VI05)
    madday.xyzw     acc,        VF20xyzw, VF08y     lq.xyzw     VF05xyzw, 1+0(VI05)
    maddx.xyzw      VF08xyzw, VF19xyzw, VF08x     iaddiu      VI07, vi00, 0
    mulaw.xyzw      acc,        VF22xyzw, VF07w     iaddiu      VI06, vi00, ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) )
    maddaz.xyzw     acc,        VF21xyzw, VF07z     ilw.z       VI04,   0(VI05)
    madday.xyzw     acc,        VF20xyzw, VF07y     isw.x       VI07,   ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)
    maddx.xyzw      VF07xyzw, VF19xyzw, VF07x     isw.y       VI05,   ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)
    mulaw.xyzw      acc,        VF22xyzw, VF06w     isw.z       VI06,   ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)
    maddaz.xyzw     acc,        VF21xyzw, VF06z     lq.y        VF14y,    0(VI05)
    madday.xyzw     acc,        VF20xyzw, VF06y     iaddiu      VI01,   vi00,   0x7f
    maddx.xyzw      VF06xyzw, VF19xyzw, VF06x     iand        VI02,   VI04,   VI01
    mulaw.xyzw      acc,        VF22xyzw, VF05w     iadd        VI08,   vi00,   VI05
    maddaz.xyzw     acc,        VF21xyzw, VF05z     iadd        VI09,   VI08,   VI02
    madday.xyzw     acc,        VF20xyzw, VF05y     iadd        VI09,   VI09,   VI02
    maddx.xyzw      VF05xyzw, VF19xyzw, VF05x     iadd        VI09,   VI09,   VI02
    suby.z          VF14z,    vf00z,      VF14y     iadd        VI09,   VI09,   VI02
    addz.x          VF25x,    vf00x,      VF19z     move.x      VF23x,    VF19x             ; V2W_20 = W2V_02   ; V2W_00 = W2V_00
    addz.y          VF25y,    vf00y,      VF20z     mr32.w      VF23w,    vf00w               ; V2W_21 = W2V_12   ; V2W_03 = 0.0f
    addx.y          VF23y,    vf00y,      VF20x     move.y      VF24y,    VF20y             ; V2W_01 = W2V_10   ; V2W_11 = W2V_11
    addx.z          VF23z,    vf00z,      VF21x     mr32.w      VF24w,    vf00w               ; V2W_02 = W2V_20   ; V2W_13 = 0.0f
    addy.x          VF24x,    vf00x,      VF19y     move.z      VF25z,    VF21z             ; V2W_10 = W2V_01   ; V2W_22 = W2V_22
    addy.z          VF24z,    vf00z,      VF21y     mr32.w      VF25w,    vf00w               ; V2W_12 = W2V_21   ; V2W_23 = 0.0f
    mulaw.xyz       acc,        vf00,       vf00w       move.w      VF26w,    vf00w               ; acc = 0,0,0       ; V2W_33 = 1.0f
    msubax.xyz      acc,        VF23xyz,  VF22x     lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 34 )(vi00)  ; acc -= z*V2W2
#line 153 ".\\vu1\\mcode\\sprite_transform.vu"
msubaz.xyz      acc,        VF25xyz,  VF22z     lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 35 )(vi00)  ; acc -= y*V2W1
#line 154 ".\\vu1\\mcode\\sprite_transform.vu"
msuby.xyz       VF26xyz,  VF24xyz,  VF22y     lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 36 )(vi00)  ; V2W3 = acc-x*V2W0
#line 155 ".\\vu1\\mcode\\sprite_transform.vu"
addw.z          VF15z,    VF15z,    vf00w       lq.xyzw     VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 37 )(vi00)
    addw.xyz        VF13xyz,  vf00xyz,    vf00w       ilw.x       VI12,       0(VI05)
    add.xy          VF29xy,   VF10xy,   vf00xy      sq.xyzw     VF23xyzw, 1+0(VI05)
    add.xy          VF30xy,   VF11xy,   vf00xy      sq.xyzw     VF24xyzw, 1+1(VI05)
    add.xy          VF31xy,   VF12xy,   vf00xy      sq.xyzw     VF25xyzw, 1+2(VI05)
    ftoi12.xy       VF15xy,   VF15xy                sq.xyzw     VF26xyzw, 1+3(VI05)
    ftoi12.xy       VF16xy,   VF16xy                iaddiu      VI10,   vi00,   0x7fff
    ftoi12.xy       VF17xy,   VF17xy                iaddiu      VI10,   VI10,   1
    ftoi12.xy       VF18xy,   VF18xy                iadd        VI13,   VI08,   VI12
    sub.z           VF25z,    vf00z,      vf00z       iadd        VI13,   VI13,   VI12
    sub.z           VF26z,    vf00z,      vf00z       ibeq        VI12,   vi00,   SPRITE_XFORM_LOOP
    nop                                                 iadd        VI13,   VI13,   VI12
    nop                                                 iadd        VI13,   VI13,   VI12
    
; Skip over the first sprites just filling in the adc bit. We do this to help the main CPU out
; in dealing with alignment issues. If a buffer is not aligned, we just back it up until it is
; aligned, and skip over the garbage data.
SPRITE_SKIP_LOOP:
    nop                                                 isw.w       VI10,   11(VI08)
    nop                                                 iaddiu      VI08,   VI08,   4
    nop                                                 nop
    nop                                                 ibne        VI08,   VI13,   SPRITE_SKIP_LOOP
    nop                                                 nop

; the main sprite transform loop
SPRITE_XFORM_LOOP:
    nop                                                 lq.xyzw     VF19xyzw, 11(VI08)                   ;                   ; load the sprite center
    nop                                                 lq.xyz      VF20xyz,  9(VI08)                   ;                   ; load the sprite rotation, scale, and flags
    nop                                                 lq.xyzw     VF24xyzw, 10(VI08)                   ;                   ; load the sprite color
    mulaw.xyzw      acc,        VF08xyzw, vf00w       isw.w       VI10,       (11+4*0)(VI08) ; l2v*center        ; store adc bit into vert 0
    maddaz.xyzw     acc,        VF07xyzw, VF19z     isw.w       VI10,       (11+4*1)(VI08) ; l2v*center        ; store adc bit into vert 1
    mulx.x          VF21x,    VF20x,    VF20x     mtir        VI11,       VF19w                         ; rot2 = rot*rot    ; VI11 = ROTS.z
    sub.x           VF20x,    vf00x,      VF20x     sq.xyzw     VF24xyzw, (10+4*1)(VI08) ; rot = -rot        ; copy color into vert 1
    madday.xyzw     acc,        VF06xyzw, VF19y     sq.xyzw     VF24xyzw, (10+4*2)(VI08) ; l2v*center        ; copy color into vert 2
    maddx.xyzw      VF19xyzw, VF05xyzw, VF19x     sq.xyzw     VF24xyzw, (10+4*3)(VI08) ; l2v*center        ; copy color into vert 3
    mulx.x          VF22x,    VF21x,    VF21x     isw.w       VI11,       (11+4*2)(VI08) ; rot4 = rot2*rot2  ; copy active bit into vert 2
    mulx.zw         VF29zw,   VF10zw,   VF20x     isw.w       VI11,       (11+4*3)(VI08) ; (B,F,Jx,Nx)       ; copy active bit into vert 3
    mulx.zw         VF30zw,   VF11zw,   VF20x     sq.xyz      VF15xyz,  (9+4*0)(VI08) ; (C,G,Kx,Ox)       ; store corner0 into uv 0
    mulx.zw         VF31zw,   VF12zw,   VF20x     sq.xyz      VF18xyz,  (9+4*1)(VI08) ; (D,H,Lx,Px)       ; store corner3 into uv 1
    adda.xy         acc,        VF09xy,   vf00xy      sq.xyz      VF16xyz,  (9+4*2)(VI08) ; acc=(A,B,?,?)     ; store corner1 into uv 2
    mulx.x          VF23x,    VF22x,    VF21x     sq.xyz      VF17xyz,  (9+4*3)(VI08) ; rot6 = rot4*rot2  ; store corner2 into uv 3
    mulax.zw        acc,        VF09zw,   VF20x     nop                                                     ; acc=(A,E,Ix,Mx)
    maddax.xyzw     acc,        VF29xyzw, VF21x     nop                                                     ; acc+=TSC1*rot2
    maddax.xyzw     acc,        VF30xyzw, VF22x     nop                                                     ; acc+=TSC2*rot4
    maddx.xyzw      VF21xyzw, VF31xyzw, VF23x     nop                                                     ; res=acc+TSC3*rot6
    muly.yz         VF20yz,   VF14yz,   VF20y     nop                                                     ; (Scale,-Scale)
    nop                                                 iaddiu      VI08,   VI08,   4*4
    nop                                                 nop
    addaw.x         acc,        vf00x,      VF21w     nop                                                     ; (-S,?,?,?)
    addaz.y         acc,        vf00y,      VF21z     nop                                                     ; (-S,S,?,?)
    maddx.xy        VF25xy,   VF13xy,   VF21x     nop                                                     ; (C-S,C+S,0,?)
    addax.x         acc,        vf00x,      VF21x     nop                                                     ; (C,?,?,?)
    adday.y         acc,        vf00y,      VF21y     nop                                                     ; (C,-C,?,?)
    maddz.xy        VF26xy,   VF13xy,   VF21z     nop                                                     ; (C+S,S-C,0,?)
    mulay.xyz       acc,        VF25xyz,  VF20y     nop                                                     ; acc=vec0*scale
    maddw.xyz       VF21xyz,  VF19xyz,  vf00w       nop                                                     ; crn0=acc+cntr
    mulaz.xyz       acc,        VF25xyz,  VF20z     nop                                                     ; acc=vec0*-scale
    maddw.xyz       VF23xyz,  VF19xyz,  vf00w       nop                                                     ; crn2=acc+cntr
    mulay.xyz       acc,        VF26xyz,  VF20y     nop                                                     ; acc=vec1*scale
    maddw.xyz       VF22xyz,  VF19xyz,  vf00w       nop                                                     ; crn1=acc+cntr
    mulaz.xyz       acc,        VF26xyz,  VF20z     nop                                                     ; acc=vec1*-scale
    maddw.xyz       VF24xyz,  VF19xyz,  vf00w       sq.xyz      VF21xyz,  (11-4*4)(VI08) ; cnr0=acc+cntr
    nop                                                 sq.xyz      VF23xyz,  (11-4*1)(VI08)
    nop                                                 sq.xyz      VF22xyz,  (11-4*2)(VI08)
    nop                                                 ibne        VI08, VI09, SPRITE_XFORM_LOOP
    nop                                                 sq.xyz      VF24xyz,  (11-4*3)(VI08)
    
    nop                                                 b           VU1_SPRITE_XFORM_CODE_START+0x0040-0x3000+16384
    nop                                                 nop

























































































VU1_VEL_SPRITE_XFORM:
































    sub.xyz         VF18xyz,  vf00xyz,    vf00xyz     xtop        VI05
    sub.xyz         VF17xyz,  vf00xyz,    vf00xyz     lq.xyzw     VF31xyzw, 1+6(VI05)
    sub.xyz         VF16xyz,  vf00xyz,    vf00xyz     lq.xyzw     VF08xyzw, 1+3(VI05)
    addw.xyz        VF19xyz,  vf00xyz,    vf00w       lq.xyzw     VF07xyzw, 1+2(VI05)
    addw.yz         VF18yz,   VF18yz,   vf00w       lq.xyzw     VF06xyzw, 1+1(VI05)
    addw.z          VF17z,    VF17z,    vf00w       lq.xyzw     VF05xyzw, 1+0(VI05)
    addw.xz         VF16xz,   VF16xz,   vf00w       lq.xyzw     VF30xyzw, 1+5(VI05)
    mulaz.xyz       acc,        VF07xyz,  VF31z     lq.xyzw     VF29xyzw, 1+4(VI05)
    madday.xyz      acc,        VF06xyz,  VF31y     ilw.z       VI04,   0(VI05)
    maddx.xyz       VF31xyz,  VF05xyz,  VF31x     iaddiu      VI07, vi00, 0
    mulaz.xyz       acc,        VF07xyz,  VF30z     iaddiu      VI06, vi00, ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) )
    madday.xyz      acc,        VF06xyz,  VF30y     isw.x       VI07, ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)
    maddx.xyz       VF30xyz,  VF05xyz,  VF30x     isw.y       VI05, ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)
    mulaz.xyz       acc,        VF07xyz,  VF29z     isw.z       VI06, ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)
    madday.xyz      acc,        VF06xyz,  VF29y     lq.y        VF20y,    0(VI05)
    maddx.xyz       VF29xyz,  VF05xyz,  VF29x     iaddiu      VI01,   vi00,   0x7f
    sub.xyz         VF13xyz,  vf00xyz,    vf00xyz     iand        VI02,   VI04,   VI01
    sub.xyz         VF14xyz,  vf00xyz,    vf00xyz     iadd        VI08,   vi00,   VI05
    sub.xyz         VF15xyz,  vf00xyz,    vf00xyz     iadd        VI09,   VI08,   VI02
    ftoi12.xy       VF19xy,   VF19xy                iadd        VI09,   VI09,   VI02
    addw.x          VF13x,    VF13x,    vf00w       iadd        VI09,   VI09,   VI02
    addw.y          VF14y,    VF14y,    vf00w       iadd        VI09,   VI09,   VI02
    addw.z          VF15z,    VF15z,    vf00w       lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 30 )(vi00)
    ftoi12.xy       VF18xy,   VF18xy                lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 31 )(vi00)
    ftoi12.xy       VF17xy,   VF17xy                lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 32 )(vi00)
    ftoi12.xy       VF16xy,   VF16xy                iaddiu      VI10,   vi00,   0x7fff
    addz.x          VF12x,    vf00x,      VF09z     iaddiu      VI10,   VI10,   1
    addz.y          VF12y,    vf00y,      VF10z     sq.xyzw     vf00xyzw,   1+3(VI05)
    addz.z          VF12z,    vf00z,      VF11z     ilw.x       VI11,   0(VI05)
    addw.x          VF20x,    vf00x,      vf00w       sq.xyzw     VF13xyzw, 1+0(VI05)
    nop                                                 sq.xyzw     VF14xyzw, 1+1(VI05)
    nop                                                 sq.xyzw     VF15xyzw, 1+2(VI05)
    nop                                                 ibeq        VI11,   vi00,   VEL_SPRITE_LOOP
    nop                                                 iadd        VI12,   VI08,   VI11
    nop                                                 iadd        VI12,   VI12,   VI11
    nop                                                 iadd        VI12,   VI12,   VI11
    nop                                                 iadd        VI12,   VI12,   VI11

; Skip over the first sprites just filling in the adc bit. We do this to help the main CPU out
; in dealing with alignment issues. If a buffer is not aligned, we just back it up until it is
; aligned, and skip over the garbage data.
VEL_SPRITE_SKIP_LOOP:
    nop                                                 isw.w       VI10,   11(VI08)
    nop                                                 iaddiu      VI08,   VI08,   4
    nop                                                 nop
    nop                                                 ibne        VI08,   VI12,   VEL_SPRITE_SKIP_LOOP
    nop                                                 nop

; the main sprite transform loop
VEL_SPRITE_LOOP:
    nop                                                 lq.xyzw     VF22xyzw, 9(VI08)                       ;               ; load velocity
    nop                                                 lq.xyzw     VF21xyzw, 11(VI08)                       ;               ; load position
    nop                                                 lq.xyzw     VF23xyzw, 10(VI08)                       ;               ; load color
    nop                                                 isw.w       VI10,       (11+4*0)(VI08)     ; xform vel     ; store adc into vert 0
    mulaz.xyz       acc,        VF31xyz,  VF22z     isw.w       VI10,       (11+4*1)(VI08)     ; xform vel     ; store adc into vert 1
    madday.xyz      acc,        VF30xyz,  VF22y     sq.w        VF21w,    (11+4*2)(VI08)     ; xform vel     ; copy active into vert 2
    maddx.xyz       VF22xyz,  VF29xyz,  VF22x     sq.w        VF21w,    (11+4*3)(VI08)     ; xform vel     ; copy active into vert 3
    muly.w          VF22w,    VF22w,    VF20y     sq.xyzw     VF23xyzw, (10+4*1)(VI08)     ; uniscl*scale  ; copy color into vert 1
    mulaw.xyzw      acc,        VF08xyzw, vf00w       sq.xyzw     VF23xyzw, (10+4*2)(VI08)     ; xform pos     ; copy color into vert 2
    maddaz.xyzw     acc,        VF07xyzw, VF21z     sq.xyzw     VF23xyzw, (10+4*3)(VI08)     ; xform pos     ; copy color into vert 3
    mul.xyz         VF13xyz,  VF22xyz,  VF22xyz   sq.xyz      VF16xyz,  (9+4*0)(VI08)     ; dot vel       ; store corner 0 uv into vert 0
    madday.xyzw     acc,        VF06xyzw, VF21y     sq.xyz      VF17xyz,  (9+4*1)(VI08)     ; xform pos     ; store corner 1 uv into vert 1
    maddx.xyzw      VF21xyzw, VF05xyzw, VF21x     sq.xyz      VF19xyz,  (9+4*2)(VI08)     ; xform pos     ; store corner 3 uv into vert 2
    nop                                                 sq.xyz      VF18xyz,  (9+4*3)(VI08)     ;               ; store covner 2 uv into vert 3
    adday.x         acc,        VF13x,    VF13y     iaddiu      VI08, VI08, 4*4                     ; dot vel       ; vptr++
    maddz.x         VF13x,    VF20x,    VF13z     nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 rsqrt       q,  vf00w,  VF13x                             ;               ; 1/sqrt(dot)
    nop                                                 waitq
    mulq.xyz        VF22xyz,  VF22xyz,  q           nop                                                         ; Vel.Normalize
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    opmula.xyz      acc,        VF12xyz,  VF22xyz   nop                                                         ; calc up vec
    opmsub.xyz      VF24xyz,  VF22xyz,  VF12xyz   nop                                                         ; calc up vec
    nop                                                 nop
    adda.xyz        acc,        vf00xyz,    VF21xyz   nop                                                         ; calc corner 0
    maddaw.xyz      acc,        VF22xyz,  VF22w     nop                                                         ; calc corner 0
    msubw.xyz       VF25xyz,  VF24xyz,  VF22w     nop                                                         ; calc corner 0
    adda.xyz        acc,        vf00xyz,    VF21xyz   nop                                                         ; calc corner 1
    msubaw.xyz      acc,        VF22xyz,  VF22w     nop                                                         ; calc corner 1
    msubw.xyz       VF26xyz,  VF24xyz,  VF22w     nop                                                         ; calc corner 1
    adda.xyz        acc,        vf00xyz,    VF21xyz   nop                                                         ; calc corner 2
    msubaw.xyz      acc,        VF22xyz,  VF22w     nop                                                         ; calc corner 2
    maddw.xyz       VF27xyz,  VF24xyz,  VF22w     nop                                                         ; calc corner 2
    adda.xyz        acc,        vf00xyz,    VF21xyz   nop                                                         ; calc corner 3
    maddaw.xyz      acc,        VF22xyz,  VF22w     nop                                                         ; calc corner 3
    maddw.xyz       VF28xyz,  VF24xyz,  VF22w     sq.xyz      VF25xyz,  (11-4*4)(VI08)     ; calc corner 3 ; store corner 0 into vert 0
    nop                                                 sq.xyz      VF26xyz,  (11-4*3)(VI08)                     ; store corner 1 into vert 1
    nop                                                 sq.xyz      VF27xyz,  (11-4*1)(VI08)                     ; store corner 2 into vert 3
    nop                                                 ibne        VI08, VI09, VEL_SPRITE_LOOP                                     ; loop
    nop                                                 sq.xyz      VF28xyz,  (11-4*2)(VI08)                     ; store corner 3 into vert 2
                                                        
    nop                                                 b           VU1_SPRITE_XFORM_CODE_START+0x0040-0x3000+16384
    nop                                                 nop
    























































VU1_DISTORT_SPRITE_XFORM:

    nop[e]                                              nop
    nop                                                 nop
    nop[e]                                              nop
    nop                                                 nop
    nop[e]                                              nop
    nop                                                 nop






















































#line 562 ".\\vu1\\mcode\\sprite_transform.vu"

VU1_SPRITE_XFORM_CODE_END:

