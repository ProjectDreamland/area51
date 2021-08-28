#line 1 ".\\vu1\\mcode\\shadreceive_transform.vu"












#line 1 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"





























































                                    













                                    



































































































#line 177 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"











#line 189 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































#line 314 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































































































































































































































































































































































































#line 791 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"

#line 14 ".\\vu1\\mcode\\shadreceive_transform.vu"
#line 1 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\shadow_include.vu"

















;==============================================================================
;  Layout of shad_material Structure
;==============================================================================


































































#line 15 ".\\vu1\\mcode\\shadreceive_transform.vu"

.vu 
.org    0x3000
.align  4 
.global VU1_SHADRECEIVE_XFORM_CODE_START
.global VU1_SHADRECEIVE_XFORM_CODE_END

.global     VU1_ENTRY_SHAD_RECEIVE_FAST
.global     VU1_ENTRY_SHAD_RECEIVE_SLOW
.equ        VU1_ENTRY_SHAD_RECEIVE_FAST, ((VU1_SHADOW_RECEIVE_FAST-VU1_SHADRECEIVE_XFORM_CODE_START+0x3000)/8)
.equ        VU1_ENTRY_SHAD_RECEIVE_SLOW, ((VU1_SHADOW_RECEIVE_SLOW-VU1_SHADRECEIVE_XFORM_CODE_START+0x3000)/8)




VU1_SHADRECEIVE_XFORM_CODE_START:

;==============================================================================
;  Shadow receive higher-level code
;==============================================================================

VU1_SHADOW_RECEIVE_FAST:
    nop                                                 xtop    VI05

    
    nop[e]                                              nop
    nop                                                 nop

    nop                                                 isubiu  VI01,   VI05,   (8 + (80 * 4))*2
    nop                                                 nop
    nop                                                 ibeq    VI01,   vi00,   SHAD_RECEIVE_BUFFER2
    nop                                                 iaddiu  VI06,   vi00,   0
    nop                                                 iaddiu  VI06,   VI05,   (8 + (80 * 4))
    SHAD_RECEIVE_BUFFER2:
    nop                                                 ilw.z   VI04,   0(VI05)
    nop                                                 lq.xyzw VF08xyzw, 1+3(VI05)
    nop                                                 lq.xyzw VF07xyzw, 1+2(VI05)
    nop                                                 lq.xyzw VF06xyzw, 1+1(VI05)
    nop                                                 lq.xyzw VF05xyzw, 1+0(VI05)
    nop                                                 iaddiu  VI01,   vi00,   0x7f
    nop                                                 iand    VI02,   VI04,   VI01
    nop                                                 bal     VI15,   TRANSFORM_AND_BACKUP_FAST
    nop                                                 nop
    nop                                                 b       VU1_SHADRECEIVE_XFORM_CODE_START+0x0FC0-0x3000+16384
    nop                                                 nop

VU1_SHADOW_RECEIVE_SKIN_FAST:
    nop[e]                                              nop
    nop                                                 nop
    nop[e]                                              nop
    nop                                                 nop
    nop[e]                                              nop
    nop                                                 nop

VU1_SHADOW_RECEIVE_SLOW:
    nop                                                 xtop        VI05
    nop                                                 ilw.z       VI04,       0(VI05)
    nop                                                 iaddiu      VI07, vi00, 0
    nop                                                 iaddiu      VI06, vi00, ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) )
    nop                                                 isw.x       VI07,   ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)
    nop                                                 isw.y       VI05,   ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)
    nop                                                 b           VU1_SHADRECEIVE_XFORM_CODE_START+0x0040-0x3000+16384
    nop                                                 isw.z       VI06,   ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)

VU1_SHADOW_RECEIVE_SKIN_SLOW:
    nop[e]                                              nop
    nop                                                 nop
    nop[e]                                              nop
    nop                                                 nop
    nop[e]                                              nop
    nop                                                 nop

;==============================================================================
;  Rigid shadow receiving transform fast
;==============================================================================

TRANSFORM_AND_BACKUP_FAST:











    ; concatenate the l2w and w2s matrices
    mulaw.xyzw      acc,        VF04xyzw, VF08w     isubiu  VI01, VI02, 4
    maddaz.xyzw     acc,        VF03xyzw, VF08z     iadd    VI10, vi00, VI05
    madday.xyzw     acc,        VF02xyzw, VF08y     ibgtz   VI01, RIGID_4ORMORE_VERTS
    maddx.xyzw      VF12xyzw, VF01xyzw, VF08x     iadd    VI01, vi00, VI02
    nop                                                 iaddiu  VI01, vi00, 4

RIGID_4ORMORE_VERTS:
    mulaw.xyzw      acc,        VF04xyzw, VF07w     iaddiu  VI11, VI06, 0x00
    maddaz.xyzw     acc,        VF03xyzw, VF07z     iadd    VI12, VI10, VI01
    madday.xyzw     acc,        VF02xyzw, VF07y     iadd    VI12, VI12, VI01
    maddx.xyzw      VF11xyzw, VF01xyzw, VF07x     iadd    VI12, VI12, VI01
    mulaw.xyzw      acc,        VF04xyzw, VF06w     iadd    VI12, VI12, VI01
    maddaz.xyzw     acc,        VF03xyzw, VF06z     mfir.w  VF18w, vi00
    madday.xyzw     acc,        VF02xyzw, VF06y     nop
    maddx.xyzw      VF10xyzw, VF01xyzw, VF06x     nop
    mulaw.xyzw      acc,        VF04xyzw, VF05w     nop
    maddaz.xyzw     acc,        VF03xyzw, VF05z     nop
    madday.xyzw     acc,        VF02xyzw, VF05y     isubiu  VI12, VI12, 4*3
    maddx.xyzw      VF09xyzw, VF01xyzw, VF05x     nop

    ; loop preamble
    nop                                                 lq.xyzw VF13xyzw, 11(VI10)                   ;                   ; load (0)
    mulaw.xyzw      acc,        VF12xyzw, vf00w       nop                                                 ; xform (0)         ;
    nop                                                 nop                                                 ;                   ;
    nop                                                 nop                                                 ;                   ;
    maddaz.xyzw     acc,        VF11xyzw, VF13z     sq.xyzw VF13xyzw, 11(VI11)                   ; xform (0)         ; backup (0)
    madday.xyzw     acc,        VF10xyzw, VF13y     nop                                                 ; xform (0)         ;
    maddx.xyzw      VF14xyzw, VF09xyzw, VF13x     nop                                                 ; xform (0)         ;
    nop                                                 nop                                                 ;                   ;
    nop                                                 lq.xyzw VF13xyzw, 11+(4*1)(VI10) ;                   ; load (1)

    mulaw.xyzw      acc,        VF12xyzw, vf00w       nop                                                 ; xform (1)         ;
    nop                                                 nop                                                 ;                   ;
    add.xyz         VF15xyz,  VF14xyz,  vf00xyz     div     q,  vf00w,  VF14w                         ; copy (0)          ; divide (0)
    maddaz.xyzw     acc,        VF11xyzw, VF13z     sq.xyzw VF13xyzw, 11+(4*1)(VI11) ; xform (1)         ; backup (1)
    madday.xyzw     acc,        VF10xyzw, VF13y     nop                                                 ; xform (1)         ;
    maddx.xyzw      VF14xyzw, VF09xyzw, VF13x     nop                                                 ; xform (1)         ;
    nop                                                 nop                                                 ;                   ;
    nop                                                 lq.xyzw VF13xyzw, 11+(4*2)(VI10) ;                   ; load (2)

    mulaw.xyzw      acc,        VF12xyzw, vf00w       nop                                                 ; xform (2)         ;
    mulq.xyz        VF16xyz,  VF15xyz,  q           nop                                                 ; project xyz (0)   ;
    add.xyz         VF15xyz,  VF14xyz,  vf00xyz     div     q,  vf00w,  VF14w                         ; copy (1)          ; divide (1)
    maddaz.xyzw     acc,        VF11xyzw, VF13z     sq.xyzw VF13xyzw, 11+(4*2)(VI11) ; xform (2)         ; backup (2)
    madday.xyzw     acc,        VF10xyzw, VF13y     nop                                                 ; xform (2)         ;
    maddx.xyzw      VF14xyzw, VF09xyzw, VF13x     nop                                                 ; xform (2)         ;
    addq.z          VF18z,    vf00z,      q           nop                                                 ; project q (0)     ;
    ftoi4.xyz       VF17xyz,  VF16xyz               lq.xyzw VF13xyzw, 11+(4*3)(VI10) ; ftoi (0)          ; load (3)

RIGID_XFORM_FAST_LOOP:
    mulaw.xyzw      acc,        VF12xyzw, vf00w       iaddiu  VI10, VI10, 4                   ; xform (i+3)       ; vptr++
    mulq.xyz        VF16xyz,  VF15xyz,  q           iaddiu  VI11, VI11, 4                   ; project xyz (i+1) ; nptr++
    add.xyz         VF15xyz,  VF14xyz,  vf00xyz     div     q,  vf00w,  VF14w                         ; copy (i+2)        ; divide (i+2)
    maddaz.xyzw     acc,        VF11xyzw, VF13z     sq.xyzw VF13xyzw, 11+(4*2)(VI11) ; xform (i+3)       ; backup (i+3)
    madday.xyzw     acc,        VF10xyzw, VF13y     sq.xyz  VF17xyz,  11-(4*1)(VI10) ; xform (i+3)       ; store xyz (i)
    maddx.xyzw      VF14xyzw, VF09xyzw, VF13x     sq.zw   VF18zw,   9-(4*1)(VI10) ; xform (i+3)       ; store q (i)
    addq.z          VF18z,    vf00z,      q           ibne    VI10, VI12, RIGID_XFORM_FAST_LOOP           ; project q(i+1)    ; loop
    ftoi4.xyz       VF17xyz,  VF16xyz               lq.xyzw VF13xyzw, 11+(4*3)(VI10) ; ftoi (i+1)        ; load (i+4)

    ; finish up the last few verts
    nop                                                 iaddiu  VI10, VI10, 4                   ;                   ; vptr++
    mulq.xyz        VF16xyz,  VF15xyz,  q           iaddiu  VI11, VI11, 4                   ; project xyz (n-1) ; nptr++
    add.xyz         VF15xyz,  VF14xyz,  vf00xyz     div     q,  vf00w,  VF14w                         ; copy (n)          ; divide (n)
    nop                                                 nop                                                 ;                   ;
    nop                                                 sq.xyz  VF17xyz,  11-(4*1)(VI10) ;                   ; store xyz (n-2)
    nop                                                 sq.zw   VF18zw,    9-(4*1)(VI10) ;                   ; store q (n-2)
    addq.z          VF18z,    vf00z,      q           nop                                                 ; project q (n-1)   ;
    ftoi4.xyz       VF17xyz,  VF16xyz               nop                                                 ; ftoi (n-1)        ;

    nop                                                 iaddiu  VI10, VI10, 4                   ;                   ; vptr++
    mulq.xyz        VF16xyz,  VF15xyz,  q           iaddiu  VI11, VI11, 4                   ; project xyz (n)   ; nptr++
    nop                                                 nop                                                 ;                   ;
    nop                                                 nop                                                 ;                   ;
    nop                                                 sq.xyz  VF17xyz,  11-(4*1)(VI10) ;                   ; store xyz (n-1)
    nop                                                 sq.zw   VF18zw,   9-(4*1)(VI10) ;                   ; store q (n-1)
    addq.z          VF18z,    vf00z,      q           nop                                                 ; project q (n)     ;
    ftoi4.xyz       VF17xyz,  VF16xyz               nop                                                 ; ftoi (n)          ;

    nop                                                 iaddiu  VI10, VI10, 4                   ;                   ; vptr++
    nop                                                 iaddiu  VI11, VI11, 4                   ;                   ; nptr++
    nop                                                 nop                                                 ;                   ;
    nop                                                 nop                                                 ;                   ;
    nop                                                 sq.xyz  VF17xyz,  11-(4*1)(VI10) ;                   ; store xyz (n)
    nop                                                 sq.zw   VF18zw,   9-(4*1)(VI10) ;                   ; store q (n)

    ; finished
    nop                                                 jr      VI15
    nop                                                 nop











VU1_SHADRECEIVE_XFORM_CODE_END:
