#line 1 ".\\vu1\\mcode\\skinning.vu"












#line 1 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"





























































                                    













                                    



































































































#line 177 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"











#line 189 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































#line 314 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































































































































































































































































































































































































#line 791 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"

#line 14 ".\\vu1\\mcode\\skinning.vu"

.vu 
.org    0x3000
.align  4 
.global VU1_SKIN_XFORM_CODE_START
.global VU1_SKIN_XFORM_CODE_END

.global     VU1_ENTRY_SKIN_XFORM_1BONE
.global     VU1_ENTRY_SKIN_XFORM_2BONES
.global     VU1_ENTRY_SKIN_XFORM_CLIPPED
.global     VU1_ENTRY_SKIN_SETUP_MATRIX
.equ        VU1_ENTRY_SKIN_XFORM_1BONE,       ((VU1_SKIN_1BONE     - VU1_SKIN_XFORM_CODE_START + 0x3000)/8)
.equ        VU1_ENTRY_SKIN_XFORM_2BONES,      ((VU1_SKIN_2BONES    - VU1_SKIN_XFORM_CODE_START + 0x3000)/8)
.equ        VU1_ENTRY_SKIN_XFORM_CLIPPED,     ((VU1_SKIN_WCULL     - VU1_SKIN_XFORM_CODE_START + 0x3000)/8)
.equ        VU1_ENTRY_SKIN_SETUP_MATRIX,      ((VU1_SKIN_MAT_SETUP - VU1_SKIN_XFORM_CODE_START + 0x3000)/8)




VU1_SKIN_XFORM_CODE_START:

;==============================================================================
;
; Matrix setup - loads a transformation matrix, pre-translates it for our
; funky itof tricks, and stores it back out.
;
;==============================================================================
VU1_SKIN_MAT_SETUP:
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 0
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 4
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 8
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 12
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 16
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 20
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 24
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 28
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 32
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 36
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 40
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 44
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 48
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 52
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 56
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 60
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 64
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 68
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 72
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 76
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 80
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 84
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 88
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 92
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 96
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 100
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 104
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 108
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 112
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 116
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 120
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 124
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 128
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 132
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 136
    nop                                                 b       SETUP_MATRIX
    nop                                                 iaddiu  VI01, vi00, 140
SETUP_MATRIX:

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

;==============================================================================
;
;   Skin Renderer - No Clipping and two bones
;                   Does both lighting and skinning in one loop
;
;==============================================================================
























































VU1_SKIN_2BONES:
    nop                                                 xtop VI05
    nop                                                 lq.xyzw     VF08xyzw, 1+3(VI05)
    nop                                                 lq.xyzw     VF07xyzw, 1+2(VI05)
    nop                                                 lq.xyzw     VF06xyzw, 1+1(VI05)
    nop                                                 lq.xyzw     VF05xyzw, 1+0(VI05)
    mulaw.xyzw  acc,        VF04xyzw, VF08w         lq.zw       VF29zw,   9+16*0(VI05)
    maddaz.xyzw acc,        VF03xyzw, VF08z         lq.zw       VF30zw,   9+16*1(VI05)
    madday.xyzw acc,        VF02xyzw, VF08y         lq.zw       VF31zw,   9+16*2(VI05)
    maddx.xyzw  VF12xyzw, VF01xyzw, VF08x         lq.zw       VF13zw,   9+4*2+16*0(VI05)
    mulaw.xyzw  acc,        VF04xyzw, VF07w         mr32.xyzw   VF29xyzw, VF29xyzw
    maddaz.xyzw acc,        VF03xyzw, VF07z         mr32.xyzw   VF30xyzw, VF30xyzw
    madday.xyzw acc,        VF02xyzw, VF07y         mr32.xyzw   VF31xyzw, VF31xyzw
    maddx.xyzw  VF11xyzw, VF01xyzw, VF07x         lq.zw       VF14zw,   9+4*2+16*1(VI05)
    mulaw.xyzw  acc,        VF04xyzw, VF06w         mr32.xyzw   VF29xyzw, VF29xyzw
    maddaz.xyzw acc,        VF03xyzw, VF06z         mr32.xyzw   VF30xyzw, VF30xyzw
    madday.xyzw acc,        VF02xyzw, VF06y         mr32.xyzw   VF31xyzw, VF31xyzw
    maddx.xyzw  VF10xyzw, VF01xyzw, VF06x         lq.zw       VF15zw,   9+4*2+16*2(VI05)
    mulaw.xyzw  acc,        VF04xyzw, VF05w         lq.zw       VF29zw,   9+4+16*0(VI05)
    maddaz.xyzw acc,        VF03xyzw, VF05z         lq.zw       VF30zw,   9+4+16*1(VI05)
    madday.xyzw acc,        VF02xyzw, VF05y         lq.zw       VF31zw,   9+4+16*2(VI05)
    maddx.xyzw  VF09xyzw, VF01xyzw, VF05x         lq.zw       VF16zw,   9+4*2+16*3(VI05)
    nop                                                 mr32.xyzw   VF13xyzw, VF13xyzw
    nop                                                 mr32.xyzw   VF14xyzw, VF14xyzw
    mulaz.xyz   acc,        VF31xyz,  VF05z         mr32.xyzw   VF15xyzw, VF15xyzw
    madday.xyz  acc,        VF30xyz,  VF05y         mr32.xyzw   VF16xyzw, VF16xyzw
    maddx.xyz   VF17xyz,  VF29xyz,  VF05x         mr32.xyzw   VF13xyzw, VF13xyzw
    mulaz.xyz   acc,        VF31xyz,  VF06z         mr32.xyzw   VF14xyzw, VF14xyzw
    madday.xyz  acc,        VF30xyz,  VF06y         mr32.xyzw   VF15xyzw, VF15xyzw
    maddx.xyz   VF18xyz,  VF29xyz,  VF06x         mr32.xyzw   VF16xyzw, VF16xyzw
    mulaz.xyz   acc,        VF31xyz,  VF07z         lq.zw       VF13zw,   9+4*3+16*0(VI05)
    madday.xyz  acc,        VF30xyz,  VF07y         lq.zw       VF14zw,   9+4*3+16*1(VI05)
    maddx.xyz   VF19xyz,  VF29xyz,  VF07x         lq.zw       VF15zw,   9+4*3+16*2(VI05)
    nop                                                 lq.zw       VF16zw,   9+4*3+16*3(VI05)

    nop                                                 ilw.z       VI04,   0(VI05)
    nop                                                 iaddiu      VI08,   VI05,   0x00
    nop                                                 iaddiu      VI10,   vi00,   ( ( ( 0 + 144 ) + (8 + (68*4)) ) + (8 + (68*4)) )
    nop                                                 iaddiu      VI01,   vi00,   0x7f
    nop                                                 iand        VI02,   VI04,   VI01
    nop                                                 iadd        VI09,   VI08,   VI02
    nop                                                 iadd        VI09,   VI09,   VI02
    nop                                                 iadd        VI09,   VI09,   VI02
    nop                                                 iadd        VI09,   VI09,   VI02
    nop                                                 iaddiu      VI06,   vi00,   ( ( ( 0 + 144 ) + (8 + (68*4)) ) + (8 + (68*4)) )

    ; loop preamble
    nop                                                 lq.xyz  VF29xyz,  8(VI08)                ;                   ; load normal0
    nop                                                 lq.xyzw VF28xyzw, 10(VI08)                 ;                   ; load weights0
    nop                                                 nop                                                 ;                   ;
    nop                                                 nop                                                 ;                   ;
    nop                                                 nop                                                 ;                   ;
    itof12.xyz  VF30xyz,  VF29xyz                   mtir    VI11, VF28x                               ; normal0->float    ; pBone0_0
    nop                                                 nop                                                 ;                   ;
    nop                                                 mtir    VI12,VF28y                                ;                   ; pBone0_1
    itof12.zw   VF28zw,   VF28zw                    nop                                                 ; weights0->float   ;
    nop                                                 lq.xyzw VF29xyzw, 11(VI08)                   ;                   ; load xyz0
    nop                                                 nop                                                 ;                   ;
    nop                                                 loi     16.062745098039215686 ; 16*256/255                                ;                   ; load scale const
    muli.zw     VF28zw,   VF28zw,   i               nop                                                 ; scale weights0    ;
    nop                                                 nop                                                 ;                   ;
    nop                                                 nop                                                 ;                   ;
    nop                                                 lq.xyzw VF23xyzw, 3(VI11)                         ;                   ; load bone0_03
    mulw.xyz    VF31xyz,  VF29xyz,  VF28w         lq.xyzw VF22xyzw, 2(VI11)                         ; weight xyz0_1     ; load bone0_02
    nop                                                 lq.xyzw VF21xyzw, 1(VI11)                         ;                   ; load bone0_01
    nop                                                 lq.xyzw VF20xyzw, 0(VI11)                         ;                   ; load bone0_00
    mulz.xyz    VF29xyz,  VF29xyz,  VF28z         lq.xyzw VF27xyzw, 3(VI12)                         ; weight xyz0_0     ; load bone0_13
    mulaz.xyz   acc,        VF22xyz,  VF30z         nop                                                 ; skin normal0      ;
    madday.xyz  acc,        VF21xyz,  VF30y         nop                                                 ; skin normal0      ;
    maddx.xyz   VF30xyz,  VF20xyz,  VF30x         nop                                                 ; skin normal0      ;
    mulaz.xyz   acc,        VF23xyz,  VF28z         nop                                                 ; skin xyz0         ;

    ; optimized loop
SKIN_LP_2_BONES:
    maddaz.xyz  acc,        VF22xyz,  VF29z         lq.xyzw VF26xyzw, 2(VI12)                         ; skin xyz0         ; load bone0_12
    madday.xyz  acc,        VF21xyz,  VF29y         lq.xyzw VF25xyzw, 1(VI12)                         ; skin xyz0         ; load bone0_12
    maddax.xyz  acc,        VF20xyz,  VF29x         lq.xyzw VF24xyzw, 0(VI12)                         ; skin xyz0         ; load bone0_12
    maddaw.xyz  acc,        VF27xyz,  VF28w         nop                                                 ; skin xyz0         ;
    maddaz.xyz  acc,        VF26xyz,  VF31z         nop                                                 ; skin xyz0         ;
    madday.xyz  acc,        VF25xyz,  VF31y         nop                                                 ; skin xyz0         ;
    maddx.xyz   VF31xyz,  VF24xyz,  VF31x         move.zw VF26zw,   vf00zw                          ; skin xyz0         ; itex.w = 1
    mulaz.xyz   acc,        VF19xyz,  VF30z         iaddiu  VI08, VI08, 4                   ; calc dot0         ; vptr++
    madday.xyz  acc,        VF18xyz,  VF30y         iaddiu  VI10, VI10, 4                   ; calc dot0         ; bptr++
    maddx.xyz   VF25xyz,  VF17xyz,  VF30x         sq.w    VF29w,    11-4(VI10)     ; calc dot0         ; backup adc0
    mulaw.xyzw  acc,        VF12xyzw, vf00w           sq.xyz  VF31xyz,  11-4(VI10)     ; calc xyz0         ; backup pos0
    maddaz.xyzw acc,        VF11xyzw, VF31z         lq.xyz  VF29xyz,  8(VI08)                ; calc xyz0         ; load normal1
    madday.xyzw acc,        VF10xyzw, VF31y         lq.xyzw VF28xyzw, 10(VI08)                 ; calc xyz0         ; load weights1
    maddx.xyzw  VF31xyzw, VF09xyzw, VF31x         lq.xy   VF26xy,   9-4(VI08)     ; calc xyz0         ; load uv0
    maxx.xyz    VF25xyz,  VF25xyz,  vf00x           mr32.zw VF26zw,   VF26zw                        ; clamp dot0        ; itex.z = 1
    ftoi12.xyz  VF24xyz,  VF30xyz                   iaddiu  VI01,   vi00,   0x80                        ; normal0->fixed    ; tmp = 0x80
    itof12.xyz  VF30xyz,  VF29xyz                   mtir    VI11, VF28x                               ; normal1->float    ; pBone1_0
    mulaw.xyz   acc,        VF16xyz,  vf00w           div     q, vf00w, VF31w                           ; calc rgb0         ; divide0
    maddaz.xyz  acc,        VF15xyz,  VF25z         mtir    VI12, VF28y                               ; calc rgb0         ; pBone1_1
    itof12.zw   VF28zw,   VF28zw                    sq.xyz  VF24xyz,  8-4(VI08)  ; weights1->float   ; store normal0
    madday.xyz  acc,        VF14xyz,  VF25y         lq.xyzw VF29xyzw, 11(VI08)                   ; calc rgb0         ; load xyz1
    maddx.xyz   VF25xyz,  VF13xyz,  VF25x         sq.xyz  VF26xyz,  9-4(VI10)     ; calc rgb0         ; backup uv0
    itof12.xy   VF26xy,   VF26xy                    loi     16.062745098039215686 ; 16*256/255                                ; uv0->fixed        ; load scale const
    muli.zw     VF28zw,   VF28zw,   i               mfir.w  VF25w, VI01                               ; scale weights1    ; alpha = 0x80
    mulq.xyz    VF24xyz,  VF31xyz,  q               loi     255.0                                 ; project xyz0      ; load clamp const
    minii.xyz   VF25xyz,  VF25xyz,  i               nop                                                 ; clamp rgb0        ;
    mulq.xyz    VF26xyz,  VF26xyz,  q               lq.xyzw VF23xyzw, 3(VI11)                         ; project uv0       ; load bone1_03
    mulw.xyz    VF31xyz,  VF29xyz,  VF28w         lq.xyzw VF22xyzw, 2(VI11)                         ; weight xyz1_1     ; load bone1_02
    ftoi4.xyz   VF24xyz,  VF24xyz                   lq.xyzw VF21xyzw, 1(VI11)                         ; xyz0->fixed       ; load bone1_01
    ftoi0.xyz   VF25xyz,  VF25xyz                   lq.xyzw VF20xyzw, 0(VI11)                         ; rgb0->fixed       ; load bone1_00
    mulz.xyz    VF29xyz,  VF29xyz,  VF28z         lq.xyzw VF27xyzw, 3(VI12)                         ; weight xyz1_0     ; load bone1_13
    mulaz.xyz   acc,        VF22xyz,  VF30z         sq.xyz  VF26xyz,  9-4(VI08)     ; skin normal1      ; store uv0
    madday.xyz  acc,        VF21xyz,  VF30y         sq.xyz  VF24xyz,  11-4(VI08)     ; skin normal1      ; store xyz0
    maddx.xyz   VF30xyz,  VF20xyz,  VF30x         ibne    VI08, VI09, SKIN_LP_2_BONES                 ; skin normal1      ; loop
    mulaz.xyz   acc,        VF23xyz,  VF28z         sq.xyzw VF25xyzw, 10-4(VI08)     ; skin xyz1         ; store rgb0

    nop                                                 iaddiu      VI01, VI00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )
    nop                                                 xgkick      VI01
    nop[e]                                              nop
    nop                                                 nop

    nop                                                 b           VU1_SKIN_XFORM_CODE_START+0x0FC0-0x3000+16384
    nop                                                 iaddiu      VI06, VI00, ( ( ( 0 + 144 ) + (8 + (68*4)) ) + (8 + (68*4)) )
























































;==============================================================================
;
;   Skin Renderer - No Clipping and one bone
;                   Does both lighting and skinning in one loop
;
;==============================================================================













































VU1_SKIN_1BONE:
    nop                                                 xtop VI05
    nop                                                 lq.xyzw     VF08xyzw, 1+3(VI05)
    nop                                                 lq.xyzw     VF07xyzw, 1+2(VI05)
    nop                                                 lq.xyzw     VF06xyzw, 1+1(VI05)
    nop                                                 lq.xyzw     VF05xyzw, 1+0(VI05)
    mulaw.xyzw  acc,        VF04xyzw, VF08w         lq.zw       VF29zw,   9+16*0(VI05)
    maddaz.xyzw acc,        VF03xyzw, VF08z         lq.zw       VF30zw,   9+16*1(VI05)
    madday.xyzw acc,        VF02xyzw, VF08y         lq.zw       VF31zw,   9+16*2(VI05)
    maddx.xyzw  VF12xyzw, VF01xyzw, VF08x         lq.zw       VF13zw,   9+4*2+16*0(VI05)
    mulaw.xyzw  acc,        VF04xyzw, VF07w         mr32.xyzw   VF29xyzw, VF29xyzw
    maddaz.xyzw acc,        VF03xyzw, VF07z         mr32.xyzw   VF30xyzw, VF30xyzw
    madday.xyzw acc,        VF02xyzw, VF07y         mr32.xyzw   VF31xyzw, VF31xyzw
    maddx.xyzw  VF11xyzw, VF01xyzw, VF07x         lq.zw       VF14zw,   9+4*2+16*1(VI05)
    mulaw.xyzw  acc,        VF04xyzw, VF06w         mr32.xyzw   VF29xyzw, VF29xyzw
    maddaz.xyzw acc,        VF03xyzw, VF06z         mr32.xyzw   VF30xyzw, VF30xyzw
    madday.xyzw acc,        VF02xyzw, VF06y         mr32.xyzw   VF31xyzw, VF31xyzw
    maddx.xyzw  VF10xyzw, VF01xyzw, VF06x         lq.zw       VF15zw,   9+4*2+16*2(VI05)
    mulaw.xyzw  acc,        VF04xyzw, VF05w         lq.zw       VF29zw,   9+4+16*0(VI05)
    maddaz.xyzw acc,        VF03xyzw, VF05z         lq.zw       VF30zw,   9+4+16*1(VI05)
    madday.xyzw acc,        VF02xyzw, VF05y         lq.zw       VF31zw,   9+4+16*2(VI05)
    maddx.xyzw  VF09xyzw, VF01xyzw, VF05x         lq.zw       VF16zw,   9+4*2+16*3(VI05)
    nop                                                 mr32.xyzw   VF13xyzw, VF13xyzw
    nop                                                 mr32.xyzw   VF14xyzw, VF14xyzw
    mulaz.xyz   acc,        VF31xyz,  VF05z         mr32.xyzw   VF15xyzw, VF15xyzw
    madday.xyz  acc,        VF30xyz,  VF05y         mr32.xyzw   VF16xyzw, VF16xyzw
    maddx.xyz   VF17xyz,  VF29xyz,  VF05x         mr32.xyzw   VF13xyzw, VF13xyzw
    mulaz.xyz   acc,        VF31xyz,  VF06z         mr32.xyzw   VF14xyzw, VF14xyzw
    madday.xyz  acc,        VF30xyz,  VF06y         mr32.xyzw   VF15xyzw, VF15xyzw
    maddx.xyz   VF18xyz,  VF29xyz,  VF06x         mr32.xyzw   VF16xyzw, VF16xyzw
    mulaz.xyz   acc,        VF31xyz,  VF07z         lq.zw       VF13zw,   9+4*3+16*0(VI05)
    madday.xyz  acc,        VF30xyz,  VF07y         lq.zw       VF14zw,   9+4*3+16*1(VI05)
    maddx.xyz   VF19xyz,  VF29xyz,  VF07x         lq.zw       VF15zw,   9+4*3+16*2(VI05)
    nop                                                 lq.zw       VF16zw,   9+4*3+16*3(VI05)

    nop                                                 ilw.z       VI04,   0(VI05)
    nop                                                 iaddiu      VI08,   VI05,   0x00
    nop                                                 iaddiu      VI10,   vi00,   ( ( ( 0 + 144 ) + (8 + (68*4)) ) + (8 + (68*4)) )
    nop                                                 iaddiu      VI01,   vi00,   0x7f
    nop                                                 iand        VI02,   VI04,   VI01
    nop                                                 iadd        VI09,   VI08,   VI02
    nop                                                 iadd        VI09,   VI09,   VI02
    nop                                                 iadd        VI09,   VI09,   VI02
    nop                                                 iadd        VI09,   VI09,   VI02
    nop                                                 iaddiu      VI06,   vi00,   ( ( ( 0 + 144 ) + (8 + (68*4)) ) + (8 + (68*4)) )

    ; loop preamble
    addw.xyzw   VF30xyzw, vf00,       vf00w           ilw.x   VI11,       10(VI08)                 ;                   ; load bi0
    nop                                                 lq.xyz  VF26xyz,  8(VI08)                ;                   ; load normal0
    nop                                                 loi     255.0                                 ;                   ; load the color clamp
    nop                                                 nop                                                 ;                   ;
    nop                                                 lq.xyzw VF23xyzw, 3(VI11)                         ;                   ; load bone0_3
    itof12.xyz  VF26xyz,  VF26xyz                   lq.xyzw VF22xyzw, 2(VI11)                         ; normal0->float    ; load bone0_2
    nop                                                 lq.xyzw VF21xyzw, 1(VI11)                         ;                   ; load bone0_1
    nop                                                 lq.xyzw VF20xyzw, 0(VI11)                         ;                   ; load bone0_0
    nop                                                 lq.xyzw VF24xyzw, 11(VI08)                   ;                   ; load pos0
    mulaz.xyz   acc,        VF22xyz,  VF26z         iaddiu  VI01,   vi00,   0x80                        ; skin normal0      ; tmp = 128
    madday.xyz  acc,        VF21xyz,  VF26y         mfir.w  VF29w, VI01                                ; skin normal0      ; alpha = 128
    maddx.xyz   VF27xyz,  VF20xyz,  VF26x         nop                                                 ; skin normal0      ;
    mulaw.xyz   acc,        VF23xyz,  vf00w           nop                                                 ; skin pos0         ;
    maddaz.xyz  acc,        VF22xyz,  VF24z         nop                                                 ; skin pos0         ;
    madday.xyz  acc,        VF21xyz,  VF24y         lq.xyz  VF26xyz,  8+4(VI08)  ; skin pos0         ; load normal1
    maddx.xyz   VF24xyz,  VF20xyz,  VF24x         ilw.x   VI11,       10+4(VI08)   ; skin pos0         ; load bi1
    mulaz.xyz   acc,        VF19xyz,  VF27z         nop                                                 ; calc dot0         ;
    madday.xyz  acc,        VF18xyz,  VF27y         nop                                                 ; calc dot0         ;
    maddx.xyz   VF28xyz,  VF17xyz,  VF27x         nop                                                 ; calc dot0         ;

    ; optimized loop
SKIN_LP_1BONE:
    mulaw.xyzw  acc,        VF12xyzw, vf00w           lq.xy   VF30xy,   9(VI08)                   ; xform pos0        ; load uv0
    maddaz.xyzw acc,        VF11xyzw, VF24z         sq.w    VF24w,    11(VI10)                   ; xform pos0        ; backup adc0
    madday.xyzw acc,        VF10xyzw, VF24y         sq.xyz  VF24xyz,  11(VI10)                   ; xform pos0        ; backup pos0
    maddx.xyzw  VF25xyzw, VF09xyzw, VF24x         lq.xyzw VF23xyzw, 3(VI11)                         ; xform pos0        ; load bone1_3
    maxx.xyz    VF29xyz,  VF28xyz,  vf00x           lq.xyzw VF22xyzw, 2(VI11)                         ; clamp dot0        ; load bone1_2
    ftoi12.xyz  VF27xyz,  VF27xyz                   lq.xyzw VF21xyzw, 1(VI11)                         ; normal0->fixed    ; load bone1_1
    itof12.xy   VF31xy,   VF30xy                    lq.xyzw VF20xyzw, 0(VI11)                         ; uv0->float        ; load bone1_0
    itof12.xyz  VF26xyz,  VF26xyz                   div     q,  vf00w,  VF25w                         ; normal1->float    ; divide0
    mulaw.xyz   acc,        VF16xyz,  vf00w           move.zw VF31zw,   VF30zw                        ; calc rgb0         ; ftex.z = 1
    maddaz.xyz  acc,        VF15xyz,  VF29z         nop                                                 ; calc rgb0         ;
    madday.xyz  acc,        VF14xyz,  VF29y         iaddiu  VI08, VI08, 4                   ; calc rgb0         ; vptr++
    maddx.xyz   VF29xyz,  VF13xyz,  VF29x         iaddiu  VI10, VI10, 4                   ; calc rgb0         ; bptr++
    mulaz.xyz   acc,        VF22xyz,  VF26z         nop                                                 ; skin normal1      ;
    madday.xyz  acc,        VF21xyz,  VF26y         sq.xyz  VF27xyz,  8-4(VI08)  ; skin normal1      ; store normal0
    mulq.xyz    VF25xyz,  VF25xyz,  q               sq.xyz  VF30xyz,  9-4(VI10)     ; project pos0      ; backup uv0
    maddx.xyz   VF27xyz,  VF20xyz,  VF26x         lq.xyzw VF24xyzw, 11(VI08)                   ; skin normal1      ; load pos1
    mulq.xyz    VF31xyz,  VF31xyz,  q               nop                                                 ; project uv0       ;
    minii.xyz   VF29xyz,  VF29xyz,  i               nop                                                 ; clamp rgb0        ;
    ftoi4.xyz   VF25xyz,  VF25xyz                   nop                                                 ; pos0->fixed       ;
    mulaw.xyz   acc,        VF23xyz,  vf00w           nop                                                 ; skin pos1         ;
    maddaz.xyz  acc,        VF22xyz,  VF24z         nop                                                 ; skin pos1         ;
    madday.xyz  acc,        VF21xyz,  VF24y         lq.xyz  VF26xyz,  8+4(VI08)  ; skin pos1         ; load normal2
    ftoi0.xyz   VF29xyz,  VF29xyz                   ilw.x   VI11,       10+4(VI08)   ; rgb0->int         ; load bi2
    maddx.xyz   VF24xyz,  VF20xyz,  VF24x         sq.xyz  VF31xyz,  9-4(VI08)     ; skin pos1         ; store uv0
    mulaz.xyz   acc,        VF19xyz,  VF27z         sq.xyz  VF25xyz,  11-4(VI08)     ; calc dot1         ; store pos0
    madday.xyz  acc,        VF18xyz,  VF27y         ibne    VI08, VI09, SKIN_LP_1BONE                   ; calc dot1         ; loop
    maddx.xyz   VF28xyz,  VF17xyz,  VF27x         sq.xyzw VF29xyzw, 10-4(VI08)     ; calc dot1         ; store rgb0

    nop                                                 iaddiu      VI01, VI00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )
    nop                                                 xgkick      VI01
    nop[e]                                              nop
    nop                                                 nop

    nop                                                 b           VU1_SKIN_XFORM_CODE_START+0x0FC0-0x3000+16384
    nop                                                 iaddiu      VI06, VI00, ( ( ( 0 + 144 ) + (8 + (68*4)) ) + (8 + (68*4)) )













































;==============================================================================
;
;   Slow Renderer for skinning - Does in-place skinning to local space, then
;   calls the normal slow renderer code.
;
;==============================================================================






















































































































































































































;==============================================================================
;
;   Slow Renderer for skinning - Does in-place skinning to local space, and
;   culls triangles (it does not bother calling the clipper, though!)
;
;==============================================================================




































































VU1_SKIN_WCULL:
    nop                                                 xtop        VI05
    nop                                                 lq.zw       VF26zw,   9+16*2(VI05)
    nop                                                 lq.zw       VF25zw,   9+16*1(VI05)
    nop                                                 lq.zw       VF24zw,   9+16*0(VI05)
    nop                                                 lq.xyzw     VF08xyzw, 1+3(VI05)
    nop                                                 mr32.xyzw   VF26xyzw, VF26xyzw
    nop                                                 mr32.xyzw   VF25xyzw, VF25xyzw
    nop                                                 mr32.xyzw   VF24xyzw, VF24xyzw
    nop                                                 lq.xyzw     VF07xyzw, 1+2(VI05)
    nop                                                 mr32.xyzw   VF26xyzw, VF26xyzw
    nop                                                 mr32.xyzw   VF25xyzw, VF25xyzw
    nop                                                 mr32.xyzw   VF24xyzw, VF24xyzw
    nop                                                 lq.xyzw     VF06xyzw, 1+1(VI05)
    nop                                                 lq.zw       VF26zw,   9+4+16*2(VI05)
    nop                                                 lq.zw       VF25zw,   9+4+16*1(VI05)
    nop                                                 lq.zw       VF24zw,   9+4+16*0(VI05)
    nop                                                 lq.xyzw     VF05xyzw, 1+0(VI05)
    mulaz.xyz   acc,        VF26xyz,  VF07z         lq.zw       VF15zw,   9+4*2+16*3(VI05)
    madday.xyz  acc,        VF25xyz,  VF07y         lq.zw       VF14zw,   9+4*2+16*2(VI05)
    maddx.xyz   VF11xyz,  VF24xyz,  VF07x         lq.zw       VF13zw,   9+4*2+16*1(VI05)
    mulaz.xyz   acc,        VF26xyz,  VF06z         lq.zw       VF12zw,   9+4*2+16*0(VI05)
    madday.xyz  acc,        VF25xyz,  VF06y         mr32.xyzw   VF15xyzw, VF15xyzw
    maddx.xyz   VF10xyz,  VF24xyz,  VF06x         mr32.xyzw   VF14xyzw, VF14xyzw
    mulaz.xyz   acc,        VF26xyz,  VF05z         mr32.xyzw   VF13xyzw, VF13xyzw
    madday.xyz  acc,        VF25xyz,  VF05y         mr32.xyzw   VF12xyzw, VF12xyzw
    maddx.xyz   VF09xyz,  VF24xyz,  VF05x         mr32.xyzw   VF15xyzw, VF15xyzw
    mulaw.xyzw  acc,        VF04xyzw, VF08w         mr32.xyzw   VF14xyzw, VF14xyzw
    maddaz.xyzw acc,        VF03xyzw, VF08z         mr32.xyzw   VF13xyzw, VF13xyzw
    madday.xyzw acc,        VF02xyzw, VF08y         mr32.xyzw   VF12xyzw, VF12xyzw
    maddx.xyzw  VF08xyzw, VF01xyzw, VF08x         lq.zw       VF15zw,   9+4*3+16*3(VI05)
    mulaw.xyzw  acc,        VF04xyzw, VF07w         lq.zw       VF14zw,   9+4*3+16*2(VI05)
    maddaz.xyzw acc,        VF03xyzw, VF07z         lq.zw       VF13zw,   9+4*3+16*1(VI05)
    madday.xyzw acc,        VF02xyzw, VF07y         lq.zw       VF12zw,   9+4*3+16*0(VI05)
    maddx.xyzw  VF07xyzw, VF01xyzw, VF07x         ilw.z       VI04,       0(VI05)
    mulaw.xyzw  acc,        VF04xyzw, VF06w         iaddiu      VI08, VI05, 0x00
    maddaz.xyzw acc,        VF03xyzw, VF06z         iaddiu      VI10, vi00, ( ( ( 0 + 144 ) + (8 + (68*4)) ) + (8 + (68*4)) )
    madday.xyzw acc,        VF02xyzw, VF06y         iaddiu      VI01, vi00, 0x7f
    maddx.xyzw  VF06xyzw, VF01xyzw, VF06x         iand        VI02, VI04, VI01
    mulaw.xyzw  acc,        VF04xyzw, VF05w         iadd        VI09, VI08, VI02
    maddaz.xyzw acc,        VF03xyzw, VF05z         iadd        VI09, VI09, VI02
    madday.xyzw acc,        VF02xyzw, VF05y         iadd        VI09, VI09, VI02
    maddx.xyzw  VF05xyzw, VF01xyzw, VF05x         iadd        VI09, VI09, VI02
    nop                                                 loi         0.0
    muli.w      VF22w,    vf00w,      i               loi         1048575.0 ; 2^(24-bit zbuffer minus 4 for ftoi conversion) minus 1 for good measure
    muli.w      VF28w,    vf00w,      i               loi         10.0
    addi.xy     VF22xy,   vf00xy,     i               loi         4085.0
    addi.xy     VF28xy,   vf00xy,     i               iaddiu      VI06, vi00, ( ( ( 0 + 144 ) + (8 + (68*4)) ) + (8 + (68*4)) )
    addw.z      VF22z,    vf00z,      VF22w         iaddiu      VI01, vi00, 0x0080
    addw.z      VF28z,    vf00z,      VF28w         mfir.w      VF23w,    VI01
    nop                                                 iaddiu      VI14, vi00, 0x00e0
    nop                                                 sq.xyzw     VF22xyzw, 1+4(VI05)
    nop                                                 sq.xyzw     VF28xyzw, 1+5(VI05)
    nop                                                 iaddiu      VI13, vi00, 0x7fff
    nop                                                 iaddiu      VI13, VI13, 0x0001
    
    ; loop preamble
    nop                                                 lq.xyzw     VF16xyzw, 10(VI08)                 ;                   ; load weights 0
    nop                                                 nop                                                     ;                   ;
    nop                                                 loi         16.062745098039215686 ; 16*256/255                                ;                   ; prep wgt scl 0
    nop                                                 lq.xyz      VF21xyz,  8(VI08)                ;                   ; load normal 0
    itof12.zw   VF16zw,   VF16zw                    mtir        VI12,       VF16y                         ; wgts to float 0   ; pBoneB 0
    nop                                                 mtir        VI11,       VF16x                         ;                   ; pBoneA 0
    nop                                                 lq.xyzw     VF17xyzw, 11(VI08)                   ;                   ; load pos 0
    nop                                                 lq.xyzw     VF30xyzw, 3(VI12)                         ;                   ; load bone_b3 0
    itof12.xyz  VF21xyz,  VF21xyz                   lq.xyzw     VF27xyzw, 3(VI11)                         ; norm to float 0   ; load bone_a3 0
    muli.zw     VF16zw,   VF16zw,   i               loi         -786432.0                                  ; scale weights 0   ; prep itof 0
    addi.xyz    VF17xyz,  VF17xyz,  i               lq.xyzw     VF26xyzw, 2(VI11)                         ; pos to float 0    ; load bone_a2 0
    nop                                                 lq.xyzw     VF25xyzw, 1(VI11)                         ;                   ; load bone_a1 0
    nop                                                 lq.xyzw     VF24xyzw, 0(VI11)                         ;                   ; load bone_a0 0
    nop                                                 lq.xyzw     VF29xzyw, 2(VI12)                         ;                   ; load bone_b2 0
    mulz.xyz    VF18xyz,  VF17xyz,  VF16z         lq.xyzw     VF28xyzw, 1(VI12)                         ; scale pos a 0     ; load bone_b1 0
    mulw.xyz    VF17xyz,  VF17xyz,  VF16w         nop                                                     ; scale pos b 0     ;
    mulaz.xyz   acc,        VF26xyz,  VF21z         nop                                                     ; xform normal 0    ;
    madday.xyz  acc,        VF25xyz,  VF21y         nop                                                     ; xform normal 0    ;
    maddx.xyz   VF22xyz,  VF24xyz,  VF21x         nop                                                     ; xform normal 0    ;
    mulaw.xyz   acc,        VF30xyz,  VF16w         nop                                                     ; skin pos 0        ;
    maddaz.xyz  acc,        VF27xyz,  VF16z         nop                                                     ; skin pos 0        ;
    maddaz.xyz  acc,        VF26xyz,  VF18z         nop                                                     ; skin pos 0        ;

SKIN_WCULL_LOOP:
    madday.xyz  acc,        VF25xyz,  VF18y         lq.xyzw     VF21xyzw, 0(VI12)                         ; skin pos 0        ; load bone_b0 0
    maddax.xyz  acc,        VF24xyz,  VF18x         iaddiu      VI08, VI08, 4                   ; skin pos 0        ; vptr++
    maddaz.xyz  acc,        VF29xyz,  VF17z         iaddiu      VI10, VI10, 4                   ; skin pos 0        ; bptr++
    madday.xyz  acc,        VF28xyz,  VF17y         sq.w        VF17w,    11-4(VI10)     ; skin pos 0        ; backup adc 0
    maddx.xyz   VF17xyz,  VF21xyz,  VF17x         move.zw     VF20zw,   vf00zw                          ; skin pos 0        ; itex.w=1.0 0
    mulaz.xyz   acc,        VF11xyz,  VF22z         nop                                                     ; calc dot 0        ;
    madday.xyz  acc,        VF10xyz,  VF22y         lq.xy       VF20xy,   9-4(VI08)     ; calc dot 0        ; load uv 0
    maddx.xyz   VF23xyz,  VF09xyz,  VF22x         nop                                                     ; calc dot 0        ;
    mulaw.xyzw  acc,        VF08xyzw, vf00w           sq.xyz      VF17xyz,  11-4(VI10)     ; xform pos 0       ; backup pos 0
    maddaz.xyzw acc,        VF07xyzw, VF17z         lq.xyzw     VF16xyzw, 10(VI08)                 ; xform pos 0       ; load weights 1
    madday.xyzw acc,        VF06xyzw, VF17y         mr32.zw     VF20zw,   VF20zw                        ; xform pos 0       ; itex.z=1.0 0
    maddx.xyzw  VF19xyzw, VF05xyzw, VF17x         loi         16.062745098039215686 ; 16*256/255                                ; xform pos 0       ; prep wgt scl 1
    ftoi12.xyz  VF22xyz,  VF22xyz                   lq.xyz      VF21xyz,  8(VI08)                ; nrm to fixed 0    ; load normal 1
    itof12.zw   VF16zw,   VF16zw                    mtir        VI12,       VF16y                         ; wgts to float 1   ; pBoneB 1
    maxx.xyz    VF23xyz,  VF23xyz,  vf00x           mtir        VI11,       VF16x                         ; clamp dot 0       ; pBoneA 1
    mulaw.xyz   acc,        VF15xyz,  vf00w           div         q,   vf00w, VF19w                         ; calc color 0      ; divide 0
    itof12.xyz  VF21xyz,  VF21xyz                   lq.xyzw     VF17xyzw, 11(VI08)                   ; norm to float 1   ; load pos 1
    muli.zw     VF16zw,   VF16zw,   i               sq.xyz      VF20xyz,  9-4(VI10)     ; scale weights 1   ; backup uv 0
    maddaz.xyz  acc,        VF14xyz,  VF23z         sq.xyz      VF22xyz,  8-4(VI08)  ; calc color 0      ; store norm 0
    madday.xyz  acc,        VF13xyz,  VF23y         loi         -786432.0                                  ; calc color 0      ; prep itof 1
    addi.xyz    VF17xyz,  VF17xyz,  i               nop                                                     ; pos to float 1    ;
    maddx.xyz   VF23xyz,  VF12xyz,  VF23x         lq.xyzw     VF22xyzw, 1+4(VI05)          ; calc color 0      ; prep cmin test 0
    itof12.xy   VF20xy,   VF20xy                    lq.xyzw     VF28xyzw, 1+5(VI05)          ; uv to float 0     ; prep cmax test 0
    mulq.xyz    VF19xyz,  VF19xyz,  q               loi         255.0                                 ; project pos 0     ; prep cclamp 0
    mulz.xyz    VF18xyz,  VF17xyz,  VF16z         lq.xyzw     VF30xyzw, 3(VI12)                         ; scale pos a 1     ; load bone_b3 1
    minii.xyz   VF23xyz,  VF23xyz,  i               lq.xyzw     VF27xyzw, 3(VI11)                         ; clamp color 0     ; load bone_a3 1
    mulw.xyz    VF17xyz,  VF17xyz,  VF16w         lq.xyzw     VF26xyzw, 2(VI11)                         ; scale pos b 1     ; load bone_a2 1
    sub.xyz     vf00,       VF22xyz,  VF19xyz       lq.xyzw     VF25xyzw, 1(VI11)                         ; cmin test 0       ; load bone_a1 1
    sub.xyz     vf00,       VF19xyz,  VF28xyz       lq.xyzw     VF24xyzw, 0(VI11)                         ; cmax test 0       ; load bone_a0 1
    ftoi0.xyz   VF23xyz,  VF23xyz                   lq.xyzw     VF29xzyw, 2(VI12)                         ; color to int 0    ; load bone_b2 1
    mulaz.xyz   acc,        VF26xyz,  VF21z         lq.xyzw     VF28xyzw, 1(VI12)                         ; xform normal 1    ; load bone_b1 1
    ftoi4.xyz   VF19xyz,  VF19xyz                   fmand       VI01,       VI14                            ; pos to fixed 0    ; cmin test 0
    madday.xyz  acc,        VF25xyz,  VF21y         fmand       VI01,       VI01                            ; xform normal 1    ; cmax test 0
    mulq.xyz    VF20xyz,  VF20xyz,  q               sq.xyzw     VF23xyzw, 10-4(VI08)     ; project uv 0      ; store color 0
    maddx.xyz   VF22xyz,  VF24xyz,  VF21x         ibne        VI01, VI14, SKIN_CULL_TRI                   ; xform normal 1    ; cull test 0
    mulaw.xyz   acc,        VF30xyz,  VF16w         sq.xyz      VF19xyz,  11-4(VI08)     ; skin pos 1        ; store pos 0
SKIN_CULL_RET:
    maddaz.xyz  acc,        VF27xyz,  VF16z         ibne        VI08, VI09, SKIN_WCULL_LOOP                 ; skin pos 1        ; loop
    maddaz.xyz  acc,        VF26xyz,  VF18z         sq.xyz      VF20xyz,  9-4(VI08)     ; skin pos 1        ; store uv 0

    ; cleanup--note that we trashed the L2W registers because we needed all we could get for the loop
    ; so reload the l2w matrix now   
    nop                                                 lq.xyzw     VF08xyzw, 1+3(VI05)
    nop                                                 lq.xyzw     VF07xyzw, 1+2(VI05)
    nop                                                 lq.xyzw     VF06xyzw, 1+1(VI05)
    nop                                                 lq.xyzw     VF05xyzw, 1+0(VI05)

    nop                                                 iaddiu      VI01, VI00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )
    nop                                                 xgkick      VI01
    ;
    ;

    nop                                                 b           VU1_SKIN_XFORM_CODE_START+0x0FC0-0x3000+16384
    nop                                                 iaddiu      VI06, VI00, ( ( ( 0 + 144 ) + (8 + (68*4)) ) + (8 + (68*4)) )

SKIN_CULL_TRI:
    ; turn on the adc bits for the current vert and the next two verts, being careful not to overrun our buffer
    nop                                                 isw.w       VI13,       11-4(VI08)     ; store adc bit into pos 0
    nop                                                 isw.w       VI13,       11-4(VI10)     ; backup adc bit into pos 0
    nop                                                 ibeq        VI08, VI09, SKIN_CULL_RET                   ; bail out if we're on the last vert
    nop                                                 iaddiu      VI01, VI08, 4                   ; tmp = vptr + 1
    nop                                                 isw.w       VI13,       11(VI08)                   ; store adc bit into pos 1
    nop                                                 ibeq        VI01, VI09, SKIN_CULL_RET                   ; bail out if we're on the next-to-last vert
    nop                                                 mfir.w      VF17w,    VI13                            ; backup adc bit 1 (note IPOS was already loaded, so change the register, not memory!)
    nop                                                 isw.w       VI13,       11+4(VI08)     ; store adc bit into pos 2
    nop                                                 b           SKIN_CULL_RET                               ; return
    nop                                                 isw.w       VI13,       11+4(VI10)     ; backup adc bit into pos 2




































































VU1_SKIN_XFORM_CODE_END:
