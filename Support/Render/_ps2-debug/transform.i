#line 1 ".\\vu1\\mcode\\transform.vu"












#line 1 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"





























































                                    













                                    



































































































#line 177 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"











#line 189 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































#line 314 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































































































































































































































































































































































































#line 791 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"

#line 14 ".\\vu1\\mcode\\transform.vu"

.vu 
.org    0x3000
.align  4 
.global VU1_RIGID_XFORM_CODE_START
.global VU1_RIGID_XFORM_CODE_END

.global     VU1_ENTRY_RIGID_XFORM_FAST
.global     VU1_ENTRY_RIGID_XFORM_CLIPPED
.equ        VU1_ENTRY_RIGID_XFORM_FAST,     ((VU1_FAST-VU1_RIGID_XFORM_CODE_START+0x3000)/8)
.equ        VU1_ENTRY_RIGID_XFORM_CLIPPED,  ((VU1_SLOW-VU1_RIGID_XFORM_CODE_START+0x3000)/8)




VU1_RIGID_XFORM_CODE_START:
VU1_FAST:
















    addw.xyzw       VF18xyzw, VF00xyzw,   VF00w       xtop VI05

    nop[e]                                              nop
    nop                                                 nop

VU1_START:
    nop                                                 ilw.z       VI04,       0(VI05)

    nop                                                 iaddiu      VI01, VI00, 0x2000
    nop                                                 iand        VI01, VI04, VI01
    nop                                                 nop
    nop                                                 ibeq        VI01, VI00, CHECK_DYNAMICLIGHT
    nop                                                 nop
    
    nop                                                 bal         VI15, DO_POINT_LIGHTING_CHEAP
    nop                                                 nop

CHECK_DYNAMICLIGHT:
    nop                                                 iaddiu      VI01, vi00, 0x0400
    nop                                                 iand        VI01, VI04, VI01
    nop                                                 nop
    nop                                                 ibeq        VI01, vi00, CHECK_FILTERLIGHT
    nop                                                 nop
    nop                                                 bal         VI15, DO_FILTER_LIGHTING
    nop                                                 nop
    
CHECK_FILTERLIGHT:

    ;------------------------------------------------------------------------------------
    ;------------------------------------------------------------------------------------
    ;------------------------------------------------------------------------------------
    .macro FastLoop Label, Next
    ;------------------------------------------------------------------------------------
    ;------------------------------------------------------------------------------------
    ;------------------------------------------------------------------------------------

    InputPos  = 11 - (4*1)
    InputTex  = 9 - (4*2)
    OutputPos = 11 - (4*5)
    OutputTex = 9 - (4*4)
    BackupPos = \Next + 11 - (4*2)
    BackupTex = \Next + 9 - (4*3)

    ; loop preamble
    nop                                                 iaddiu      VI08, VI08, 4       ;                       ; vptr++
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 lq.xyzw     VF13xyzw, InputPos(VI08)      ;                       ; load xyz0
    nop                                                 nop
    nop                                                 nop
    
    mulaw.xyzw      acc,        VF12xyzw, vf00w       iaddiu      VI08, VI08, 4       ; xform xyz0            ; vptr++
    maddaz.xyzw     acc,        VF11xyzw, VF13z     sq.xyzw     VF13xyzw, BackupPos(VI08)     ; xform xyz0            ; backup xyz0
    madday.xyzw     acc,        VF10xyzw, VF13y     nop                                         ; xform xyz0
    nop                                                 nop
    maddx.xyzw      VF14xyzw, VF09xyzw, VF13x     nop                                         ; xform xyz0
    nop                                                 lq.xy       VF18xy,   InputTex(VI08)                              ; load uv0
    nop                                                 lq.xyzw     VF13xyzw, InputPos(VI08)      ;                       ; load xyz1
    nop                                                 nop
    add.xyz         VF15xyz,  VF14xyz,  vf00xyz     div         q, vf00w, VF14w               ; copy xyz0             ; divide0

    mulaw.xyzw      acc,        VF12xyzw, vf00w       iaddiu      VI08, VI08, 4       ; xform xyz1            ; vptr++
    maddaz.xyzw     acc,        VF11xyzw, VF13z     sq.xyzw     VF13xyzw, BackupPos(VI08)     ; xform xyz1            ; backup xyz1
    madday.xyzw     acc,        VF10xyzw, VF13y     nop                                         ; xform xyz1
    itof12.xy       VF19xy,   VF18xy                nop                                         ; unpack uv0
    maddx.xyzw      VF14xyzw, VF09xyzw, VF13x     sq.xy       VF18xy,   BackupTex(VI08)     ; xform xyz1            ; backup uv0
    nop                                                 lq.xy       VF18xy,   InputTex(VI08)                              ; load uv1
    mulq.xyz        VF16xyz,  VF15xyz,  q           lq.xyzw     VF13xyzw, InputPos(VI08)      ; project sxyz0         ; load xyz2
    mulq.xyz        VF20xyz,  VF19xyz,  q           nop                                         ; project uv0
    add.xyz         VF15xyz,  VF14xyz,  vf00xyz     div         q, vf00w, VF14w               ; copy xyz1             ; divide1

    mulaw.xyzw      acc,        VF12xyzw, vf00w       iaddiu      VI08, VI08, 4       ; xform xyz2            ; vptr++
    maddaz.xyzw     acc,        VF11xyzw, VF13z     sq.xyzw     VF13xyzw, BackupPos(VI08)     ; xform xyz2            ; backup xyz2
    madday.xyzw     acc,        VF10xyzw, VF13y     nop                                         ; xform xyz2
    itof12.xy       VF19xy,   VF18xy                sq.xyz      VF20xyz,  OutputTex(VI08)     ; unpack uv1            ; store stq0
    maddx.xyzw      VF14xyzw, VF09xyzw, VF13x     sq.xy       VF18xy,   BackupTex(VI08)     ; xform xyz2            ; backup uv1
    ftoi4.xyz       VF17xyz,  VF16xyz               lq.xy       VF18xy,   InputTex(VI08)      ; convert sxyz0         ; load uv2
    mulq.xyz        VF16xyz,  VF15xyz,  q           lq.xyzw     VF13xyzw, InputPos(VI08)      ; project sxyz1         ; load xyz3
    mulq.xyz        VF20xyz,  VF19xyz,  q           nop                                         ; project uv1
    add.xyz         VF15xyz,  VF14xyz,  vf00xyz     div         q, vf00w, VF14w               ; copy xyz2             ; divide2

    ; the main loop
    \Label:
    mulaw.xyzw      acc,        VF12xzyw, vf00w       iaddiu      VI08, VI08, 4       ; xform xyz3            ; vptr++
    maddaz.xyzw     acc,        VF11xyzw, VF13z     sq.xyzw     VF13xyzw, BackupPos(VI08)     ; xform xyz3            ; backup xyz3
    madday.xyzw     acc,        VF10xyzw, VF13y     sq.xyz      VF17xyz,  OutputPos(VI08)     ; xform xyz3            ; store sxyz0
    itof12.xy       VF19xy,   VF18xy                sq.xyz      VF20xyz,  OutputTex(VI08)     ; unpack uv2            ; store stq1
    maddx.xyzw      VF14xyzw, VF09xyzw, VF13x     sq.xy       VF18xy,   BackupTex(VI08)     ; xform xyz3            ; backup uv2
    ftoi4.xyz       VF17xyz,  VF16xyz               lq.xy       VF18xy,   InputTex(VI08)      ; convert sxyz1         ; load uv3
    mulq.xyz        VF16xyz,  VF15xyz,  q           lq.xyzw     VF13xyzw, InputPos(VI08)      ; project sxyz2         ; load xyz4
    mulq.xyz        VF20xyz,  VF19xyz,  q           ibne        VI08, VI09, \Label              ; project uv2           ; loop
    add.xyz         VF15xyz,  VF14xyz,  vf00xyz     div         q, vf00w, VF14w               ; copy xyz3             ; divide3

    ; post-loop
    mulaw.xyzw      acc,        VF12xyzw, vf00w       iaddiu      VI08, VI08, 4       ; xform xyz4            ; vptr++
    maddaz.xyzw     acc,        VF11xyzw, VF13z     sq.xyzw     VF13xyzw, BackupPos(VI08)     ; xform xyz4            ; backup xyz4
    madday.xyzw     acc,        VF10xyzw, VF13y     sq.xyz      VF17xyz,  OutputPos(VI08)     ; xform xyz4            ; store sxyz1
    itof12.xy       VF19xy,   VF18xy                sq.xyz      VF20xyz,  OutputTex(VI08)     ; unpack uv3            ; store stq2
    maddx.xyzw      VF14xyzw, VF09xyzw, VF13x     sq.xy       VF18xy,   BackupTex(VI08)     ; xform xyz4            ; backup uv3
    ftoi4.xyz       VF17xyz,  VF16xyz               lq.xy       VF18xy,   InputTex(VI08)      ; convert sxyz2         ; load uv4
    mulq.xyz        VF16xyz,  VF15xyz,  q           nop                                         ; project sxyz3
    mulq.xyz        VF20xyz,  VF19xyz,  q           nop                                         ; project uv3
    add.xyz         VF15xyz,  VF14xyz,  vf00xyz     div         q, vf00w, VF14w               ; copy sxyz4            ; divide4

    nop                                                 iaddiu      VI08, VI08, 4                               ; vptr++
    nop                                                 nop
    nop                                                 sq.xyz      VF17xyz,  OutputPos(VI08)                             ; store sxyz2
    itof12.xy       VF19xy,   VF18xy                sq.xyz      VF20xyz,  OutputTex(VI08)     ; unpack uv4            ; store stq3
    nop                                                 sq.xy       VF18xy,   BackupTex(VI08)                             ; backup uv4
    ftoi4.xyz       VF17xyz,  VF16xyz               nop                                         ; convert sxyz3
    mulq.xyz        VF16xyz,  VF15xyz,  q           nop                                         ; project sxyz4
    mulq.xyz        VF20xyz,  VF19xyz,  q           nop                                         ; project uv4
    nop                                                 nop

    nop                                                 iaddiu      VI08, VI08, 4                               ; vptr++
    nop                                                 nop
    nop                                                 sq.xyz      VF17xyz,  OutputPos(VI08)                             ; store sxyz3
    nop                                                 sq.xyz      VF20xyz,  OutputTex(VI08)                             ; store stq4
    nop                                                 nop
    ftoi4.xyz       VF17xyz,  VF16xyz               nop                                         ; convert sxyz4
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop

    nop                                                 iaddiu      VI08, VI08, 4                               ; vptr++
    nop                                                 nop
    nop                                                 sq.xyz      VF17xyz,  OutputPos(VI08)                             ; store sxyz4

    ;------------------------------------------------------------------------------------
    ;------------------------------------------------------------------------------------
    ;------------------------------------------------------------------------------------
    .endm
    ;------------------------------------------------------------------------------------
    ;------------------------------------------------------------------------------------
    ;------------------------------------------------------------------------------------

    addw.z          VF19z,    vf00z,      vf00w       lq.xyzw VF08xyzw, 1+3(VI05)
    nop                                                 lq.xyzw VF07xyzw, 1+2(VI05)
    nop                                                 lq.xyzw VF06xyzw, 1+1(VI05)
    nop                                                 lq.xyzw VF05xyzw, 1+0(VI05)
    mulaw.xyzw      acc,        VF04xyzw, VF08w     ilw.z       VI04, 0(VI05)
    maddaz.xyzw     acc,        VF03xyzw, VF08z     iaddiu      VI08, VI05, 0x00
    madday.xyzw     acc,        VF02xyzw, VF08y     iaddiu      VI01, vi00, 0x7f
    maddx.xyzw      VF12xyzw, VF01xyzw, VF08x     iand        VI02, VI04, VI01
    mulaw.xyzw      acc,        VF04xyzw, VF07w     isubiu      VI01, VI02, 5
    maddaz.xyzw     acc,        VF03xyzw, VF07z     nop
    madday.xyzw     acc,        VF02xyzw, VF07y     ibgez       VI01, TRANSFORM_VERT_COUNT_OK
    maddx.xyzw      VF11xyzw, VF01xyzw, VF07x     iadd        VI10, VI02, vi00
    nop                                                 iaddiu      VI10, vi00, 5

TRANSFORM_VERT_COUNT_OK:
    mulaw.xyzw      acc,        VF04xyzw, VF06w     iadd        VI09, VI08, VI10
    maddaz.xyzw     acc,        VF03xyzw, VF06z     iadd        VI09, VI09, VI10
    madday.xyzw     acc,        VF02xyzw, VF06y     iadd        VI09, VI09, VI10
    maddx.xyzw      VF10xyzw, VF01xyzw, VF06x     iadd        VI09, VI09, VI10
    mulaw.xyzw      acc,        VF04xyzw, VF05w     nop
    maddaz.xyzw     acc,        VF03xyzw, VF05z     nop
    madday.xyzw     acc,        VF02xyzw, VF05y     isubiu      VI01, VI05, (8 + (80 * 4)) * 2
    maddx.xyzw      VF09xyzw, VF01xyzw, VF05x     nop

    nop                                                 ibeq        VI01, vi00, FAST_BUFFER2
    nop                                                 nop

    Next = (8 + (80 * 4))
    FastLoop                                            FAST_LOOP1, Next
    nop                                                 b           VU1_RIGID_XFORM_CODE_START+0x0FC0-0x3000+16384
    nop                                                 iaddiu      VI06, VI05, (8 + (80 * 4))

FAST_BUFFER2:
    Next = (-(8 + (80 * 4))*2)
    FastLoop                                            FAST_LOOP2, Next
    nop                                                 b           VU1_RIGID_XFORM_CODE_START+0x0FC0-0x3000+16384
    nop                                                 iadd        VI06, vi00, vi00

















;==============================================================================
;
;   Slow Renderer - Clipping
;
;==============================================================================


VU1_SLOW:

    nop                                                 xtop        VI05

    nop                                                 ilw.z       VI04,       0(VI05)
    nop                                                 iaddiu      VI01, vi00, 0x2000
    nop                                                 iand        VI01, VI04, VI01
    nop                                                 nop
    nop                                                 ibeq        VI01, vi00, CLIP_CHECK_DYNAMICLIGHT
    nop                                                 nop
    nop                                                 bal         VI15, DO_POINT_LIGHTING_CHEAP
    nop                                                 nop
CLIP_CHECK_DYNAMICLIGHT:

    nop                                                 iaddiu      VI01, vi00, 0x0400
    nop                                                 iand        VI01, VI04, VI01
    nop                                                 nop
    nop                                                 ibeq        VI01, vi00, CLIP_CHECK_FILTERLIGHT
    nop                                                 nop
    nop                                                 bal         VI15, DO_FILTER_LIGHTING
    nop                                                 nop
CLIP_CHECK_FILTERLIGHT:

    nop                                                 iaddiu      VI07, vi00, 0                ; Get a pointer to the Clip Buffer
    nop                                                 iaddiu      VI06, vi00, ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) )                ; Get a pointer to the Next Buffer

    nop                                                 isw.x       VI07,   ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)        ; store the buffer pointers
    nop                                                 isw.y       VI05,   ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)

    nop                                                 b           VU1_RIGID_XFORM_CODE_START+0x0040 -0x3000+16384
    nop                                                 isw.z       VI06,   ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)

;==============================================================================
;
;   Lighting - This loop does 3 distance-based points--normals are ignored, so
;              all vertices get the same intensity.
;
;==============================================================================




















DO_POINT_LIGHTING_CHEAP:

    ; load the light information (its all packed into zw of the uvs, so this should be fun...)
    nop                                                 lq.zw       VF10zw,   9+16*0(VI05)
    nop                                                 lq.zw       VF11zw,   9+16*1(VI05)
    nop                                                 lq.zw       VF12zw,   9+16*2(VI05)
    nop                                                 lq.zw       VF13zw,   9+4*2+16*0(VI05)
    nop                                                 lq.zw       VF14zw,   9+4*2+16*1(VI05)
    nop                                                 lq.zw       VF15zw,   9+4*2+16*2(VI05)
    nop                                                 mr32.xyzw   VF10xyzw, VF10xyzw
    nop                                                 mr32.xyzw   VF13xyzw, VF13xyzw
    nop                                                 mr32.xyzw   VF11xyzw, VF11xyzw
    nop                                                 mr32.xyzw   VF14xyzw, VF14xyzw
    nop                                                 mr32.xyzw   VF12xyzw, VF12xyzw
    nop                                                 mr32.xyzw   VF15xyzw, VF15xyzw
    nop                                                 mr32.xyzw   VF10xyzw, VF10xyzw
    nop                                                 mr32.xyzw   VF13xyzw, VF13xyzw
    nop                                                 mr32.xyzw   VF11xyzw, VF11xyzw
    nop                                                 mr32.xyzw   VF14xyzw, VF14xyzw
    nop                                                 mr32.xyzw   VF12xyzw, VF12xyzw
    nop                                                 mr32.xyzw   VF15xyzw, VF15xyzw
    nop                                                 lq.zw       VF10zw,   9+4+16*0(VI05)
    nop                                                 lq.zw       VF11zw,   9+4+16*1(VI05)
    nop                                                 lq.zw       VF12zw,   9+4+16*2(VI05)
    nop                                                 lq.zw       VF13zw,   9+4*3+16*0(VI05)
    nop                                                 lq.zw       VF14zw,   9+4*3+16*1(VI05)
    nop                                                 lq.zw       VF15zw,   9+4*3+16*2(VI05)

    ; prepare for the loop
    addw.xyz    VF09xyz,  VF00xyz,    VF00w           ilw.z       VI04, 0(VI05)
    mulw.w      VF09w,    VF00w,      VF00w           iaddiu      VI08, VI05, 0x00
    nop                                                 iaddiu      VI01, VI00, 0x7F
    nop                                                 iand        VI02, VI04, VI01
    nop                                                 iadd        VI09, VI08, VI02
    mulw.x      VF23x,    VF09x,    VF10w         iadd        VI09, VI09, VI02
    mulw.y      VF23y,    VF09y,    VF11w         iadd        VI09, VI09, VI02
    mulw.z      VF23z,    VF09z,    VF12w         iadd        VI09, VI09, VI02
    nop                                                 loi         -1.0
    addi.y      VF22y,    VF00y,      I               loi         0x4efe0000
    addi.x      VF22x,    VF00x,      I               nop

POINT_LIGHT_CHEAP_LOOP:
    nop                                                 lq.xyz      VF16xyz,  11(VI08)
    nop                                                 lq.xyz      VF17xyz,  10(VI08)
    nop                                                 nop
    nop                                                 nop
    sub.xyz     VF20xyz,  VF12xyz,  VF16xyz       nop
    sub.xyz     VF19xyz,  VF11xyz,  VF16xyz       nop
    sub.xyz     VF18xyz,  VF10xyz,  VF16xyz       nop
    itof0.xyz   VF17xyz,  VF17xyz                   nop
    mul.xyz     VF20xyz,  VF20xyz,  VF20xyz       nop
    mul.xyz     VF19xyz,  VF19xyz,  VF19xyz       nop
    mul.xyz     VF18xyz,  VF18xyz,  VF18xyz       nop
    nop                                                 nop
    adday.z     ACC,        VF20z,    VF20y         nop
    maddx.z     VF21z,    VF09z,    VF20x         nop
    addaz.y     ACC,        VF19y,    VF19z         nop
    maddx.y     VF21y,    VF09y,    VF19x         nop
    addaz.x     ACC,        VF18x,    VF18z         nop
    maddy.x     VF21x,    VF09x,    VF18y         nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    mul.xyz     VF21xyz,  VF21xyz,  VF23xyz       nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    miniw.xyz   VF21xyz,  VF21xyz,  VF00w           nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    sub.xyz     VF21xyz,  VF09xyz,  VF21xyz       nop
    nop                                                 nop
    nop                                                 nop
    mulaw.xyz   ACC,        VF17xyz,  VF00w           nop
    maddaz.xyz  ACC,        VF15xyz,  VF21z         nop
    madday.xyz  ACC,        VF14xyz,  VF21y         nop
    maddx.xyz   VF17xyz,  VF13xyz,  VF21x         nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 loi     255.0
    minii.xyz   VF17xyz,  VF17xyz,  I               nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    ftoi0.xyz   VF17xyz,  VF17xyz                   nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 sq.zw       VF09zw,   9(VI08)
    nop                                                 sq.xyz      VF17xyz,  10(VI08)
    nop                                                 iaddiu      VI08,   VI08,   4
    nop                                                 nop
    nop                                                 ibne        VI08,   VI09,   POINT_LIGHT_CHEAP_LOOP
    nop                                                 nop



















    nop                                                 jr          VI15
    nop                                                 nop

;==============================================================================
;
;   Filter Lighting - This code modulates the vertex color based on a fixed
;   filter light color.
;
;==============================================================================











DO_FILTER_LIGHTING:
    nop                                                 ilw.z   VI04, 0(VI05)
    nop                                                 lq.xyzw VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 10 )(vi00)
    nop                                                 iaddiu  VI08, VI05, 0x00
    nop                                                 iaddiu  VI01, vi00, 0x7f
    nop                                                 iand    VI02, VI04, VI01
    nop                                                 iadd    VI09, VI08, VI02
    nop                                                 iadd    VI09, VI09, VI02
    nop                                                 iadd    VI09, VI09, VI02
    nop                                                 iadd    VI09, VI09, VI02
    nop                                                 loi     255.0
    
    ; loop preamble
    nop                                                 lq.xyz  VF10xyz,  10(VI08)                   ;                   ; load color 0
    nop                                                 nop                                                 ;
    nop                                                 nop                                                 ;
    nop                                                 nop                                                 ;
    itof0.xyz       VF11xyz,  VF10xyz               nop                                                 ; itof color 0

    nop                                                 lq.xyz  VF10xyz,  10+(4*1)(VI08) ;                   ; load color 1
    nop                                                 nop                                                 ;
    nop                                                 nop                                                 ;
    mul.xyz         VF12xyz,  VF11xyz,  VF09xyz   nop                                                 ; mul color 0
    itof0.xyz       VF11xyz,  VF10xyz               nop                                                 ; itof color 1

    nop                                                 lq.xyz  VF10xyz,  10+(4*2)(VI08) ;                   ; load color 2
    nop                                                 nop                                                 ;
    minii.xyz       VF13xyz,  VF12xyz,  i           nop                                                 ; clamp color 0
    mul.xyz         VF12xyz,  VF11xyz,  VF09xyz   nop                                                 ; mul color 1
    itof0.xyz       VF11xyz,  VF10xyz               nop                                                 ; itof color 2

    nop                                                 lq.xyz  VF10xyz,  10+(4*3)(VI08) ;                   ; load color 3
    add.xyz         VF14xyz,  VF13xyz,  vf00xyz     nop                                                 ; copy color 0
    minii.xyz       VF13xyz,  VF12xyz,  i           nop                                                 ; clamp color 1
    mul.xyz         VF12xyz,  VF11xyz,  VF09xyz   nop                                                 ; mul color 2
    itof0.xyz       VF11xyz,  VF10xyz               nop                                                 ; itof color 3
    
FILTER_LIGHTING_LOOP:
    ftoi0.xyz       VF15xyz,  VF14xyz               lq.xyz  VF10xyz,  10+(4*4)(VI08) ; ftoi color 0      ; load color 4
    add.xyz         VF14xyz,  VF13xyz,  vf00xyz     iaddiu  VI08, VI08, 4                   ; copy color 1      ; vptr++
    minii.xyz       VF13xyz,  VF12xyz,  i           nop                                                 ; clamp color 2
    mul.xyz         VF12xyz,  VF11xyz,  VF09xyz   ibne    VI08, VI09, FILTER_LIGHTING_LOOP            ; mul color 3       ; loop
    itof0.xyz       VF11xyz,  VF10xyz               sq.xyz  VF15xyz,  10-4(VI08)     ; itof color 4      ; store color 0

    nop                                                 jr          VI15
    nop                                                 nop











VU1_RIGID_XFORM_CODE_END:
