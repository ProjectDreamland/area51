#line 1 ".\\vu1\\mcode\\shadreceive_passes.vu"













#line 1 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"





























































                                    













                                    



































































































#line 177 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"











#line 189 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































#line 314 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































































































































































































































































































































































































#line 791 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"

#line 15 ".\\vu1\\mcode\\shadreceive_passes.vu"
#line 1 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\shadow_include.vu"

















;==============================================================================
;  Layout of shad_material Structure
;==============================================================================


































































#line 16 ".\\vu1\\mcode\\shadreceive_passes.vu"

.vu 
.org 0x0FC0
.align 4 
.global VU1_SHADRECEIVE_PASSES_CODE_START
.global VU1_SHADRECEIVE_PASSES_CODE_END

VU1_SHADRECEIVE_PASSES_CODE_START:

    ; setup
    nop                                                 ilw.x   VI03,   ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 0 )(vi00)
    nop                                                 ilw.w   VI09,   0(VI05)

    ;==============================================================================
    ; kick a shadow pass that primes the z-buffer for us. it is a super-simple kick
    ;==============================================================================
    nop                                                 iaddiu      VI10, vi00, 0x7FFF
    nop                                                 iaddiu      VI10, VI10, 1
    nop                                                 ior         VI12, VI10, VI02
    nop                                                 iaddiu      VI11, vi00, 1
    nop                                                 lq          VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )(vi00)
    nop                                                 iaddiu      VI01, vi00, 0x3F
    nop                                                 mfir.z      VF10z,    VI01
    nop                                                 lq          VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 1 )(vi00)
    nop                                                 mfir.x      VF11x,    VI12
    nop                                                 mfir.x      VF09x,    VI11
    nop                                                 iaddiu      VI13, VI05, 5
    nop                                                 sq          VF09xyzw, 0(VI13)
    nop                                                 sq          VF10xyzw, 1(VI13)
    nop                                                 sq          VF11xyzw, 2(VI13)
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 xgkick      VI13

    ;==============================================================================
    ;  Shadow 0
    ;==============================================================================
    nop                                                 iaddiu  VI01,       vi00,   0x0001
    nop                                                 iand    VI01,       VI09,   VI01
    nop                                                 nop
    nop                                                 ibeq    VI01,       vi00,   SKIP_SHADOW_RECEIVE_0
    nop                                                 nop
    nop                                                 lq.xyzw VF31xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 7 )(vi00)
    nop                                                 lq.xyzw VF30xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 6 )(vi00)
    nop                                                 lq.xyzw VF29xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 5 )(vi00)
    nop                                                 lq.xyzw VF28xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 4 )(vi00)
    nop                                                 bal     VI15,       VU1_SHADOW_RECEIVE_XFORM
    nop                                                 nop
    nop                                                 bal     VI15,       VU1_SHADOW_RECEIVE_KICK
    nop                                                 lq      VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 3 )(vi00)

    nop                                                 iaddiu  VI01, vi00, (0x0002|0x0004|0x0008|0x0010)
    nop                                                 iand    VI01, VI09, VI01
    nop                                                 nop
    nop                                                 ibeq    VI01,       vi00,   SKIP_NEXT_RECEIVE_PASSES
    nop                                                 iaddiu  VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )
    nop                                                 xgkick  VI01

    SKIP_SHADOW_RECEIVE_0:

    ;==============================================================================
    ;  Shadow 1
    ;==============================================================================
    nop                                                 iaddiu  VI01,       vi00,   0x0002
    nop                                                 iand    VI01,       VI09,   VI01
    nop                                                 nop
    nop                                                 ibeq    VI01,       vi00,   SKIP_SHADOW_RECEIVE_1
    nop                                                 nop
    nop                                                 lq.xyzw VF31xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 12 )(vi00)
    nop                                                 lq.xyzw VF30xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 11 )(vi00)
    nop                                                 lq.xyzw VF29xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 10 )(vi00)
    nop                                                 lq.xyzw VF28xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 9 )(vi00)
    nop                                                 bal     VI15,       VU1_SHADOW_RECEIVE_XFORM
    nop                                                 nop
    nop                                                 bal     VI15,       VU1_SHADOW_RECEIVE_KICK
    nop                                                 lq      VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 8 )(vi00)

    nop                                                 iaddiu  VI01, vi00, (0x0004|0x0008|0x0010)
    nop                                                 iand    VI01, VI09, VI01
    nop                                                 nop
    nop                                                 ibeq    VI01,       vi00,   SKIP_NEXT_RECEIVE_PASSES
    nop                                                 iaddiu  VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )
    nop                                                 xgkick  VI01

    SKIP_SHADOW_RECEIVE_1:

    ;==============================================================================
    ;  Shadow 2
    ;==============================================================================
    nop                                                 iaddiu  VI01,       vi00,   0x0004
    nop                                                 iand    VI01,       VI09,   VI01
    nop                                                 nop
    nop                                                 ibeq    VI01,       vi00,   SKIP_SHADOW_RECEIVE_2
    nop                                                 nop
    nop                                                 lq.xyzw VF31xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 17 )(vi00)
    nop                                                 lq.xyzw VF30xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 16 )(vi00)
    nop                                                 lq.xyzw VF29xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 15 )(vi00)
    nop                                                 lq.xyzw VF28xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 14 )(vi00)
    nop                                                 bal     VI15,       VU1_SHADOW_RECEIVE_XFORM
    nop                                                 nop
    nop                                                 bal     VI15,       VU1_SHADOW_RECEIVE_KICK
    nop                                                 lq      VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 13 )(vi00)

    nop                                                 iaddiu  VI01, vi00, (0x0008|0x0010)
    nop                                                 iand    VI01, VI09, VI01
    nop                                                 nop
    nop                                                 ibeq    VI01,       vi00,   SKIP_NEXT_RECEIVE_PASSES
    nop                                                 iaddiu  VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )
    nop                                                 xgkick  VI01

    SKIP_SHADOW_RECEIVE_2:

    ;==============================================================================
    ;  Shadow 3
    ;==============================================================================
    nop                                                 iaddiu  VI01,       vi00,   0x0008
    nop                                                 iand    VI01,       VI09,   VI01
    nop                                                 nop
    nop                                                 ibeq    VI01,       vi00,   SKIP_SHADOW_RECEIVE_3
    nop                                                 nop
    nop                                                 lq.xyzw VF31xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 22 )(vi00)
    nop                                                 lq.xyzw VF30xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 21 )(vi00)
    nop                                                 lq.xyzw VF29xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 20 )(vi00)
    nop                                                 lq.xyzw VF28xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 19 )(vi00)
    nop                                                 bal     VI15,       VU1_SHADOW_RECEIVE_XFORM
    nop                                                 nop
    nop                                                 bal     VI15,       VU1_SHADOW_RECEIVE_KICK
    nop                                                 lq      VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 18 )(vi00)

    nop                                                 iaddiu  VI01, vi00, (0x0010)
    nop                                                 iand    VI01, VI09, VI01
    nop                                                 nop
    nop                                                 ibeq    VI01,       vi00,   SKIP_NEXT_RECEIVE_PASSES
    nop                                                 iaddiu  VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )
    nop                                                 xgkick  VI01

    SKIP_SHADOW_RECEIVE_3:

    ;==============================================================================
    ;  Shadow 4
    ;==============================================================================
    nop                                                 iaddiu  VI01,       vi00,   0x0010
    nop                                                 iand    VI01,       VI09,   VI01
    nop                                                 nop
    nop                                                 ibeq    VI01,       vi00,   SKIP_SHADOW_RECEIVE_4
    nop                                                 nop
    nop                                                 lq.xyzw VF31xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 27 )(vi00)
    nop                                                 lq.xyzw VF30xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 26 )(vi00)
    nop                                                 lq.xyzw VF29xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 25 )(vi00)
    nop                                                 lq.xyzw VF28xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 24 )(vi00)
    nop                                                 bal     VI15,       VU1_SHADOW_RECEIVE_XFORM
    nop                                                 nop
    nop                                                 bal     VI15,       VU1_SHADOW_RECEIVE_KICK
    nop                                                 lq      VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 23 )(vi00)
    SKIP_SHADOW_RECEIVE_4:
    SKIP_NEXT_RECEIVE_PASSES:

    ; should we return to the clipper, or are we done?
    nop                                                 iaddiu      VI01, vi00, (1 << 13)
    nop                                                 iand        VI01, VI03, VI01
    nop                                                 nop
    nop                                                 ibeq        VI01, vi00, FINISHED_RECEIVE
    nop                                                 nop
    nop                                                 iaddiu  VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )
    nop                                                 xgkick  VI01
    nop                                                 ilw.w       VI15, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 39 )(vi00)
    nop                                                 jr          VI15
    nop                                                 nop
    FINISHED_RECEIVE:

    nop                                                 nop
    nop[E]                                              nop
    nop                                                 nop


;==============================================================================
;  Shadow receive uv calculation:
;==============================================================================















VU1_SHADOW_RECEIVE_XFORM:
    ; concatenate the l2w and w2s matrices
    mulaw.xyzw      acc,        VF31xyzw, VF08w     iadd    VI11, vi00, VI05
    maddaz.xyzw     acc,        VF30xyzw, VF08z     iaddiu  VI12, VI06, 0x00
    madday.xyzw     acc,        VF29xyzw, VF08y     iadd    VI10, VI11, VI02
    maddx.xyzw      VF12xyzw, VF28xyzw, VF08x     iadd    VI10, VI10, VI02
    mulaw.xyzw      acc,        VF31xyzw, VF07w     iadd    VI10, VI10, VI02
    maddaz.xyzw     acc,        VF30xyzw, VF07z     iadd    VI10, VI10, VI02
    madday.xyzw     acc,        VF29xyzw, VF07y     iaddiu  VI01, vi00, 0x80
    maddx.xyzw      VF11xyzw, VF28xyzw, VF07x     mfir.w  VF20w, VI01
    mulaw.xyzw      acc,        VF31xyzw, VF06w     mfir.z  VF20z, VI01
    maddaz.xyzw     acc,        VF30xyzw, VF06z     mfir.y  VF20y, VI01
    madday.xyzw     acc,        VF29xyzw, VF06y     mfir.x  VF20x, VI01
    maddx.xyzw      VF10xyzw, VF28xyzw, VF06x     iaddiu  VI13, vi00, 0x20
    mulaw.xyzw      acc,        VF31xyzw, VF05w     nop
    maddaz.xyzw     acc,        VF30xyzw, VF05z     nop
    madday.xyzw     acc,        VF29xyzw, VF05y     nop
    maddx.xyzw      VF09xyzw, VF28xyzw, VF05x     nop

    ; loop preamble - handles the first two verts
    mulx.xyzw       VF18xyzw, vf00xyzw,   vf00x       lq.xyzw VF17xyzw, 11(VI12)       ; sub=0             ; load vert0
    mulx.xyzw       VF21xyzw, vf00xyzw,   vf00x       lq.z    VF23z,    9(VI11)       ; sub=0             ; load q0
    nop                                                 nop
    mulaw.xyzw      acc,        VF12xyzw, vf00w       nop                                     ; xform vert0
    maddaz.xyzw     acc,        VF11xyzw, VF17z     nop                                     ; xform vert0
    madday.xyzw     acc,        VF10xyzw, VF17y     nop                                     ; xform vert0
    maddx.xyzw      VF22xyzw, VF09xyzw, VF17x     nop                                     ; xform vert0
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    mulz.xy         VF23xy,   VF22xy,   VF23z     nop                                     ; project uv0
    add.xyz         VF19xyz,  VF22xyz,  vf00xyz     nop                                     ; prev pos = pos0
    nop                                                 nop
    nop                                                 sq.xyzw VF20xyzw, 10(VI11)                           ; store vert 0
    nop                                                 sq.xyz  VF23xyz,  9(VI11)                           ; store vert 0
    nop                                                 iaddiu  VI11, VI11, 4                           ; vptr++
    nop                                                 iaddiu  VI12, VI12, 4                           ; snxt++

    nop                                                 lq.xyzw VF17xyzw, 11(VI12)                           ; load vert1
    nop                                                 lq.z    VF23z,    9(VI11)                           ; load q1
    nop                                                 nop
    mulaw.xyzw      acc,        VF12xyzw, vf00w       nop                                     ; xform vert1
    maddaz.xyzw     acc,        VF11xyzw, VF17z     nop                                     ; xform vert1
    madday.xyzw     acc,        VF10xyzw, VF17y     nop                                     ; xform vert1
    maddx.xyzw      VF22xyzw, VF09xyzw, VF17x     nop                                     ; xform vert1
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    mulz.xy         VF23xy,   VF22xy,   VF23z     nop                                     ; project uv1
    sub.xy          VF18xy,   VF22xy,   VF19xy    nop                                     ; vert1-vert0
    add.xyz         VF19xyz,  VF22xyz,  vf00xyz     sq.xyzw VF20xyzw, 10(VI11)       ; prev pos = pos1   ; store vert 1
    nop                                                 sq.xyz  VF23xyz,  9(VI11)                           ; store vert 1
    nop                                                 iaddiu  VI11, VI11, 4                           ; vptr++
    nop                                                 iaddiu  VI12, VI12, 4

SHADOW_RECEIVE_XFORM_LOOP:
    nop                                                 lq.xyzw VF17xyzw, 11(VI12)                           ; load vert2
    nop                                                 lq.z    VF23z,    9(VI11)                           ; load q2
    nop                                                 nop
    mulaw.xyzw      acc,        VF12xyzw, vf00w       nop                                     ; xform vert2
    maddaz.xyzw     acc,        VF11xyzw, VF17z     mtir    VI08,   VF17w                 ; xform vert2       ; grab out adc and w bit
    madday.xyzw     acc,        VF10xyzw, VF17y     nop                                     ; xform vert2
    maddx.xyzw      VF22xyzw, VF09xyzw, VF17x     nop                                     ; xform vert2
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    mulz.xy         VF23xy,   VF22xy,   VF23z     fmand   VI14, VI13                          ; project uv2
    sub.xy          VF21xy,   VF22xy,   VF19xy    nop    ; vert2-vert1
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    opmula.xyz      acc,        VF18xyz,  VF21xyz   nop                                     ; cross product
    opmsub.xyz      vf00xyz,    VF21xyz,  VF18xyz   nop                                     ; cross product
    add.xy          VF18xy,   VF21xy,   vf00xy      nop;ibne    VI14, vi00, REJECT_SHADOW_POLY  ; psub = csub
    add.xyz         VF19xyz,  VF22xyz,  vf00xyz     nop                                     ; prev pos = pos2
    nop                                                 iand    VI01,   VI08, VI13                                  ; mask out winding bit
    nop                                                 fmand   VI14, VI13                                          ; grab sign of z
    nop                                                 iadd    VI01, VI14, VI01                                    ; sign + winding bit
    nop                                                 iand    VI01, VI01, VI13                                    ; winding bit masked out

    nop                                                 nop
    nop                                                 ibne    VI01, VI13, NOT_BACKFACE
    nop                                                 nop
    nop                                                 iaddiu  VI01, vi00, 0x7fff
    nop                                                 iaddiu  VI01, VI01, 1
    nop                                                 iand    VI01, VI08, VI01
    nop                                                 nop
    nop                                                 ibne    VI01, vi00, NOT_BACKFACE
    ;
    nop                                                 nop

REJECT_SHADOW_POLY:
    nop                                                 iaddiu  VI01, VI01, 0xff
    nop                                                 isw.w   VI01, 10-(4*2)(VI11)
    nop                                                 isw.w   VI01, 10-(4*1)(VI11)
    nop                                                 b       BACKFACE
    nop                                                 isw.w   VI01, 10(VI11)

NOT_BACKFACE:
    nop                                                 sq.w    VF20w,   10(VI11)
BACKFACE:
    nop                                                 sq.xyz  VF20xyz,  10(VI11)                           ; store vert 2
    nop                                                 sq.xyz  VF23xyz,  9(VI11)                           ; store vert 2
    nop                                                 iaddiu  VI11, VI11, 4
    nop                                                 iaddiu  VI12, VI12, 4
    nop                                                 ibne    VI11, VI10, SHADOW_RECEIVE_XFORM_LOOP
    nop                                                 nop

    nop                                                 jr      VI15
    nop                                                 nop

















;==============================================================================
;  Shadow receive kicking:
;==============================================================================
VU1_SHADOW_RECEIVE_KICK:
    nop                                                 iaddiu      VI10, vi00, 0x7FFF                      ; Set EOP Bit
    nop                                                 iaddiu      VI10, VI10, 1
    nop                                                 ior         VI12, VI10, VI02                        ; EOP | Num Vertices
    nop                                                 iaddiu      VI11, vi00, 2
    nop                                                 lq          VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )(vi00)       ; Load 1 GS Register
    nop                                                 iaddiu      VI01, vi00, 0x3F
    nop                                                 mfir.z      VF10z,    VI01
    ;                                                   lq          TF02, blah -- TF02 was already set to the proper tex0 register
    nop                                                 lq          VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 2 )(vi00)        ; Strip: Opaque/Alpha | Context1 | UV0
    nop                                                 mfir.x      VF12x,    VI12                        ; Set EOP and Num Vertices in GIFTAG
    nop                                                 mfir.x      VF09x,    VI11                        ; Set 2 registers

    ; stall the previous pass    
    nop                                                 iaddiu  VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )
    nop                                                 xgkick  VI01
    
    nop                                                 iaddiu      VI13, VI05, 4         ; Get start address of GIFTAG
    nop                                                 sq          VF09xyzw, 0(VI13)                     ; Build GS chain
    nop                                                 sq          VF10xyzw, 1(VI13)
    nop                                                 sq          VF11xyzw, 2(VI13)
    nop                                                 sq          VF12xyzw, 3(VI13)
    nop                                                 nop
    nop                                                 nop
    nop                                                 jr          VI15
    nop                                                 xgkick      VI13                                    ; Kick GS chain

VU1_SHADRECEIVE_PASSES_CODE_END:
