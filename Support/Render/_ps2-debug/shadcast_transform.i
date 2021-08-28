#line 1 ".\\vu1\\mcode\\shadcast_transform.vu"















#line 1 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"





























































                                    













                                    



































































































#line 177 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"











#line 189 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































#line 314 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































































































































































































































































































































































































#line 791 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"

#line 17 ".\\vu1\\mcode\\shadcast_transform.vu"
#line 1 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\shadow_include.vu"

















;==============================================================================
;  Layout of shad_material Structure
;==============================================================================


































































#line 18 ".\\vu1\\mcode\\shadcast_transform.vu"

.vu
.org        0x3000
.align      4 
.global     VU1_SHADCAST_XFORM_CODE_START
.global     VU1_SHADCAST_XFORM_CODE_END

.global     VU1_ENTRY_SHAD_CAST_SETUP_MATRIX
.global     VU1_ENTRY_SHAD_CAST_2BONES
.global     VU1_ENTRY_SHAD_CAST_1BONE
.equ        VU1_ENTRY_SHAD_CAST_SETUP_MATRIX,   ((SHAD_CAST_SETUP_MATRIX-VU1_SHADCAST_XFORM_CODE_START+0x3000)/8)
.equ        VU1_ENTRY_SHAD_CAST_2BONES,         ((SHAD_CAST_2BONES      -VU1_SHADCAST_XFORM_CODE_START+0x3000)/8)
.equ        VU1_ENTRY_SHAD_CAST_1BONE,          ((SHAD_CAST_1BONE       -VU1_SHADCAST_XFORM_CODE_START+0x3000)/8)



VU1_SHADCAST_XFORM_CODE_START:






SHAD_CAST_SETUP_MATRIX:
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 0
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 4
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 8
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 12
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 16
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 20
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 24
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 28
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 32
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 36
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 40
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 44
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 48
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 52
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 56
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 60
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 64
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 68
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 72
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 76
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 80
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 84
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 88
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 92
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 96
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 100
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 104
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 108
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 112
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 116
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 120
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 124
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 128
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 132
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 136
    nop                                                 b       SC_SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 140
SC_SETUP_MATRIX:
    nop                                                 loi     -786432.0
    addi.xyz    VF13xyz,  vf00,       i               lq.xyzw VF12xyzw, 3(VI01)
    nop                                                 lq.xyzw VF11xyzw, 2(VI01)
    nop                                                 lq.xyzw VF10xyzw, 1(VI01)
    nop                                                 lq.xyzw VF09xyzw, 0(VI01)
    mulaw.xyzw  acc,        VF12xyzw, vf00w           nop
    maddaz.xyzw acc,        VF11xyzw, VF13z         nop
    madday.xyzw acc,        VF10xyzw, VF13y         nop
    maddx.xyzw  VF12xyzw, VF09xyzw, VF13x         nop
    nop                                                 nop
    nop                                                 nop
    nop[e]                                              nop
    nop                                                 sq.xyzw VF12xyzw, 3(VI01)





SHAD_CAST_2BONES:
    nop                                                 xtop    VI05
    nop[e]                                              nop
    nop                                                 nop
    nop                                                 b       VU1_DO_CASTING
    nop                                                 iaddiu  VI08,   vi00,   ((VU1_SHADOW_SKIN_CAST_XFORM)-SHAD_CAST_SETUP_MATRIX+0x3000)





SHAD_CAST_1BONE:
    nop                                                 xtop    VI05
    nop[e]                                              nop
    nop                                                 nop
    nop                                                 b       VU1_DO_CASTING
    nop                                                 iaddiu  VI08,   vi00,   ((VU1_SHADOW_SKIN_CAST_XFORM_1BONE)-SHAD_CAST_SETUP_MATRIX+0x3000)






VU1_DO_CASTING:
    ; load up the shadow flags
    nop                                                 ilw.z   VI04,   0(VI05)
    nop                                                 lq.xyzw VF08xyzw, 1+3(VI05)
    nop                                                 lq.xyzw VF07xyzw, 1+2(VI05)
    nop                                                 lq.xyzw VF06xyzw, 1+1(VI05)
    nop                                                 lq.xyzw VF05xyzw, 1+0(VI05)
    nop                                                 iaddiu  VI01,   vi00,   0x7f
    nop                                                 iand    VI02,   VI04,   VI01
    nop                                                 ilw.w   VI09,   0(VI05)

    ; transform the shadows
    nop                                                 lq.xzyw VF31xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 7 )(vi00)
    nop                                                 lq.xzyw VF30xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 6 )(vi00)
    nop                                                 lq.xzyw VF29xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 5 )(vi00)
    nop                                                 lq.xzyw VF28xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 4 )(vi00)
    nop                                                 jalr    VI15,       VI08
    nop                                                 nop

    ; kick the shadows
    nop                                                 bal     VI15,       VU1_SHADOW_CAST_KICK
    nop                                                 lq      VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 3 )(vi00)
    nop                                                 nop
    nop                                                 nop
    nop[E]                                              nop
    nop                                                 nop

;==============================================================================
;  Skin shadow casting transform
;==============================================================================























VU1_SHADOW_SKIN_CAST_XFORM:

    ; concatenate the l2w and w2s matrices
    mulaw.xyzw      acc,        VF31xyzw, VF08w     iadd    VI11, vi00, VI05
    maddaz.xyzw     acc,        VF30xyzw, VF08z     iadd    VI10, VI11, VI02
    madday.xyzw     acc,        VF29xyzw, VF08y     iadd    VI10, VI10, VI02
    maddx.xyzw      VF12xyzw, VF28xyzw, VF08x     iadd    VI10, VI10, VI02
    mulaw.xyzw      acc,        VF31xyzw, VF07w     iadd    VI10, VI10, VI02
    maddaz.xyzw     acc,        VF30xyzw, VF07z     iaddiu  VI01, vi00, 0x80
    madday.xyzw     acc,        VF29xyzw, VF07y     mfir.w  VF16w,    VI01
    maddx.xyzw      VF11xyzw, VF28xyzw, VF07x     mfir.z  VF16z,    vi00
    mulaw.xyzw      acc,        VF31xyzw, VF06w     mfir.y  VF16y,    vi00
    maddaz.xyzw     acc,        VF30xyzw, VF06z     mfir.x  VF16x,    vi00
    madday.xyzw     acc,        VF29xyzw, VF06y     loi     16.062745098039215686               ; 16*256/255
    maddx.xyzw      VF10xyzw, VF28xyzw, VF06x     nop
    mulaw.xyzw      acc,        VF31xyzw, VF05w     nop
    maddaz.xyzw     acc,        VF30xyzw, VF05z     nop
    madday.xyzw     acc,        VF29xyzw, VF05y     nop
    maddx.xyzw      VF09xyzw, VF28xyzw, VF05x     nop


    ; loop preamble
    nop                                                 lq.xyzw VF17xyzw, 10(VI11)                   ;               ; load weights0
    nop                                                 lq.xyzw VF13xyzw, 11(VI11)                   ;               ; load xyz0    
    nop                                                 nop                                                 ; 
    nop                                                 nop                                                 ; 
    itof12.zw       VF17zw,   VF17zw                mtir    VI13, VF17x                               ; wght0->float  ; load bi0
    nop                                                 mtir    VI14, VF17y                               ;               ; load bi0
    nop                                                 lq.xyzw VF22xyzw, 3(VI13)                         ;               ; load bone0_0
    nop                                                 lq.xyzw VF21xyzw, 2(VI13)                         ;               ; load bone0_0
    muli.zw         VF18zw,   VF17zw,   i           lq.xyzw VF20xyzw, 1(VI13)                         ; scale wght0   ; load bone0_0
    nop                                                 lq.xyzw VF19xyzw, 0(VI13)                         ;               ; load bone0_0
    nop                                                 lq.xyzw VF26xyzw, 3(VI14)                         ;               ; load bone0_1
    nop                                                 lq.xyzw VF25xyzw, 2(VI14)                         ;               ; load bone0_1
    mulz.xyz        VF14xyz,  VF13xyz,  VF18z     lq.xyzw VF24xyzw, 1(VI14)                         ; xyz0*wght0_0  ; load bone0_1
    nop                                                 lq.xyzw VF23xyzw, 0(VI14)                         ;               ; load bone0_1
    mulw.xyz        VF15xyz,  VF13xyz,  VF18w     nop                                                 ; xyz0*wght0_1
    mulaz.xyz       acc,        VF22xyz,  VF18z     nop                                                 ; xyz0*bone0
    maddaz.xyz      acc,        VF21xyz,  VF14z     nop                                                 ; xyz0*bone0
    madday.xyz      acc,        VF20xyz,  VF14y     nop                                                 ; xyz0*bone0

    maddax.xyz      acc,        VF19xyz,  VF14x     lq.xyzw VF17xyzw, 10+(4*1)(VI11) ; xyz0*bone0    ; load weights1
    maddaw.xyz      acc,        VF26xyz,  VF18w     lq.xyzw VF13xyzw, 11+(4*1)(VI11) ; xyz0*bone1    ; load xyz1     
    maddaz.xyz      acc,        VF25xyz,  VF15z     nop                                                 ; xyz0*bone1
    madday.xyz      acc,        VF24xyz,  VF15y     nop                                                 ; xyz0*bone1
    itof12.zw       VF17zw,   VF17zw                mtir    VI13, VF17x                               ; wght1->float  ; load bi1
    maddx.xyz       VF27xyz,  VF23xyz,  VF15x     mtir    VI14, VF17y                               ; xyz0*bone1    ; load bi1
    nop                                                 lq.xyzw VF22xyzw, 3(VI13)                         ;               ; load bone1_0
    mulaw.xyzw      acc,        VF12xyzw, vf00w       lq.xyzw VF21xyzw, 2(VI13)                         ; xform xyz0    ; load bone1_0
    muli.zw         VF18zw,   VF17zw,   i           lq.xyzw VF20xyzw, 1(VI13)                         ; scale wght1   ; load bone1_0
    maddaz.xyzw     acc,        VF11xyzw, VF27z     lq.xyzw VF19xyzw, 0(VI13)                         ; xform xyz0    ; load bone1_0
    madday.xyzw     acc,        VF10xyzw, VF27y     lq.xyzw VF26xyzw, 3(VI14)                         ; xform xyz0    ; load bone1_1
    maddx.xyzw      VF28xyzw, VF09xyzw, VF27x     lq.xyzw VF25xyzw, 2(VI14)                         ; xform xyz0    ; load bone1_1
    mulz.xyz        VF14xyz,  VF13xyz,  VF18z     lq.xyzw VF24xyzw, 1(VI14)                         ; xyz1*wght1_0  ; load bone1_1
    nop                                                 lq.xyzw VF23xyzw, 0(VI14)                         ;               ; load bone1_1
    mulw.xyz        VF15xyz,  VF13xyz,  VF18w     nop                                                 ; xyz1*wght1_1
    mulaz.xyz       acc,        VF22xyz,  VF18z     div     q,  vf00w,  VF28w                         ; xyz1*bone0    ; div0
    maddaz.xyz      acc,        VF21xyz,  VF14z     nop                                                 ; xyz1*bone0
    madday.xyz      acc,        VF20xyz,  VF14y     nop                                                 ; xyz1*bone0

    ; tight loop
SHAD_CAST_SKINNING_LOOP:
    maddax.xyz      acc,        VF19xyz,  VF14x     lq.xyzw VF17xyzw, 10+(4*2)(VI11) ; xyz1*bone0    ; load weights2 
    maddaw.xyz      acc,        VF26xyz,  VF18w     lq.xyzw VF13xyzw, 11+(4*2)(VI11) ; xyz1*bone1    ; load xyz2     
    maddaz.xyz      acc,        VF25xyz,  VF15z     nop                                                 ; xyz1*bone1    ;
    madday.xyz      acc,        VF24xyz,  VF15y     iaddiu  VI11, VI11, 4                   ; xyz1*bone1    ; pCur++
    itof12.zw       VF17zw,   VF17zw                mtir    VI13, VF17x                               ; wght2->float  ; load bi2
    maddx.xyz       VF27xyz,  VF23xyz,  VF15x     mtir    VI14, VF17y                               ; xyz1*bone1    ; load bi2
    mulq.xyz        VF29xyz,  VF28xyz,  q           lq.xyzw VF22xyzw, 3(VI13)                         ; project xyz0  ; load bone2_0
    mulaw.xyzw      acc,        VF12xyzw, vf00w       lq.xyzw VF21xyzw, 2(VI13)                         ; xform xyz1    ; load bone2_0
    muli.zw         VF18zw,   VF17zw,   i           lq.xyzw VF20xyzw, 1(VI13)                         ; scale wght2   ; load bone2_0
    maddaz.xyzw     acc,        VF11xyzw, VF27z     lq.xyzw VF19xyzw, 0(VI13)                         ; xform xyz1    ; load bone2_0
    madday.xyzw     acc,        VF10xyzw, VF27y     lq.xyzw VF26xyzw, 3(VI14)                         ; xform xyz1    ; load bone2_1
    maddx.xyzw      VF28xyzw, VF09xyzw, VF27x     lq.xyzw VF25xyzw, 2(VI14)                         ; xform xyz1    ; load bone2_1
    mulz.xyz        VF14xyz,  VF13xyz,  VF18z     lq.xyzw VF24xyzw, 1(VI14)                         ; xyz2*wght2_0  ; load bone2_1
    ftoi4.xyz       VF29xyz,  VF29xyz               lq.xyzw VF23xyzw, 0(VI14)                         ; xyz0->fixed   ; load bone2_1
    mulw.xyz        VF15xyz,  VF13xyz,  VF18w     sq.xyzw VF16xyzw, 10-4(VI11)     ; xyz2*wght2_1  ; store rgb0
    mulaz.xyz       acc,        VF22xyz,  VF18z     div     q,  vf00w,  VF28w                         ; xyz2*bone0    ; div1
    maddaz.xyz      acc,        VF21xyz,  VF14z     ibne    VI11, VI10, SHAD_CAST_SKINNING_LOOP         ; xyz2*bone0    ; loop
    madday.xyz      acc,        VF20xyz,  VF14y     sq.xyz  VF29xyz,  11-4(VI11)     ; xyz2*bone0    ; store xyz0

    ; finished
    nop                                                 jr      VI15
    nop                                                 nop
























;==============================================================================
;  Skin shadow casting transform for a single bone
;==============================================================================

















VU1_SHADOW_SKIN_CAST_XFORM_1BONE:
    ; concatenate the l2w and w2s matrices
    mulaw.xyzw      acc,        VF31xyzw, VF08w     iadd    VI11, vi00, VI05
    maddaz.xyzw     acc,        VF30xyzw, VF08z     iadd    VI10, VI11, VI02
    madday.xyzw     acc,        VF29xyzw, VF08y     iadd    VI10, VI10, VI02
    maddx.xyzw      VF12xyzw, VF28xyzw, VF08x     iadd    VI10, VI10, VI02
    mulaw.xyzw      acc,        VF31xyzw, VF07w     iadd    VI10, VI10, VI02
    maddaz.xyzw     acc,        VF30xyzw, VF07z     iaddiu  VI01, vi00, 0x80
    madday.xyzw     acc,        VF29xyzw, VF07y     mfir.w  VF13w,    VI01
    maddx.xyzw      VF11xyzw, VF28xyzw, VF07x     mfir.z  VF13z,    vi00
    mulaw.xyzw      acc,        VF31xyzw, VF06w     mfir.y  VF13y,    vi00
    maddaz.xyzw     acc,        VF30xyzw, VF06z     mfir.x  VF13x,    vi00
    madday.xyzw     acc,        VF29xyzw, VF06y     loi     16.062745098039215686               ; 16*256/255
    maddx.xyzw      VF10xyzw, VF28xyzw, VF06x     nop
    mulaw.xyzw      acc,        VF31xyzw, VF05w     nop
    maddaz.xyzw     acc,        VF30xyzw, VF05z     nop
    madday.xyzw     acc,        VF29xyzw, VF05y     nop
    maddx.xyzw      VF09xyzw, VF28xyzw, VF05x     nop

    ; loop preamble
    nop                                                 ilw.x   VI13,       10(VI11)                     ;               ; load bptr0
    nop                                                 nop                                                     ;               ;
    nop                                                 nop                                                     ;               ;
    nop                                                 lq.xyzw VF18xyzw, 11(VI11)                       ;               ; load xyz0

    nop                                                 lq.xyzw VF17xyzw, 3(VI13)                             ;               ; load bone0
    nop                                                 lq.xyzw VF16xyzw, 2(VI13)                             ;               ; load bone0
    nop                                                 lq.xyzw VF15xyzw, 1(VI13)                             ;               ; load bone0
    nop                                                 lq.xyzw VF14xyzw, 0(VI13)                             ;               ; load bone0
    nop                                                 nop                                                     ;               ;
    nop                                                 nop                                                     ;               ;
    mulaw.xyz       acc,        VF17xyz,  vf00w       ilw.x   VI13,       10+(4*1)(VI11)   ; xform xyz0    ; load bptr1
    maddaz.xyz      acc,        VF16xyz,  VF18z     nop                                                     ; xform xyz0    ;
    madday.xyz      acc,        VF15xyz,  VF18y     nop                                                     ; xform xyz0    ;
    maddx.xyz       VF19xyz,  VF14xyz,  VF18x     nop                                                     ; xform xyz0    ;
    nop                                                 nop                                                     ;               ;
    nop                                                 lq.xyzw VF18xyzw, 11+(4*1)(VI11)     ;               ; load xyz1

    mulaw.xyzw      acc,        VF12xyzw, vf00w       lq.xyzw VF17xyzw, 3(VI13)                             ; l2s xyz0      ; load bone1
    maddaz.xyzw     acc,        VF11xyzw, VF19z     lq.xyzw VF16xyzw, 2(VI13)                             ; l2s xyz0      ; load bone1
    madday.xyzw     acc,        VF10xyzw, VF19y     lq.xyzw VF15xyzw, 1(VI13)                             ; l2s xyz0      ; load bone1
    maddx.xyzw      VF20xyzw, VF09xyzw, VF19x     lq.xyzw VF14xyzw, 0(VI13)                             ; l2s xyz0      ; load bone1
    nop                                                 nop                                                     ;               ;
    nop                                                 nop                                                     ;               ;
    mulaw.xyz       acc,        VF17xyz,  vf00w       ilw.x   VI13,       10+(4*2)(VI11)   ; xform xyz1    ; load bptr2
    maddaz.xyz      acc,        VF16xyz,  VF18z     div     q,  vf00w,  VF20w                             ; xform xyz1    ; divide0
    madday.xyz      acc,        VF15xyz,  VF18y     nop                                                     ; xform xyz1    ;
    maddx.xyz       VF19xyz,  VF14xyz,  VF18x     nop                                                     ; xform xyz1    ;
    nop                                                 nop                                                     ;               ;
    add.xyz         VF21xyz,  VF20xyz,  vf00xyz     lq.xyzw VF18xyzw, 11+(4*2)(VI11)     ; copy xyz0     ; load xyz2

    mulaw.xyzw      acc,        VF12xyzw, vf00w       lq.xyzw VF17xyzw, 3(VI13)                             ; l2s xyz1      ; load bone2
    maddaz.xyzw     acc,        VF11xyzw, VF19z     lq.xyzw VF16xyzw, 2(VI13)                             ; l2s xyz1      ; load bone2
    madday.xyzw     acc,        VF10xyzw, VF19y     lq.xyzw VF15xyzw, 1(VI13)                             ; l2s xyz1      ; load bone2
    maddx.xyzw      VF20xyzw, VF09xyzw, VF19x     lq.xyzw VF14xyzw, 0(VI13)                             ; l2s xyz1      ; load bone2
    mulq.xyz        VF22xyz,  VF21xyz,  q           nop                                                     ; project xyz0  ;
    nop                                                 nop                                                     ;               ;
    mulaw.xyz       acc,        VF17xyz,  vf00w       ilw.x   VI13,       10+(4*3)(VI11)   ; xform xyz2    ; load bptr3
    maddaz.xyz      acc,        VF16xyz,  VF18z     div     q,  vf00w,  VF20w                             ; xform xyz2    ; divide1
    madday.xyz      acc,        VF15xyz,  VF18y     nop                                                     ; xform xyz2    ;
    maddx.xyz       VF19xyz,  VF14xyz,  VF18x     nop                                                     ; xform xyz2    ;
    ftoi4.xyz       VF23xyz,  VF22xyz               nop                                                     ; ftoi0         ;
    add.xyz         VF21xyz,  VF20xyz,  vf00xyz     lq.xyzw VF18xyzw, 11+(4*3)(VI11)     ; copy xyz1     ; load xyz3

    ; optimized loop
SKIN_CAST_1BONE_LOOP:
    mulaw.xyzw      acc,        VF12xyzw, vf00w       lq.xyzw VF17xyzw, 3(VI13)                             ; l2s xyz2      ; load bone3
    maddaz.xyzw     acc,        VF11xyzw, VF19z     lq.xyzw VF16xyzw, 2(VI13)                             ; l2s xyz2      ; load bone3
    madday.xyzw     acc,        VF10xyzw, VF19y     lq.xyzw VF15xyzw, 1(VI13)                             ; l2s xyz2      ; load bone3
    maddx.xyzw      VF20xyzw, VF09xyzw, VF19x     lq.xyzw VF14xyzw, 0(VI13)                             ; l2s xyz2      ; load bone3
    mulq.xyz        VF22xyz,  VF21xyz,  q           sq.xyz  VF23xyz,  11(VI11)                       ; project xyz1  ; store xyz0
    nop                                                 sq.xyzw VF13xyzw, 10(VI11)                       ;               ; store rgb0
    mulaw.xyz       acc,        VF17xyz,  vf00w       ilw.x   VI13,       10+(4*4)(VI11)   ; xform xyz3    ; load bptr4
    maddaz.xyz      acc,        VF16xyz,  VF18z     div     q,  vf00w,  VF20w                             ; xform xyz3    ; divide2
    madday.xyz      acc,        VF15xyz,  VF18y     iaddiu  VI11, VI11, 4                       ; xform xyz3    ; vptr++
    maddx.xyz       VF19xyz,  VF14xyz,  VF18x     nop                                                     ; xform xyz3    ; nptr++
    ftoi4.xyz       VF23xyz,  VF22xyz               ibne    VI11, VI10, SKIN_CAST_1BONE_LOOP                ; ftoi1         ; loop
    add.xyz         VF21xyz,  VF20xyz,  vf00xyz     lq.xyzw VF18xyzw, 11+(4*3)(VI11)     ; copy xyz2     ; load xyz4

    ; finished
    nop                                                 jr      VI15
    nop                                                 nop

















;==============================================================================
;  Shadow creation kick
;==============================================================================
VU1_SHADOW_CAST_KICK:
    nop                                                 iaddiu      VI10, vi00, 0x7FFF                      ; Set EOP Bit
    nop                                                 iaddiu      VI10, VI10, 1
    nop                                                 ior         VI12, VI10, VI02                        ; EOP | Num Vertices
    nop                                                 iaddiu      VI11, vi00, 2
    nop                                                 lq          VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )(vi00)       ; Load 1 GS Register
    nop                                                 iaddiu      VI01, vi00, 0x3F
    nop                                                 mfir.z      VF10z,    VI01
    ;                                                   lq          TF02, blah -- TF02 was already set to the proper frame register
    nop                                                 lq          VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 1 )(vi00)        ; Strip: Opaque/Alpha | Context1 | UV0
    nop                                                 mfir.x      VF12x,    VI12                        ; Set EOP and Num Vertices in GIFTAG
    nop                                                 mfir.x      VF09x,    VI11                        ; Set 2 registers
    nop                                                 iaddiu      VI13, VI05, 4         ; Get start address of GIFTAG
    nop                                                 sq          VF09xyzw, 0(VI13)                     ; Build GS chain
    nop                                                 sq          VF10xyzw, 1(VI13)
    nop                                                 sq          VF11xyzw, 2(VI13)
    nop                                                 sq          VF12xyzw, 3(VI13)
    nop                                                 nop
    nop                                                 nop
    nop                                                 jr          VI15
    nop                                                 xgkick      VI13                                    ; Kick GS chain

VU1_SHADCAST_XFORM_CODE_END:
