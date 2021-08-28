#line 1 ".\\vu1\\mcode\\clipper.vu"

















#line 1 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"





























































                                    













                                    



































































































#line 177 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"











#line 189 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































#line 314 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































































































































































































































































































































































































#line 791 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"

#line 19 ".\\vu1\\mcode\\clipper.vu"

.vu 
.org 0x0040
.align 4 
.global VU1_CLIPPER_CODE_START
.global VU1_CLIPPER_CODE_END









VU1_CLIPPER_CODE_START:
CLIPPER_ADDRESS_START:
























































































    nop                                                 lq          VF13xyzw, ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 )+0(VI07)
    nop                                                 lq          VF14xyzw, ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 )+1(VI07)
    nop                                                 lq          VF15xyzw, ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 )+2(VI07)
    nop                                                 lq          VF16xyzw, ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 )+3(VI07)

    nop                                                 lq          VF05xyzw, 1+0(VI05)
    nop                                                 lq          VF06xyzw, 1+1(VI05)
    nop                                                 lq          VF07xyzw, 1+2(VI05)
    nop                                                 lq          VF08xyzw, 1+3(VI05) 

    mulax.xyzw      acc,        VF13xyzw, VF05x     ilw.z       VI04, 0(VI05)
    madday.xyzw     acc,        VF14xyzw, VF05y     iaddiu      VI08, VI05, 0x00                        ; Get pointer to start of vertice
    maddaz.xyzw     acc,        VF15xyzw, VF05z     iaddiu      VI01, vi00, 0x7F
    maddw.xyzw      VF09xyzw, VF16xyzw, VF05w     iand        VI02, VI04, VI01
                                                                   
    mulax.xyzw      acc,        VF13xyzw, VF06x     iadd        VI09, VI08, VI02                        ; Calculate end address for loop 
    madday.xyzw     acc,        VF14xyzw, VF06y     iadd        VI09, VI09, VI02 
    maddaz.xyzw     acc,        VF15xyzw, VF06z     iadd        VI09, VI09, VI02 
    maddw.xyzw      VF10xyzw, VF16xyzw, VF06w     iadd        VI09, VI09, VI02 
                                                                   
    mulax.xyzw      acc,        VF13xyzw, VF07x     iaddiu      VI10, VI07, ( ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )          ; Get pointer to output buffer for indexes
    madday.xyzw     acc,        VF14xyzw, VF07y     nop
    maddaz.xyzw     acc,        VF15xyzw, VF07z     nop
    maddw.xyzw      VF11xyzw, VF16xyzw, VF07w     nop 
                                                                   
    mulax.xyzw      acc,        VF13xyzw, VF08x     iaddiu      VI11, vi00, 0x7FFF                      ; Get the ADC Bit mask
    madday.xyzw     acc,        VF14xyzw, VF08y     iaddiu      VI11, VI11, 1      
    maddaz.xyzw     acc,        VF15xyzw, VF08z     iadd        VI14, vi00, vi00                        ; Clear the index counter
    maddw.xyzw      VF12xyzw, VF16xyzw, VF08w     nop

    nop                                                 sq          VF09xyzw, ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 )+0(VI07)        ; Save the L2C matrix
    nop                                                 sq          VF10xyzw, ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 )+1(VI07)
    nop                                                 sq          VF11xyzw, ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 )+2(VI07)
    nop                                                 sq          VF12xyzw, ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)

    addw.xyzw       VF24xyzw, VF00xyzw,   VF00w       lq          VF17xyzw, ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 )+0(VI07)        ; Load the C2S matrix
    nop                                                 lq          VF18xyzw, ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 )+1(VI07)
    nop                                                 lq          VF19xyzw, ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 )+2(VI07)
    nop                                                 lq          VF20xyzw, ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 )+3(VI07)

CLIPTEST_LOOP:

    addw.zw     VF24zw,   VF00zw,     VF00w           ilw.w       VI13,       11+4(VI08)     ; Get ADC bits from next vertex (OK to read past end of buffer)
    nop                                                 lq          VF23xyzw, 11(VI08)                   ; Get next vertex Position
    nop                                                 lq.xy       VF24xy,   9(VI08)                   ; Get texture coordinates
    nop                                                 lq          VF25xyzw, 10(VI08)                   ; Get Instance Color
    nop                                                 isub        VI01,   VI08,   VI05                        ; calc backup position

    mulax.xyzw  acc,        VF09xyzw, VF23x         iadd        VI01,   VI01,   VI06                        ; calc backup position
    madday.xyzw acc,        VF10xyzw, VF23y         mtir        VI12, VF23w                               ; Transform Position to Clip space
    maddaz.xyzw acc,        VF11xyzw, VF23z         sq          VF23xyzw, 11(VI01)                   ; Backup Position to Next Buffer
    maddw.xyzw  VF23xyzw, VF12xyzw, VF00w           sq          VF24xyzw, 9(VI01)                   ; Backup UV to Next Buffer

    ; Transform Position by Screen space
    mulax.xyzw  acc,        VF17xyzw, VF23x         sq          VF25xyzw, 10(VI01)                   ; Backup Color to Next Buffer
    madday.xyzw acc,        VF18xyzw, VF23y         nop
    maddaz.xyzw acc,        VF19xyzw, VF23z         nop
    maddw.xyzw  VF26xyzw, VF20xyzw, VF23w         nop
    
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop

    itof12.xy   VF24xy,   VF24xy                    div         q,  VF00w,  VF26w                     ; Start perspective divide
    nop                                                 waitq
    
    mulq.xyz    VF26xyz,  VF26xyz,  q               nop
    mulq.xyz    VF27xyz,  VF24xyz,  q               nop
    nop                                                 nop
    nop                                                 nop
    ftoi4.xyz   VF26xyz,  VF26xyz                   nop
    
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop

    nop                                                 sq.xyz      VF27xyz,  9(VI08)
    nop                                                 sq.xyz      VF26xyz,  11(VI08)

    nop                                                 iand        VI01, VI12, VI11                        ; Test if current vertex has ADC bit set
    nop                                                 nop
    nop                                                 ibeq        VI01, vi00, NO_ADC
    nop                                                 iand        VI01, VI13, VI11                        ; Test if next vertex has ADC bit set
    nop                                                 nop
    nop                                                 ibeq        VI01, vi00, NO_ADC
    nop                                                 nop
    nop                                                 fcset       0                                       ; Clear Clip flags since starting a new Tri

NO_ADC:

    clipw.xyz   VF23xyz,  VF23w                     nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 move        VF21xyzw, VF22xyzw                  ; Test if the vertex needs clipped | Roll the queue 
    nop                                                 move        VF22xyzw, VF23xyzw                                           
    nop                                                 fcand       VI01, 0x3FFFF                           ; Trivial Accept test
    nop                                                 ibeq        VI01, vi00, NO_CLIP
    
    nop                                                 iand        VI01, VI12, VI11
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, REJECT                      ; Check if this is NOT the first 2 verts on a new Tri
    nop                                                 iadd        VI03, vi00, vi00

    nop                                                 fcor        VI01, ~( ( 1 << 0 ) | ( ( 1 << 0 ) << 6 ) | ( ( 1 << 0 ) << 12 ))                   ; Test if all vertices are > +X
    nop                                                 ior         VI03, VI03, VI01
    nop                                                 fcor        VI01, ~( ( 1 << 1 ) | ( ( 1 << 1 ) << 6 ) | ( ( 1 << 1 ) << 12 ))                   ; Test if all vertices are < -X
    nop                                                 ior         VI03, VI03, VI01

    nop                                                 fcor        VI01, ~( ( 1 << 2 ) | ( ( 1 << 2 ) << 6 ) | ( ( 1 << 2 ) << 12 ))                   ; Test if all vertices are > +Y
    nop                                                 ior         VI03, VI03, VI01
    nop                                                 fcor        VI01, ~( ( 1 << 3 ) | ( ( 1 << 3 ) << 6 ) | ( ( 1 << 3 ) << 12 ))                   ; Test if all vertices are < -Y
    nop                                                 ior         VI03, VI03, VI01

    nop                                                 fcand       VI01, ( ( 1 << 4 ) | ( ( 1 << 4 ) << 6 ) | ( ( 1 << 4 ) << 12 ))                    ; Test if ANY vertices are > +Z
    nop                                                 ior         VI03, VI03, VI01
    nop                                                 fcor        VI01, ~( ( 1 << 5 ) | ( ( 1 << 5 ) << 6 ) | ( ( 1 << 5 ) << 12 ))                   ; Test if all vertices are < -Z
    nop                                                 ior         VI03, VI03, VI01
    
    nop                                                 nop
    nop                                                 ibne        VI03, vi00, REJECT                      ; Trivial Reject test
    nop                                                 isubiu      VI01, VI14, 2                           ; Get starting index for Tri

    nop                                                 isw.w       VI01, 0(VI10)                           ; Store index
    nop                                                 iaddiu      VI10, VI10, 1

REJECT:

    nop                                                 ior         VI01, VI12, VI11
    nop                                                 isw.w       VI01,       11(VI08)                   ; Set ADC Bit

NO_CLIP:

    nop                                                 iaddiu      VI08, VI08, 4               ; Advance to next vertex 
    nop                                                 nop
    nop                                                 ibne        VI08, VI09, CLIPTEST_LOOP
    nop                                                 iaddiu      VI14, VI14, 1                           ; Increment vertex index

;------------------------------------------------------------------------------

    nop                                                 isub        VI10, VI10, VI07
    nop                                                 isubiu      VI10, VI10, ( ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )          ; Get number of Indexes in list

    nop                                                 lq          VF21xyzw, 0(VI05)             ; Backup the first vector (Flags, Count etc)
    nop                                                 sq          VF21xyzw, ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 0(VI07)
    nop                                                 isw.x       VI10,       ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 1(VI07)

    nop                                                 iaddiu      VI01, vi00, ((CLIP_RET)-CLIPPER_ADDRESS_START+0x0040)
    nop                                                 isw.w       VI01,       ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 39 )(vi00)          ; Store Return Address

    nop                                                 ilw.x       VI03,       ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 0 )(vi00)
    nop                                                 iaddiu      VI01, vi00, (1 << 13)
    nop                                                 ior         VI03, VI03, VI01
    nop                                                 isw.x       VI03,       ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 0 )(vi00)

    nop                                                 b           VU1_CLIPPER_CODE_START+0x0FC0-0x0040
    nop                                                 nop

;==============================================================================
;
;   Triangle Clipper
;
;==============================================================================

CLIP_RET:

    nop                                                 ilw.x       VI08,       ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 1(VI07)  ; Load number of Clipped vertices
    nop                                                 nop
    nop                                                 ibeq        VI08, vi00, CLIP_EXIT                   ; Check for NO vertices needed clipping
    nop                                                 nop

    nop                                                 lq          VF09xyzw, ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 0(VI07)
    nop                                                 lq          VF10xyzw, ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 1(VI07)
    nop                                                 lq          VF11xyzw, ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 2(VI07)
    nop                                                 lq          VF12xyzw, ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 3(VI07)

    nop                                                 lq          VF17xyzw, ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 0(VI07)
    nop                                                 lq          VF18xyzw, ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 1(VI07)
    nop                                                 lq          VF19xyzw, ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 2(VI07)
    nop                                                 lq          VF20xyzw, ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 3(VI07)

    addz.x      VF23x,    VF00x,      VF05z         move.x      VF21x,    VF05x                     ; W2L = L2W.InvertRT()
    addz.y      VF23y,    VF00y,      VF06z         mr32.w      VF21w,    VF00w   
    addx.y      VF21y,    VF00y,      VF06x         move.y      VF22y,    VF06y
    addx.z      VF21z,    VF00z,      VF07x         mr32.w      VF22w,    VF00w  
    addy.x      VF22x,    VF00x,      VF05y         move.z      VF23z,    VF07z
    addy.z      VF22z,    VF00z,      VF07y         mr32.w      VF23w,    VF00w   
    mulax.xyz   acc,        VF21xyz,  VF08x         move.w      VF24w,    VF00w
    madday.xyz  acc,        VF22xyz,  VF08y         nop
    maddz.xyz   VF24xyz,  VF23xyz,  VF08z         nop
    sub.xyz     VF24xyz,  VF00xyz,    VF24xyz       nop

    mulax.xyzw  acc,        VF21xyzw, VF17x         nop                                                 ; C2L = W2L * C2W
    madday.xyzw acc,        VF22xyzw, VF17y         nop
    maddaz.xyzw acc,        VF23xyzw, VF17z         nop
    maddw.xyzw  VF13xyzw, VF24xyzw, VF17w         nop
                                                     
    mulax.xyzw  acc,        VF21xyzw, VF18x         nop
    madday.xyzw acc,        VF22xyzw, VF18y         nop
    maddaz.xyzw acc,        VF23xyzw, VF18z         nop
    maddw.xyzw  VF14xyzw, VF24xyzw, VF18w         nop
                                                     
    mulax.xyzw  acc,        VF21xyzw, VF19x         nop
    madday.xyzw acc,        VF22xyzw, VF19y         nop
    maddaz.xyzw acc,        VF23xyzw, VF19z         nop
    maddw.xyzw  VF15xyzw, VF24xyzw, VF19w         nop

    mulax.xyzw  acc,        VF21xyzw, VF20x         nop
    madday.xyzw acc,        VF22xyzw, VF20y         nop
    maddaz.xyzw acc,        VF23xyzw, VF20z         nop
    maddw.xyzw  VF16xyzw, VF24xyzw, VF20w         iaddiu      VI09, VI07, ( ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )

    nop                                                 sq          VF13xyzw, ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 )+0(VI07)        ; Save the C2L matrix
    nop                                                 sq          VF14xyzw, ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 )+1(VI07)
    nop                                                 sq          VF15xyzw, ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 )+2(VI07)
    nop                                                 sq          VF16xyzw, ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 )+3(VI07)

;==============================================================================

CLIP_VERT_LOOP:

    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )             ; Wait for GS to finish
    nop                                                 xgkick      VI01

    sub         VF15xyzw, VF00xyzw,   VF00xyzw        ilw.w       VI01, 0(VI09)                           ; Get starting index of tri
    sub         VF14xyzw, VF00xyzw,   VF00xyzw        ilw.z       VI10, ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)
    sub         VF16xyzw, VF00xyzw,   VF00xyzw        iadd        VI10, VI10, VI01                        ; Compute address of vertex
    add         VF13xyzw, VF15xyzw, VF00xyzw        iadd        VI10, VI10, VI01
    nop                                                 iadd        VI10, VI10, VI01
    nop                                                 iadd        VI10, VI10, VI01
    nop                                                 iaddiu      VI10, VI10, 8

    nop                                                 lq          VF25xyzw, ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 )+0(VI07)        ; Load the L2C matrix
    nop                                                 lq          VF26xyzw, ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 )+1(VI07)
    nop                                                 lq          VF27xyzw, ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 )+2(VI07)
    nop                                                 lq          VF28xyzw, ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 )+3(VI07)

    nop                                                 lqi.xyz     VF15xyz,  (VI10++)                    ; Load vertex 0 of Tri
    itof12.xyz  VF15xyz,  VF15xyz                   lqi.xy      VF14xy,   (VI10++)
    itof12.xy   VF14xy,   VF14xy                    lqi.xyzw    VF16xyzw, (VI10++)
    itof12.xyzw VF16xyzw, VF16xyzw                  lqi.xyz     VF13xyz,  (VI10++)

    nop                                                 bal         VI15,   TRANSFORM_VERT
    nop                                                 iaddiu      VI11,   VI07,   ( 0 + 8 )
    
    nop                                                 sqi         VF19xyzw, (VI11++)                    ; Store vertex 0
    nop                                                 sqi         VF18xyzw, (VI11++) 
    nop                                                 sqi         VF20xyzw, (VI11++) 
    nop                                                 sqi         VF17xyzw, (VI11++) 

    nop                                                 lqi.xyz     VF15xyz,  (VI10++)                    ; Load vertex 1 of Tri
    itof12.xyz  VF15xyz,  VF15xyz                   lqi.xy      VF14xy,   (VI10++)
    itof12.xy   VF14xy,   VF14xy                    lqi.xyzw    VF16xyzw, (VI10++)
    itof12.xyzw VF16xyzw, VF16xyzw                  lqi.xyz     VF13xyz,  (VI10++)

    nop                                                 bal         VI15, TRANSFORM_VERT
    nop                                                 nop
    
    nop                                                 sqi         VF19xyzw, (VI11++)                    ; Store vertex 1
    nop                                                 sqi         VF18xyzw, (VI11++) 
    nop                                                 sqi         VF20xyzw, (VI11++) 
    nop                                                 sqi         VF17xyzw, (VI11++) 

    nop                                                 ilw.w       VI01, 3(VI10)                           ; backup winding of tri
    nop                                                 isw.y       VI01, ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 1(VI07)

    nop                                                 lqi.xyz     VF15xyz,  (VI10++)                    ; Load vertex 2 of Tri
    itof12.xyz  VF15xyz,  VF15xyz                   lqi.xy      VF14xy,   (VI10++)
    itof12.xy   VF14xy,   VF14xy                    lqi.xyzw    VF16xyzw, (VI10++)
    itof12.xyzw VF16xyzw, VF16xyzw                  lqi.xyz     VF13xyz,  (VI10++)

    nop                                                 bal         VI15, TRANSFORM_VERT
    nop                                                 nop
    
    nop                                                 sqi         VF19xyzw, (VI11++)                    ; Store vertex 2
    nop                                                 sqi         VF18xyzw, (VI11++)
    nop                                                 sqi         VF20xyzw, (VI11++) 
    nop                                                 sqi         VF17xyzw, (VI11++) 

    nop                                                 iaddiu      VI10, VI07, ( 0 + 8 )             ; Get address of vertices in Clip Space
    nop                                                 iaddiu      VI12, vi00, 3                           ; Initially we have 3 vertices
    nop                                                 iaddiu      VI13, VI07, ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) )             ; Get address of Plane Table
    nop                                                 fcset       0                                       ; Clear the Clip Flags

;==============================================================================

CLIP_PLANE_LOOP:

    nop                                                 ilw.x       VI14, 0(VI13)                           ; Get the Plane Mask
    nop                                                 ibeq        VI14, vi00, CLIP_DONE_PLANES            ; Any more planes left?
    nop                                                 nop

    nop                                                 iadd        VI01, VI12, VI12                        ; Get address of Destination Buffer
    nop                                                 iadd        VI01, VI01, VI12
    nop                                                 iadd        VI01, VI01, VI12
    nop                                                 iadd        VI11, VI10, VI01                        ; pDst = pSrc + nDst
    nop                                                 iadd        VI06, VI11, vi00                        ; pEnd = pDst
    nop                                                 iadd        VI12, vi00, vi00                        ; nDst = 0

    nop                                                 isubiu      VI01, VI11, 4                           ; Get Last vertex from Source Buffer
    nop                                                 lqi         VF15xyzw, (VI01++)                    ; This is the Edge Start Vertex
    nop                                                 lqi         VF14xyzw, (VI01++)
    nop                                                 lqi         VF16xyzw, (VI01++)
    nop                                                 lqi         VF13xyzw, (VI01++)

    clipw.xyz   VF13xyz,  VF13w                     nop                                                 ; Clip Test
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 fcget       VI03
    nop                                                 iand        VI03, VI03, VI14                        ; Check if vertex is outside Plane

;==============================================================================

CLIP_EDGE_LOOP:

    nop                                                 lqi         VF19xyzw, (VI10++)                    ; Get Edge End vertex from Source Buffer
    nop                                                 lqi         VF18xyzw, (VI10++)
    nop                                                 lqi         VF20xyzw, (VI10++)
    nop                                                 lqi         VF17xyzw, (VI10++)
    
    clipw.xyz   VF17xyz,  VF17w                     nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 fcget       VI04
    nop                                                 iand        VI04, VI04, VI14                        ; Check if vertex is outside Plane

    nop                                                 ibeq        VI03, vi00, CLIP_INSIDE                 ; Vertex 0 Inside?
    nop                                                 nop
    nop                                                 ibne        VI04, vi00, CLIP_EDGE_NEXT              ; Outside -> Outside?
    nop                                                 nop

;   Outside -> Inside

    nop                                                 move        VF23xyzw, VF15xyzw                  ; Swap Edge vertexes so we always Clip Inside->Outside
    nop                                                 move        VF22xyzw, VF14xyzw                  ; This will reduce cracking
    nop                                                 move        VF24xyzw, VF16xyzw
    nop                                                 move        VF21xyzw, VF13xyzw
    nop                                                 move        VF15xyzw, VF19xyzw
    nop                                                 move        VF14xyzw, VF18xyzw
    nop                                                 move        VF16xyzw, VF20xyzw
    nop                                                 move        VF13xyzw, VF17xyzw
    nop                                                 move        VF19xyzw, VF23xyzw
    nop                                                 move        VF18xyzw, VF22xyzw
    nop                                                 move        VF20xyzw, VF24xyzw
    nop                                                 move        VF17xyzw, VF21xyzw

    nop                                                 bal         VI15,   INTERPOLATE
    nop                                                 nop
    
    nop                                                 sqi         VF23xyzw, (VI11++)                    ; Output Intersection vertex
    nop                                                 sqi         VF22xyzw, (VI11++)                    
    nop                                                 sqi         VF24xyzw, (VI11++)
    nop                                                 sqi         VF21xyzw, (VI11++)
    nop                                                 sqi         VF15xyzw, (VI11++)                    ; Output Edge End vertex
    nop                                                 sqi         VF14xyzw, (VI11++)                    
    nop                                                 sqi         VF16xyzw, (VI11++)
    nop                                                 sqi         VF13xyzw, (VI11++)

    nop                                                 b           CLIP_EDGE_SWAP
    nop                                                 iaddiu      VI12, VI12, 2                           ; nDst += 2

CLIP_INSIDE:

    nop                                                 ibeq        VI04, vi00, CLIP_INSIDE_INSIDE          ; Inside -> Inside?
    nop                                                 nop

;   Inside -> Outside

    nop                                                 bal         VI15,   INTERPOLATE
    nop                                                 nop
    
    nop                                                 sqi         VF23xyzw, (VI11++)                    ; Output Intersection vertex
    nop                                                 sqi         VF22xyzw, (VI11++)
    nop                                                 sqi         VF24xyzw, (VI11++)
    nop                                                 sqi         VF21xyzw, (VI11++)

    nop                                                 b           CLIP_EDGE_NEXT
    nop                                                 iaddiu      VI12, VI12, 1                           ; nDst++

;   Inside -> Inside

CLIP_INSIDE_INSIDE:

    nop                                                 sqi         VF19xyzw, (VI11++)                    ; Output Edge End vertex
    nop                                                 sqi         VF18xyzw, (VI11++)
    nop                                                 sqi         VF20xyzw, (VI11++)
    nop                                                 sqi         VF17xyzw, (VI11++)
    
    nop                                                 iaddiu      VI12, VI12, 1                           ; nDst++

CLIP_EDGE_NEXT:

    nop                                                 move        VF15xyzw, VF19xyzw                  ; Edge Start Vertex = End Vertex
    nop                                                 move        VF14xyzw, VF18xyzw
    nop                                                 move        VF16xyzw, VF20xyzw
    nop                                                 move        VF13xyzw, VF17xyzw
    
CLIP_EDGE_SWAP:

    nop                                                 iadd        VI03, vi00, VI04
    nop                                                 ibne        VI10, VI06, CLIP_EDGE_LOOP              ; Looped through all edges?
    nop                                                 nop

    nop                                                 isubiu      VI01, VI12, 3                           ; Ensure we have at least 3 vertices
    nop                                                 nop
    nop                                                 ibltz       VI01, CLIP_VERT_NEXT
    nop                                                 nop

    nop                                                 b           CLIP_PLANE_LOOP                         ; Looped through all planes?
    nop                                                 iaddiu      VI13, VI13, 1

;==============================================================================

CLIP_DONE_PLANES:

    nop                                                 iaddiu      VI03, VI07, ( 0 + 8 )             ; Set Output address for Strip vertices
    nop                                                 iaddiu      VI04, VI07, ( ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )          ; Set Output address for C2L vertices
    nop                                                 iaddiu      VI11, vi00, 1
    nop                                                 iadd        VI14, vi00, vi00

CLIP_STRIP_LOOP:

    nop                                                 isubiu      VI01, VI14, 1                           ; Check if VI14 > 1
    nop                                                 iadd        VI13, VI14, vi00                        ; Set Index
    nop                                                 iblez       VI01, CLIP_START_STRIP
    nop                                                 nop
    nop                                                 iand        VI01, VI14, VI01                        ; Odd vertex?
    nop                                                 nop
    nop                                                 ibeq        VI01, vi00, CLIP_EVEN_VERT
    nop                                                 nop
    nop                                                 iaddiu      VI11, VI11, 1                           ; VI11++
    nop                                                 b           CLIP_START_STRIP
    nop                                                 iadd        VI13, VI11, vi00                        ; Index = VI11

CLIP_EVEN_VERT:

    nop                                                 isub        VI13, VI12, VI11                        ; Index = VI12 - VI11

CLIP_START_STRIP:

    nop                                                 iadd        VI01, VI10, VI13                        ; Compute pSrc[Index]
    nop                                                 iadd        VI01, VI01, VI13
    nop                                                 iadd        VI01, VI01, VI13
    nop                                                 iadd        VI01, VI01, VI13
    
    nop                                                 lqi         VF15xyzw, (VI01++)                    ; Load vertex
    nop                                                 lqi         VF14xyzw, (VI01++)
    nop                                                 lqi         VF16xyzw, (VI01++)
    nop                                                 lqi         VF13xyzw, (VI01++)
    
    nop                                                 lq          VF25xyzw, ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 )+0(VI07)        ; Set C2L Matrix
    nop                                                 lq          VF26xyzw, ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 )+1(VI07)
    nop                                                 lq          VF27xyzw, ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 )+2(VI07)
    nop                                                 lq          VF28xyzw, ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 )+3(VI07)
    
    nop                                                 bal         VI15, TRANSFORM_VERT
    nop                                                 nop

    addw.z      VF18z,    VF00z,  VF00w               loi         0.0001221                               ; Round up color value

    ; DBS (7/20/2004): Not sure why we needed to round up, but is seems unnecessary, so I'm
    ; commenting this line out and seeing what complaints come up    
;    addi.xyzw   VF20xyzw, VF20xyzw,  I              nop

    ftoi12.xyz  VF19xyz,  VF19xyz                   nop
    ftoi12.xy   VF18xy,   VF18xy                    nop
    ftoi12.xyzw VF20xyzw, VF20xyzw                  nop
    
    nop                                                 sqi         VF19xyzw, (VI03++)                    ; Store vertex 
    nop                                                 sqi         VF18xyzw, (VI03++)
    nop                                                 sqi         VF20xyzw, (VI03++)
    nop                                                 sqi         VF13xyzw, (VI03++)                    ; Output clipped vertex in Clip Space
    nop                                                 sqi.xyz     VF17xyz,  (VI04++)                    ; Output clipped vertex in Local Space

    nop                                                 iaddiu      VI14, VI14, 1
    nop                                                 nop
    nop                                                 ibne        VI14, VI12, CLIP_STRIP_LOOP
    nop                                                 nop

;==============================================================================

    nop                                                 isw.x       VI08,       ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 2(VI07)  ; Backup registers
    nop                                                 isw.y       VI09,       ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 2(VI07)

    nop                                                 ilw.x       VI05,       ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 3(VI07)  ; VI05 = first buffer
    nop                                                 ilw.y       VI06,       ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 3(VI07)  ; VI06 = middle buffer
    nop                                                 nop
    nop                                                 nop
    nop                                                 iaddiu      VI08, VI05, 0x00

    nop                                                 ilw.z       VI04,       ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 0(VI05)
    nop                                                 iadd        VI02, vi00, VI12
    nop                                                 iadd        VI09, VI08, VI02
    nop                                                 iadd        VI09, VI09, VI02
    nop                                                 iadd        VI09, VI09, VI02
    nop                                                 iadd        VI09, VI09, VI02

    nop                                                 iaddiu      VI13, VI07, ( ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 )          ; Get address of C2L positions

    sub             VF14xyzw, VF00xyzw,   VF00xyzw    lq          VF25xyzw, ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 )+0(VI07)        ; Load the C2S matrix
    sub             VF23xyzw, VF00xyzw,   VF00xyzw    lq          VF26xyzw, ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 )+1(VI07)
    addw.xyzw       VF15xyzw, VF00xyzw,   VF00w       lq          VF27xyzw, ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 )+2(VI07)
    nop                                                 lq          VF28xyzw, ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 )+3(VI07)

    nop                                                 ilw.y       VI10, ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 1(VI07)        ; load up winding for triangle
    nop                                                 iaddiu      VI01, vi00, 0x0020
    nop                                                 iand        VI10, VI10, VI01

;==============================================================================

CLIP_TRANSFORM_LOOP:

    nop                                                 lq          VF13xyzw, 11(VI08)                   ; Get Position in Clip space
    nop                                                 lq          VF16xyzw, 9(VI08)
    nop                                                 lqi.xyz     VF23xyz,  (VI13++)                    ; Get C2L position

    mulax.xyzw  acc,        VF25xyzw, VF13x         nop                                                 ; Transform vertex by C2S matrix
    madday.xyzw acc,        VF26xyzw, VF13y         isub        VI01,   VI08,   VI05
    maddaz.xyzw acc,        VF27xyzw, VF13z         iadd        VI01,   VI01,   VI06
    maddw.xyzw  VF13xyzw, VF28xyzw, VF13w         nop
    
    itof12.xy   VF15xy,   VF16xy                    div         q,  VF00w,  VF13w
    nop                                                 sq          VF16xyzw, 9(VI01)
    nop                                                 sq.xyz      VF23xyz,  11(VI01)
    nop                                                 isw.w       VI10, 11(VI01)
    nop                                                 isw.w       VI10, 11(VI08)
    nop                                                 iaddiu      VI01, vi00, 0x0020
    nop                                                 iadd        VI10, VI10, VI01
    nop                                                 iand        VI10, VI10, VI01
    
    mulq.xyz    VF13xyz,  VF13xyz,  q               nop
    mulq.xyz    VF17xyz,  VF15xyz,  q               nop
    ftoi4.xyz   VF14xyz,  VF13xyz                   nop
    
    nop                                                 sq.xyz      VF17xyz,  9(VI08)
    nop                                                 sq.xyz      VF14xyz,  11(VI08)
    
    nop                                                 iaddiu      VI08, VI08, 4
    nop                                                 nop
    nop                                                 ibne        VI08, VI09, CLIP_TRANSFORM_LOOP
    nop                                                 nop

;==============================================================================

    nop                                                 iaddiu      VI01, vi00, 0x7FFF                      ; Set ADC Bit on first 2 vertices
    nop                                                 iaddiu      VI01, VI01, 1
    nop                                                 isw.w       VI01, (4*0)+11(VI05)
    nop                                                 isw.w       VI01, (4*1)+11(VI05)

    nop                                                 lq          VF14xyzw, ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 0(VI07)
    nop                                                 mtir        VI08, VF14w
    nop                                                 iaddiu      VI01, vi00, 0x7FC0
    nop                                                 iaddiu      VI01, VI01, 0x7FC0
    nop                                                 iand        VI01, VI01, VI08
    nop                                                 ior         VI01, VI01, VI02
    nop                                                 mfir.z      VF14z,    VI01
    nop                                                 sq          VF14xyzw, 0(VI05)

    nop                                                 iaddiu      VI01, vi00, ((CLIP_RET2)-CLIPPER_ADDRESS_START+0x0040)
    nop                                                 isw.w       VI01,       ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 39 )(vi00)          ; Store Return Address

    nop                                                 b           VU1_CLIPPER_CODE_START+0x0FC0-0x0040
    nop                                                 nop

;==============================================================================

CLIP_RET2:

    nop                                                 ilw.x       VI08,       ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 2(VI07)
    nop                                                 ilw.y       VI09,       ( ( ( ( ( ( ( ( 0 + 8 ) + (( 3 + 4 + 5 + 6 + 7 + 8 + 9 ) * 4) ) + 7 ) + 4 ) + 4 ) + 4 ) + 4 ) + 4 ) + 2(VI07)

CLIP_VERT_NEXT:

    nop                                                 isubiu      VI08, VI08, 1
    nop                                                 nop
    nop                                                 ibne        VI08, vi00, CLIP_VERT_LOOP
    nop                                                 iaddiu      VI09, VI09, 1

;==============================================================================

CLIP_EXIT:

    nop                                                 ilw.x       VI03,       ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 0 )(vi00)             ; Clear Clip flag
    nop                                                 iaddiu      VI01, vi00, ~(1 << 13)
    nop                                                 iand        VI03, VI03, VI01
    nop                                                 isw.x       VI03,       ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 0 )(vi00)

    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )             ; Wait for GS to finish
    nop                                                 xgkick      VI01

    nop[e]                                              nop
    nop                                                 nop

;==============================================================================
;
;   Transform Vertex by Matrix
;
;==============================================================================

TRANSFORM_VERT:
    
    ; DBS (7/20/2004) : We used to transform the normals, uvs, and colors into clip space, but
    ; I don't really think that bought anything, so I'm taking it out. This allows us to clip
    ; alpha values as well. (I suspect this was done to minimize issues with non-perspective
    ; correct gouraud shading, but I can't tell a difference.)
;    mulax.xyzw  acc,        VF25xyzw, VF15x         nop                                                 ; Transform vertex by matrix
;    madday.xyzw acc,        VF26xyzw, VF15y         nop
;    maddaz.xyzw acc,        VF27xyzw, VF15z         nop
;    maddw.xyzw  VF19xyzw, VF28xyzw, VF15w         nop
    nop                                                 move.xyzw   VF19XYZW, VF15xyzw

;    mulax.xyzw  acc,        VF25xyzw, VF14x         nop
;    madday.xyzw acc,        VF26xyzw, VF14y         nop
;    maddaz.xyzw acc,        VF27xyzw, VF14z         nop
;    maddw.xyzw  VF18xyzw, VF28xyzw, VF14w         nop
    nop                                                 move.xyzw   VF18XYZW, VF14xyzw

;    mulax.xyzw  acc,        VF25xyzw, VF16x         nop
;    madday.xyzw acc,        VF26xyzw, VF16y         nop
;    maddaz.xyzw acc,        VF27xyzw, VF16z         nop
;    maddw.xyzw  VF20xyzw, VF28xyzw, VF16w         nop
    nop                                                 move.xyzw   VF20XYZW, VF16xyzw

    mulax.xyzw  acc,        VF25xyzw, VF13x         nop
    madday.xyzw acc,        VF26xyzw, VF13y         nop
    maddaz.xyzw acc,        VF27xyzw, VF13z         jr  VI15
    maddw.xyzw  VF17xyzw, VF28xyzw, VF13w         nop

;==============================================================================
;
;   Calculate Edge-Plane Intersection
;
;==============================================================================

INTERPOLATE:

    nop                                                 move        VF25xyzw, VF13xyzw
    nop                                                 move        VF26xyzw, VF17xyzw
    nop                                                 ilw.y       VI01, 0(VI13)
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 iaddiu      VI01, VI01, ((VU1_CLIP_JUMPADR)-CLIPPER_ADDRESS_START+0x0040)
    nop                                                 jr          VI01
    nop                                                 nop

;   +VE Plane: Right, Top, Back
;   A = POS0.? - POS0.W
;   B = POS1.W - POS1.?
;   T = A / (A + B)

CLIP_PLANE_POS_Z:

    nop                                                 mr32.xyzw   VF25xyzw, VF25xyzw                  ; CLP0.W = CLP0.X
    nop                                                 mr32.xyzw   VF26xyzw, VF26xyzw                  ; CLP1.W = CLP1.X

CLIP_PLANE_POS_Y:

    nop                                                 mr32.xyzw   VF25xyzw, VF25xyzw                  ; CLP0.W = CLP0.Y
    nop                                                 mr32.xyzw   VF26xyzw, VF26xyzw                  ; CLP1.W = CLP1.Y

CLIP_PLANE_POS_X:

    nop                                                 mr32.xyzw   VF25xyzw, VF25xyzw                  ; CLP0.W = CLP0.Z
    nop                                                 mr32.xyzw   VF26xyzw, VF26xyzw                  ; CLP1.W = CLP1.Z

    subw.w      VF25w,    VF25w,    VF13w         nop                                                 ; CLP0.W = A
    subw.w      VF26w,    VF17w,    VF26w         b           CLIP_CREATE_VERT                        ; CLP1.W = B
    addw.w      VF26w,    VF26w,    VF25w         nop                                                 ; CLP1.W = A + B

;   -VE Plane: Left, Bottom, Front
;   A = POS0.? + POS0.W
;   B = POS1.W + POS1.?
;   T = A / (A - B)

CLIP_PLANE_NEG_Z:

    nop                                                 mr32.xyzw   VF25xyzw, VF25xyzw                  ; CLP0.W = CLP0.X
    nop                                                 mr32.xyzw   VF26xyzw, VF26xyzw                  ; CLP1.W = CLP1.X

CLIP_PLANE_NEG_Y:

    nop                                                 mr32.xyzw   VF25xyzw, VF25xyzw                  ; CLP0.W = CLP0.Y
    nop                                                 mr32.xyzw   VF26xyzw, VF26xyzw                  ; CLP1.W = CLP1.Y

CLIP_PLANE_NEG_X:

    nop                                                 mr32.xyzw   VF25xyzw, VF25xyzw                  ; CLP0.W = CLP0.Z
    nop                                                 mr32.xyzw   VF26xyzw, VF26xyzw                  ; CLP1.W = CLP1.Z

    addw.w      VF25w,    VF25w,    VF13w         nop                                                 ; CLP0.W = A
    addw.w      VF26w,    VF17w,    VF26w         nop                                                 ; CLP1.W = B
    subw.w      VF26w,    VF25w,    VF26w         nop                                                 ; CLP1.W = A + B

CLIP_CREATE_VERT:

    nop                                                 div     q,  VF25w,    VF26w                     ; Q = A / (A+B)
    sub         VF23xyzw, VF19xyzw, VF15xyzw      nop
    sub         VF22xyzw, VF18xyzw, VF14xyzw      nop
    sub         VF24xyzw, VF20xyzw, VF16xyzw      nop
    sub         VF21xyzw, VF17xyzw, VF13xyzw      nop                                                 ; POS2 = (POS1-POS0)
    
    nop                                                 waitq
    mulq.xyzw   VF23xyzw, VF23xyzw, q               nop                                                 ; POS2 *= Q
    mulq.xyzw   VF22xyzw, VF22xyzw, q               nop
    mulq.xyzw   VF24xyzw, VF24xyzw, q               nop
    mulq.xyzw   VF21xyzw, VF21xyzw, q               nop
    
    add.xyzw    VF23xyzw, VF23xyzw, VF15xyzw      nop
    add.xyzw    VF22xyzw, VF22xyzw, VF14xyzw      nop
    add.xyzw    VF24xyzw, VF24xyzw, VF16xyzw      nop
    add.xyzw    VF21xyzw, VF21xyzw, VF13xyzw      nop                                                 ; POS2 = POS0 + ((POS1-POS0) * Q)

    nop                                                 jr  VI15
    nop                                                 nop

VU1_CLIP_JUMPADR:

    nop                                                 b   CLIP_PLANE_NEG_Z                                ;  0
    nop                                                 nop

    nop                                                 b   CLIP_PLANE_POS_Z                                ;  2
    nop                                                 nop

    nop                                                 b   CLIP_PLANE_POS_X                                ;  4
    nop                                                 nop

    nop                                                 b   CLIP_PLANE_NEG_X                                ;  6
    nop                                                 nop

    nop                                                 b   CLIP_PLANE_POS_Y                                ;  8
    nop                                                 nop

    nop                                                 b   CLIP_PLANE_NEG_Y                                ; 10
    nop                                                 nop
















































































VU1_CLIPPER_CODE_END:
