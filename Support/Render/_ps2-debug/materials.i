#line 1 ".\\vu1\\mcode\\materials.vu"










#line 1 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"





























































                                    













                                    



































































































#line 177 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"











#line 189 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































#line 314 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"




























































































































































































































































































































































































































































































#line 791 "c:\\projects\\a51\\support\\render\\vu1\\mcode\\include.vu"

#line 12 ".\\vu1\\mcode\\materials.vu"

.vu 
.org 0x0FC0
.align 4 
.global VU1_MATERIAL_CODE_START
.global VU1_MATERIAL_CODE_END


VU1_MATERIAL_CODE_START:
    nop                                                 ilw.x       VI03,     ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 0 )(vi00)

    ; check for glowing geometry
    nop                                                 iaddiu      VI01, vi00, 0x0100
    nop                                                 iand        VI01, VI04, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, GLOWING_GEOMETRY
    nop                                                 nop
    GLOWING_GEOMETRY_RET:

    ; check for fading geometry
    nop                                                 iaddiu      VI01, vi00, 0x1000
    nop                                                 iand        VI01, VI04, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, FADING_ALPHA
    nop                                                 nop
    FADING_ALPHA_RET:
    
    ; check for uv animation
    nop                                                 iaddiu      VI01, vi00, (1 << 4)
    nop                                                 iand        VI01, VI03, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, UV_SCROLL
    nop                                                 nop
    UV_SCROLL_RET:

    ; check for distortion material
    nop                                                 iaddiu      VI01, vi00, (1 << 12)
    nop                                                 iand        VI01, VI03, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, DISTORTION
    nop                                                 nop
    DISTORTION_RET:

    ; check for a normal diffuse pass
    nop                                                 iaddiu      VI01, vi00, (1 << 0)
    nop                                                 iand        VI01, VI03, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, DIFFUSE
    nop                                                 nop
    DIFFUSE_RET:

    ; check for z-priming
    nop                                                 iaddiu      VI01, vi00, (1 << 8)
    nop                                                 iand        VI01, VI03, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, ZPRIME
    nop                                                 nop
    ZPRIME_RET:

    ; check for a spotlight using the projected texture method
    nop                                                 iaddiu      VI01, vi00, 0x0800
    nop                                                 iand        VI01, VI04, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, SPOTLIGHT
    nop                                                 nop

    ; backup the uv1/normals (not necessary if we did a spotlight pass)
    nop                                                 b           BACKUP_UV1
    nop                                                 nop
    SPOTLIGHT_RET:
    BACKUP_RET:

    ; Local Position, UV0 and UV1/Normal are Backed up to Next Buffer by this point
    ; check for a detail map pass (must have both the instance AND material flag set for this one)
    nop                                                 iaddiu      VI01, vi00, 0x4000
    nop                                                 iand        VI01, VI04, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, DETAIL
    nop                                                 nop
    DETAIL_RET:

    ; check for environment mapping
    nop                                                 iaddiu      VI01, vi00, (1 << 6)
    nop                                                 iand        VI01, VI03, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, ENVIRONMENT_POSITIONS
    
    nop                                                 iaddiu      VI01, vi00, (1 << 1)
    nop                                                 iand        VI01, VI03, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, ENVIRONMENT_NORMALS
    nop                                                 nop
    ENV_MAP_RET:

    ; check for a self-illumination pass
    nop                                                 iaddiu      VI01, vi00, (1 << 2)
    nop                                                 iand        VI01, VI03, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, SELFILLUM
    nop                                                 nop
    SELFILLUM_RET:
    
    ; check for a shadow map pass
    nop                                                 iaddiu      VI01, vi00, 0x0200
    nop                                                 iand        VI01, VI04, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, SHADOW
    nop                                                 nop
    SHADOW_RET:
    
    ; check for a projected shadow pass
    nop                                                 ilw.w       VI14, 0(VI05)
    nop                                                 iaddiu      VI01, vi00, 0x0100
    nop                                                 nop
    nop                                                 nop
    nop                                                 iand        VI01, VI14, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, PROJ_SHADOW_1
    nop                                                 nop
    PROJ_SHADOW_1_RET:
    nop                                                 iaddiu      VI01, vi00, 0x0200
    nop                                                 iand        VI01, VI14, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, PROJ_SHADOW_2
    nop                                                 nop
    PROJ_SHADOW_2_RET:

    ; should we return to the clipper, or are we done?
    nop                                                 iaddiu      VI01, vi00, (1 << 13)
    nop                                                 iand        VI01, VI03, VI01
    nop                                                 nop
    nop                                                 ibeq        VI01, vi00, DONE
    nop                                                 nop
    nop                                                 ilw.w       VI15, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 39 )(vi00)
    nop                                                 jr          VI15
    nop                                                 nop

    ; all of the passes have been completed
    DONE:
    nop[e]                                              nop
    nop                                                 nop
    nop[e]                                              nop
    nop                                                 nop

;==============================================================================
;
; Glowing geometry routine
;
;   Alters the material flags such that we end up turning off environment mapping
;   and turning on self-illumination glow with diffuse lighting
;
;==============================================================================

GLOWING_GEOMETRY:
    ; Note that because glowing geometry is forced to be the last thing rendered,
    ; we can actually overwrite memory in the material data. Normally we'd avoid
    ; this sort of thing, but in this case we'll let it slide.
    nop                                                 iaddiu  VI01, vi00, ~((1 << 1)|(1 << 7)|(1 << 6))
    nop                                                 iand    VI03, VI03, VI01
    nop                                                 iaddiu  VI01, vi00, ((1 << 2)|(1 << 11)|(1 << 10));
    nop                                                 ior     VI03, VI03, VI01
    nop                                                 ilw.w   VI01, 0(VI05)
    nop                                                 isw.w   VI01, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 0 )(vi00)
    nop                                                 iaddiu  VI01, vi00, 0x80
    nop                                                 isw.w   VI01, 0(VI05)
    nop                                                 iaddiu  VI01, vi00, 0x54
    nop                                                 isw.x   VI01, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 7 )(vi00)
    nop                                                 iaddiu  VI01, vi00, 0x42
    nop                                                 b       GLOWING_GEOMETRY_RET
    nop                                                 isw.z   VI01, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 7 )(vi00)


;==============================================================================
;
; Fading alpha routine
;
;   Runs through all of the vertices, and sets the vertex alpha so they should
;   fade out
;
;==============================================================================





FADING_ALPHA:
    nop                                                 ilw.w       VI10, 0(VI05)
    nop                                                 iadd        VI08, VI05, vi00
    nop                                                 iadd        VI09, VI08, VI02
    nop                                                 iadd        VI09, VI09, VI02
    nop                                                 iadd        VI09, VI09, VI02
    nop                                                 iadd        VI09, VI09, VI02
    nop                                                 iaddiu      VI01, vi00, 0xff
    nop                                                 iand        VI10, VI10, VI01
    
FADING_ALPHA_LOOP:
    nop                                                 isw.w       VI10, 10(VI08)
    nop                                                 iaddiu      VI08, VI08, 4
    nop                                                 nop
    nop                                                 ibne        VI08, VI09, FADING_ALPHA_LOOP
    nop                                                 nop
    
    nop                                                 b   FADING_ALPHA_RET
    nop                                                 nop
    




;==============================================================================
;
; UV Scrolling routine
;
;   Multiplies the scroll amount by the screen-space Q, and adds it to the
;   already calculated screen-space ST value.
;
;==============================================================================










UV_SCROLL:
    nop                                                 lq.xy       VF09xy,   0(VI05)
    nop                                                 iadd        VI08, VI05, vi00
    nop                                                 iadd        VI09, VI08, VI02
    nop                                                 iadd        VI09, VI09, VI02
    itof0.xy        VF09xy,   VF09xy                iadd        VI09, VI09, VI02
    nop                                                 iadd        VI09, VI09, VI02
    nop                                                 nop
    nop                                                 loi         0.00392157  ; 1/255
    muli.xy         VF09xy,   VF09xy,   i           nop
    

    ; loop preamble
    nop                                                 lq.xyz      VF10xyz,  9(VI08)                   ;               ; load uv0
    nop                                                 nop                                                     ;               ;
    nop                                                 nop                                                     ;               ;
    nop                                                 nop                                                     ;               ;
    mulz.xy         VF12xy,   VF09xy,   VF10z     move.xyz    VF11xyz,  VF10xyz                       ; project uv0   ; copy uv0

    nop                                                 lq.xyz      VF10xyz,  (9+4*1)(VI08) ;               ; load uv1
    nop                                                 nop                                                     ;               ;
    nop                                                 nop                                                     ;               ;
    add.xy          VF13xy,   VF12xy,   VF11xy    nop                                                     ; scroll uv0    ;
    mulz.xy         VF12xy,   VF09xy,   VF10z     move.xyz    VF11xyz,  VF10xyz                       ; project uv1   ; copy uv1

UV_SCROLL_LOOP:
    nop                                                 lq.xyz      VF10xyz,  (9+4*2)(VI08) ;               ; load uv2
    nop                                                 iaddiu      VI08, VI08, 4                   ;               ; vptr++
    nop                                                 sq.xy       VF13xy,   (9-4)(VI08)   ;               ; store uv0
    add.xy          VF13xy,   VF12xy,   VF11xy    ibne        VI08, VI09, UV_SCROLL_LOOP                  ; scroll uv1    ; loop
    mulz.xy         VF12xy,   VF09xy,   VF10z     move.xyz    VF11xyz,  VF10xyz                       ; project uv2   ; copy uv2

    nop                                                 b           UV_SCROLL_RET
    nop                                                 nop









    
;==============================================================================
;    
; Diffuse Pass
;
;   Sets up the context registers for a diffuse pass, and kicks the already
;   transformed vertices
;
;==============================================================================

DIFFUSE:
    ; load up the registers with the appropriate settings
    nop                                                 iaddiu      VI08, vi00, 0x7FFF
    nop                                                 iaddiu      VI08, VI08, 1
    nop                                                 ior         VI10, VI08, VI02
    nop                                                 iaddiu      VI09, vi00, 5
    nop                                                 ilw.w       VI01, 0(VI05)
    nop                                                 lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )(vi00)
    nop                                                 lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 13 )(vi00)
    nop                                                 lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 14 )(vi00)
    nop                                                 lq.xyzw     VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 6 )(vi00)
    nop                                                 lq.xyzw     VF13xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 1 )(vi00)
    nop                                                 lq.xyzw     VF14xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 5 )(vi00)
    nop                                                 lq.yw       VF15yw,   ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 20 )(vi00)
    nop                                                 mfir.x      VF09x,    VI09                ; fill in the reg load giftag
    nop                                                 mfir.y      VF12y,    VI01                ; fill in the fixed alpha
    nop                                                 mfir.x      VF15x,    VI10                ; fill in the primitive giftag
    nop                                                 iaddiu      VI01, vi00, 0x512F        ; fill in the primitive giftag
    nop                                                 mfir.z      VF15z,    VI01                ; fill in the primitive giftag

    ; stall the previous pass
    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )             ; Wait for GS to finish
    nop                                                 xgkick      VI01

    ; render with the appropriate settings
    nop                                                 iaddiu      VI11, VI05, 1
    nop                                                 sq.xyzw     VF09xyzw, 0(VI11)
    nop                                                 sq.xyzw     VF10xyzw, 1(VI11)
    nop                                                 sq.xyzw     VF11xyzw, 2(VI11)
    nop                                                 sq.xyzw     VF12xyzw, 3(VI11)
    nop                                                 sq.xyzw     VF13xyzw, 4(VI11)
    nop                                                 sq.xyzw     VF14xyzw, 5(VI11)
    nop                                                 sq.xyzw     VF15xyzw, 6(VI11)
    nop                                                 nop
    nop                                                 nop
    nop                                                 b           DIFFUSE_RET
    nop                                                 xgkick      VI11

;==============================================================================
;
; Z-Priming
;
;   This routine renders the poly with the frame buffer masked out, but is
;   useful for priming the z-buffer (used for fading geometry).
;
;==============================================================================

ZPRIME:
    ; load up the registers with the appropriate settings
    nop                                                 iaddiu      VI08, vi00, 0x7FFF
    nop                                                 iaddiu      VI08, VI08, 1
    nop                                                 ior         VI10, VI08, VI02
    nop                                                 iaddiu      VI09, vi00, 5
    nop                                                 lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )(vi00)
    nop                                                 lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 13 )(vi00)
    nop                                                 lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 16 )(vi00)
    nop                                                 lq.xyzw     VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 6 )(vi00)
    nop                                                 lq.xyzw     VF13xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 13 )(vi00)
    nop                                                 lq.xyzw     VF14xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 5 )(vi00)
    nop                                                 lq.yw       VF15yw,   ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 22 )(vi00)
    nop                                                 mfir.x      VF09x,    VI09                ; fill in the reg load giftag
    nop                                                 mfir.x      VF15x,    VI10                ; fill in the primitive giftag
    nop                                                 iaddiu      VI01, vi00, 0x5FFF        ; fill in the primitive giftag
    nop                                                 mfir.z      VF15z,    VI01                ; fill in the primitive giftag

    ; stall the previous pass
    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )             ; Wait for GS to finish
    nop                                                 xgkick      VI01

    ; render with the appropriate settings
    nop                                                 iaddiu      VI11, VI05, 1
    nop                                                 sq.xyzw     VF09xyzw, 0(VI11)
    nop                                                 sq.xyzw     VF10xyzw, 1(VI11)
    nop                                                 sq.xyzw     VF11xyzw, 2(VI11)
    nop                                                 sq.xyzw     VF12xyzw, 3(VI11)
    nop                                                 sq.xyzw     VF13xyzw, 4(VI11)
    nop                                                 sq.xyzw     VF14xyzw, 5(VI11)
    nop                                                 sq.xyzw     VF15xyzw, 6(VI11)
    nop                                                 nop
    nop                                                 nop
    nop                                                 b           ZPRIME_RET
    nop                                                 xgkick      VI11

;==============================================================================
;
; Calculate Environment Map UV's From Screen-space Positions
;
;   This routine sets the uv1 slot to wherever the screen position is scaled
;   into [0..1]. An environment mapped texture has been precomputed so that
;   these UV's actually make sense. Wacky stuff... 
;
;==============================================================================

ENVIRONMENT_POSITIONS:









    ; we need to account for bilinear filtering by bringing in the uv's half a pixel, so...
    ; uv = (((pos-scissor)/scr_size)*63 + .5) / 64
    ;    = (pos-scissor)/(scr_size*64/63) + .5/64
    ;    = (pos-scissor)/(scr_size*64/63) + (.5*scr_size/63)/(scr_size*64/63)
    ;    = (pos-scissor+.5*scr_size/63)/(scr_size*64/63)
    ;  u = (pos-1792+.5*512/63) / (512*64/63)
    ;    = (pos-1792+256/63)    / (32768/63)
    ;    = (pos-1792+4.0635)    * 0.0019226
    ;    = (pos-1787.9365) * 0.0019226

    ; stall if uv1 is in use
    nop                                                 iaddiu      VI01, vi00, (1 << 14)
    nop                                                 iand        VI01, VI03, VI01
    nop                                                 nop
    nop                                                 ibeq        VI01, vi00, ENV_POSITION_DONT_STALL
    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )
    nop                                                 xgkick      VI01
ENV_POSITION_DONT_STALL:
    addw.zw         VF12zw,   vf00zw,     vf00w       loi         1787.9365
    addi.xy         VF13xy,   vf00xy,     i           loi         0.0019226
    addi.z          VF13z,    vf00z,      i           iadd        VI08, VI05, vi00
    nop                                                 iadd        VI09, VI08, VI02
    nop                                                 iadd        VI09, VI09, VI02
    nop                                                 iadd        VI09, VI09, VI02
    nop                                                 iadd        VI09, VI09, VI02

    ; loop preamble
    nop                                                 lq.xyzw     VF09xyzw, 11(VI08)                   ; load xyz0
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    itof4.xy        VF10xy,   VF09xy                nop                                                     ; xyz0->float
    nop                                                 lq.xyzw     VF09xyzw, 11+4*1(VI08)   ; load xyz1
    nop                                                 nop
    nop                                                 nop
    sub.xy          VF11xy,   VF10xy,   VF13xy    nop                                                     ; uv0-scissor
    itof4.xy        VF10xy,   VF09xy                nop                                                     ; xyz1->float
    nop                                                 lq.xyzw     VF09xyzw, 11+4*2(VI08)   ; load xyz2
    nop                                                 nop
    mulz.xy         VF12xy,   VF11xy,   VF13z     nop                                                     ; uv0/screen size
    sub.xy          VF11xy,   VF10xy,   VF13xy    nop                                                     ; uv1-scissor

    ; the main loop
ENVPOS_LOOP:
    itof4.xy        VF10xy,   VF09xy                iaddiu      VI08, VI08, 4                   ; xyz2->float
    nop                                                 lq.xyzw     VF09xyzw, 11+4*2(VI08)   ; load xyz3
    nop                                                 sq.xyz      VF12xyz,  8-4(VI08)     ; store uv0
    mulz.xy         VF12xy,   VF11xy,   VF13z     ibne        VI08, VI09, ENVPOS_LOOP                     ; uv1/screen size
    sub.xy          VF11xy,   VF10xy,   VF13xy    nop                                                     ; uv2-scissor

    nop                                                 b           ENVKICK
    nop                                                 nop









;==============================================================================
;
; Calculate Environment Map UV's From Normals
;
;   This routine just calculate environment-mapped uvs using a texture matrix.
;       a) load normal from backup buffer
;       b) convert normal to float
;       c) multiply normal by env. map matrix (3x2 matrix)
;       d) scale by 16 to finish fixed-to-float conversion
;       e) add .5 so it ranges from 0..1
;       f) load projected q from first uv
;       g) multiply (u,v,1) by projected q
;       h) store result
;       i) goto a
;==============================================================================

ENVIRONMENT_NORMALS:


















    ; stall if uv1 is in use
    nop                                                 iaddiu      VI01, vi00, (1 << 14)
    nop                                                 iand        VI01, VI03, VI01
    nop                                                 nop
    nop                                                 ibeq        VI01, vi00, ENV_NORMALS_DONT_STALL
    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )
    nop                                                 xgkick      VI01
ENV_NORMALS_DONT_STALL:
    
    addw.z          VF19z,    vf00z,      vf00w       lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 8 )(vi00)
    nop                                                 lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 9 )(vi00)
    nop                                                 loi         0.5
    addi.x          VF17x,    vf00x,      i           loi         16.0
    addi.y          VF17y,    vf00y,      i           mr32.xyzw   VF11xyzw, VF09xyzw
    nop                                                 iadd        VI08, VI06, vi00
    nop                                                 iadd        VI09, VI05, vi00
    nop                                                 iadd        VI10, VI09, VI02
    nop                                                 mr32.xyzw   VF11xyzw, VF11xyzw
    nop                                                 iadd        VI10, VI10, VI02
    mulax.xy        acc,        VF09xy,   VF07x     iadd        VI10, VI10, VI02
    madday.xy       acc,        VF10xy,   VF07y     iadd        VI10, VI10, VI02
    maddz.xy        VF14xy,   VF11xy,   VF07z     nop
    mulax.xy        acc,        VF09xy,   VF06x     nop
    madday.xy       acc,        VF10xy,   VF06y     nop
    maddz.xy        VF13xy,   VF11xy,   VF06z     lq.xyz      VF15xyz,  8(VI08)                                        ; load normal0
    mulax.xy        acc,        VF09xy,   VF05x     nop
    madday.xy       acc,        VF10xy,   VF05y     nop
    maddz.xy        VF12xy,   VF11xy,   VF05z     nop

    ; loop preamble
    itof12.xyz      VF18xyz,  VF15xyz               nop                                                         ; convert normal0

    nop                                                 nop
    nop                                                 nop
    nop                                                 lq.xyz      VF15xyz,  8+4(VI08)                          ; load normal1
    mulaz.xy        acc,        VF14xy,   VF18z     nop                                                         ; xform uv0
    madday.xy       acc,        VF13xy,   VF18y     nop                                                         ; xform uv0
    maddx.xy        VF16xy,   VF12xy,   VF18x     nop                                                         ; xform uv0
    itof12.xyz      VF18xyz,  VF15xyz               nop                                                         ; convert normal1

    nop                                                 nop
    addax.xy        acc,        vf00xy,     VF17x     nop                                                         ; offset uv0
    maddy.xy        VF19xy,   VF16xy,   VF17y     lq.xyz      VF15xyz,  (8+4*2)(VI08)  ; scale uv0         ; load normal2
    mulaz.xy        acc,        VF14xy,   VF18z     lq.z        VF21z,    9(VI09)                       ; xform uv1         ; load q0
    madday.xy       acc,        VF13xy,   VF18y     nop                                                         ; xform uv1
    maddx.xy        VF16xy,   VF12xy,   VF18x     nop                                                         ; xform uv1
    itof12.xyz      VF18xyz,  VF15xyz               nop                                                         ; convert normal2

    ; the main loop
ENVNRM_LOOP:
    mulz.xyz        VF20xyz,  VF19xyz,  VF21z     iaddiu      VI08, VI08, 4                       ; project uv0       ; pNxt++
    addax.xy        acc,        vf00xy,     VF17x     iaddiu      VI09, VI09, 4                       ; offset uv1        ; pOut++
    maddy.xy        VF19xy,   VF16xy,   VF17y     lq.xyz      VF15xyz,  (8+4*2)(VI08)  ; scale uv1         ; load normal3
    mulaz.xy        acc,        VF14xy,   VF18z     lq.z        VF21z,    9(VI09)                       ; xform uv2         ; load q1
    madday.xy       acc,        VF13xy,   VF18y     sq.xyz      VF20xyz,  (8-4)(VI09)       ; xform uv2         ; store stq0
    maddx.xy        VF16xy,   VF12xy,   VF18x     ibne        VI09, VI10, ENVNRM_LOOP                         ; xform uv2         ; loop
    itof12.xyz      VF18xyz,  VF15xyz               nop                                                         ; convert normal3

    nop                                                 b           ENVKICK
    nop                                                 nop



















;==============================================================================
;
;   Environment Map Pass
;
;==============================================================================

ENVKICK:
    ; load up the registers with the appropriate settings
    nop                                                 iaddiu      VI08, vi00, 0x7FFF
    nop                                                 iaddiu      VI08, VI08, 1
    nop                                                 ior         VI10, VI08, VI02
    nop                                                 iaddiu      VI09, vi00, 5
    nop                                                 lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )(vi00)
    nop                                                 lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 13 )(vi00)
    nop                                                 lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 15 )(vi00)
    nop                                                 lq.xyzw     VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 7 )(vi00)
    nop                                                 lq.xyzw     VF13xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 5 )(vi00)
    nop                                                 lq.xyzw     VF14xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 3 )(vi00)
    nop                                                 lq.yw       VF15yw,   ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 21 )(vi00)
    nop                                                 mfir.x      VF09x,    VI09                ; fill in the reg load giftag
    nop                                                 iaddiu      VI01, vi00, 0x121           ; turn off mipping
    nop                                                 mfir.x      VF13x,    VI01                ; turn off mipping
    nop                                                 iaddiu      VI01, vi00, 0x51F2        ; fill in the primitive giftag
    nop                                                 mfir.x      VF15x,    VI10                ; fill in the primitive giftag
    nop                                                 mfir.z      VF15z,    VI01                ; fill in the primitive giftag

    ; stall the previous pass
    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )             ; Wait for GS to finish
    nop                                                 xgkick      VI01

    ; render with the appropriate settings
    nop                                                 iaddiu      VI11, VI05, 1
    nop                                                 sq.xyzw     VF09xyzw, 0(VI11)
    nop                                                 sq.xyzw     VF10xyzw, 1(VI11)
    nop                                                 sq.xyzw     VF11xyzw, 2(VI11)
    nop                                                 sq.xyzw     VF12xyzw, 3(VI11)
    nop                                                 sq.xyzw     VF13xyzw, 4(VI11)
    nop                                                 sq.xyzw     VF14xyzw, 5(VI11)
    nop                                                 sq.xyzw     VF15xyzw, 6(VI11)
    nop                                                 iaddiu      VI01, vi00, (1 << 14)
    nop                                                 ior         VI03, VI03, VI01
    nop                                                 b           ENV_MAP_RET
    nop                                                 xgkick      VI11

;==============================================================================
;
;   Backup Second Set of UVs to Next Buffer
;
;==============================================================================

BACKUP_UV1:






    nop                                                 iadd        VI08, VI06, vi00
    nop                                                 iadd        VI09, VI05, vi00
    nop                                                 iadd        VI10, VI09, VI02
    nop                                                 iadd        VI10, VI10, VI02
    nop                                                 iadd        VI10, VI10, VI02
    nop                                                 iadd        VI10, VI10, VI02

BACKUP_UV1_LOOP:
    nop                                                 lq          VF09xyzw, 8(VI09)
    nop                                                 iaddiu      VI09, VI09, 4
    nop                                                 iaddiu      VI08, VI08, 4
    nop                                                 ibne        VI09, VI10, BACKUP_UV1_LOOP
    nop                                                 sq          VF09xyzw, 8-4(VI08)

    nop                                                 b           BACKUP_RET
    nop                                                 nop





    
;==============================================================================
;
; Detail Map Pass
;
;   Scales the screen-space by uv's by a detail scale, stores them back, then
;   renders the detail map pass.
;
;==============================================================================

DETAIL:








    nop                                                 lq.z        VF09z,    ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 0 )(vi00)             ; Get Detail Scale
    nop                                                 iadd        VI08, VI05, vi00                        ; Get address of current buffer
    nop                                                 iadd        VI09, VI08, VI02
    nop                                                 iadd        VI09, VI09, VI02
    nop                                                 iadd        VI09, VI09, VI02
    nop                                                 iadd        VI09, VI09, VI02 

    ; loop preamble
    nop                                                 lq.xyz      VF10xyz,  9(VI08)               ; load uv 0
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    mulz.xy         VF11xy,   VF10xy,   VF09z     nop                                                 ; scale uv 0
    add.z           VF11z,    VF10z,    vf00z       nop                                                 ; project uv 0


DETAIL_LOOP:
    nop                                                 lq.xyz      VF10xyz,  (9+4)(VI08)   ; load uv 1
    nop                                                 iaddiu      VI08, VI08, 4
    nop                                                 nop
    nop                                                 sq.xyz      VF11xyz,  (8-4)(VI08)   ; store uv 0
    mulz.xy         VF11xy,   VF10xy,   VF09z     ibne        VI08, VI09, DETAIL_LOOP                     ; scale uv 1
    add.z           VF11z,    VF10z,    vf00z       nop                                                     ; project uv 1

    ; load up the registers with the appropriate settings
    nop                                                 iaddiu      VI08, vi00, 0x7FFF
    nop                                                 iaddiu      VI08, VI08, 1
    nop                                                 ior         VI10, VI08, VI02
    nop                                                 iaddiu      VI09, vi00, 5
    nop                                                 lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )(vi00)
    nop                                                 lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 13 )(vi00)
    nop                                                 lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 15 )(vi00)
    nop                                                 lq.xyzw     VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 18 )(vi00)
    nop                                                 lq.xyzw     VF13xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 4 )(vi00)
    nop                                                 lq.xyzw     VF14xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 5 )(vi00)
    nop                                                 lq.yw       VF15yw,   ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 21 )(vi00)
    nop                                                 mfir.x      VF09x,    VI09                ; fill in the reg load giftag
    nop                                                 move.xy     VF10xy,   vf00xy              ; turn clamping off
    nop                                                 iaddiu      VI01, vi00, 0x51F2        ; fill in the primitive giftag
    nop                                                 mfir.x      VF15x,    VI10                ; fill in the primitive giftag
    nop                                                 mfir.z      VF15z,    VI01                ; fill in the primitive giftag

    ; stall the previous pass
    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )     ; Wait for GS to finish
    nop                                                 xgkick      VI01

    ; render with the appropriate settings
    nop                                                 iaddiu      VI11, VI05, 1
    nop                                                 sq.xyzw     VF09xyzw, 0(VI11)
    nop                                                 sq.xyzw     VF10xyzw, 1(VI11)
    nop                                                 sq.xyzw     VF11xyzw, 2(VI11)
    nop                                                 sq.xyzw     VF12xyzw, 3(VI11)
    nop                                                 sq.xyzw     VF13xyzw, 4(VI11)
    nop                                                 sq.xyzw     VF14xyzw, 5(VI11)
    nop                                                 sq.xyzw     VF15xyzw, 6(VI11)
    nop                                                 iaddiu      VI01, vi00, (1 << 14)
    nop                                                 ior         VI03, VI03, VI01
    nop                                                 b           DETAIL_RET
    nop                                                 xgkick      VI11









;==============================================================================
;
; Self Illumination Pass
;
;   This routine a) copies screen uv's from uv0 to uv1
;                b) renders the diffuse texture again in decal mode using the
;                   destination alpha to determine which parts are illuminated
;                c) re-renders the self-illumination to the front-buffer alpha
;                d) resets the frame buffer register             
;
;==============================================================================

SELFILLUM:

    ; if we're using the diffuse lighting, but just doing a glow, skip the
    ; self-illum portion and go straight to the glow
    nop                                                 iaddiu      VI01, vi00, (1 << 10)
    nop                                                 iand        VI01, VI03, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, SKIP_SELFILLUM_DECAL

    ; if we're fading the geometry out, skip the self-illum portion and go straight to the glow
    nop                                                 iaddiu      VI01, vi00, 0x1000
    nop                                                 iand        VI01, VI04, VI01
    nop                                                 nop
    nop                                                 ibne        VI01, vi00, SKIP_SELFILLUM_DECAL

    ; load up the registers with the appropriate settings
    nop                                                 iaddiu      VI08, vi00, 0x7FFF
    nop                                                 iaddiu      VI08, VI08, 1
    nop                                                 ior         VI10, VI08, VI02
    nop                                                 iaddiu      VI09, vi00, 5
    nop                                                 lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )(vi00)
    nop                                                 lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 13 )(vi00)
    nop                                                 lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 14 )(vi00)
    nop                                                 lq.xyzw     VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 7 )(vi00)
    nop                                                 lq.xyzw     VF13xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 2 )(vi00)
    nop                                                 lq.xyzw     VF14xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 5 )(vi00)
    nop                                                 lq.yw       VF15yw,   ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 20 )(vi00)
    nop                                                 mfir.x      VF09x,    VI09                ; fill in the reg load giftag
    nop                                                 iaddiu      VI01, vi00, 0x512F        ; fill in the primitive giftag
    nop                                                 mfir.x      VF15x,    VI10                ; fill in the primitive giftag
    nop                                                 mfir.z      VF15z,    VI01                ; fill in the primitive giftag

    ; stall the previous pass
    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )             ; Wait for GS to finish
    nop                                                 xgkick      VI01

    ; render with the appropriate settings
    nop                                                 iaddiu      VI11, VI05, 1
    nop                                                 sq.xyzw     VF09xyzw, 0(VI11)
    nop                                                 sq.xyzw     VF10xyzw, 1(VI11)
    nop                                                 sq.xyzw     VF11xyzw, 2(VI11)
    nop                                                 sq.xyzw     VF12xyzw, 3(VI11)
    nop                                                 sq.xyzw     VF13xyzw, 4(VI11)
    nop                                                 sq.xyzw     VF14xyzw, 5(VI11)
    nop                                                 sq.xyzw     VF15xyzw, 6(VI11)
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 xgkick      VI11
    
SKIP_SELFILLUM_DECAL:
    ; now if we are rendering with a per-poly glow, we need to set that alpha value everywhere,
    ; otherwise, we just render into the front buffer alpha

    nop                                                 iaddiu      VI01, vi00, (1 << 11)
    nop                                                 iand        VI01, VI03, VI01
    nop                                                 nop
    nop                                                 ibeq        VI01, vi00, DO_GLOW_PERPIXEL
    





    nop                                                 ilw.w       VI10, 0(VI05)
    nop                                                 lq.w        VF10w, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 0 )(vi00)
    nop                                                 iadd        VI08, VI05, vi00
    nop                                                 iaddiu      VI01, vi00, 0xff
    nop                                                 iand        VI10, VI10, VI01
    nop                                                 mfir.w      VF09w, VI10
    nop                                                 iadd        VI09, VI08, VI02
    nop                                                 iadd        VI09, VI09, VI02
    itof0.w         VF10w,    VF10w                 iadd        VI09, VI09, VI02
    itof0.w         VF09w,    VF09w                 iadd        VI09, VI09, VI02
    nop                                                 loi         0.0078125   ; 1/128
    nop                                                 nop
    nop                                                 nop
    mul.w           VF09w,    VF09w,    VF10w     nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    muli.w          VF09w,    VF09w,    i           nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    ftoi0.w         VF09w,    VF09w                 nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
      
PERPOLY_GLOW_LOOP:
    nop                                                 sq.w        VF09w, 9(VI08)
    nop                                                 iaddiu      VI08, VI08, 4
    nop                                                 nop
    nop                                                 ibne        VI08, VI09, PERPOLY_GLOW_LOOP
    nop                                                 nop






    ; load up the registers with the glow settings
    nop                                                 iaddiu      VI08, vi00, 0x7FFF
    nop                                                 iaddiu      VI08, VI08, 1
    nop                                                 ior         VI10, VI08, VI02
    nop                                                 iaddiu      VI09, vi00, 5
    nop                                                 lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )(vi00)
    nop                                                 lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 13 )(vi00)
    nop                                                 lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 17 )(vi00)
    nop                                                 lq.xyzw     VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 6 )(vi00)
    nop                                                 lq.xyzw     VF13xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 1 )(vi00)
    nop                                                 lq.xyzw     VF14xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 5 )(vi00)
    nop                                                 lq.yw       VF15yw,   ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 22 )(vi00)
    nop                                                 mfir.x      VF09x,    VI09                ; fill in the reg load giftag
    nop                                                 iaddiu      VI01, vi00, 0x4D          ; use context 2 frame
    nop                                                 mfir.z      VF11z,    VI01                ; use context 2 frame
    nop                                                 iaddiu      VI01, vi00, 0x5F1F        ; fill in the primitive giftag
    nop                                                 mfir.x      VF15x,    VI10                ; fill in the primitive giftag
    nop                                                 mfir.z      VF15z,    VI01                ; fill in the primitive giftag

    ; stall the previous pass
    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )             ; Wait for GS to finish
    nop                                                 xgkick      VI01

    ; render with the appropriate settings
    nop                                                 iaddiu      VI11, VI05, 1
    nop                                                 sq.xyzw     VF09xyzw, 0(VI11)
    nop                                                 sq.xyzw     VF10xyzw, 1(VI11)
    nop                                                 sq.xyzw     VF11xyzw, 2(VI11)
    nop                                                 sq.xyzw     VF12xyzw, 3(VI11)
    nop                                                 sq.xyzw     VF13xyzw, 4(VI11)
    nop                                                 sq.xyzw     VF14xyzw, 5(VI11)
    nop                                                 sq.xyzw     VF15xyzw, 6(VI11)
    nop                                                 iaddiu      VI01, vi00, ~(1 << 14)
    nop                                                 iand        VI03, VI03, VI01
    nop                                                 b           SELFILLUM_RET
    nop                                                 xgkick      VI11
    
DO_GLOW_PERPIXEL:
    ; load up the registers with the glow settings
    nop                                                 iaddiu      VI08, vi00, 0x7FFF
    nop                                                 iaddiu      VI08, VI08, 1
    nop                                                 ior         VI10, VI08, VI02
    nop                                                 iaddiu      VI09, vi00, 5
    nop                                                 lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )(vi00)
    nop                                                 lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 13 )(vi00)
    nop                                                 lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 17 )(vi00)
    nop                                                 lq.xyzw     VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 6 )(vi00)
    nop                                                 lq.xyzw     VF13xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 1 )(vi00)
    nop                                                 lq.xyzw     VF14xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 5 )(vi00)
    nop                                                 lq.yw       VF15yw,   ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 20 )(vi00)
    nop                                                 mfir.x      VF09x,    VI09                ; fill in the reg load giftag
    nop                                                 iaddiu      VI01, vi00, 0x512F        ; fill in the primitive giftag
    nop                                                 mfir.x      VF15x,    VI10                ; fill in the primitive giftag
    nop                                                 mfir.z      VF15z,    VI01                ; fill in the primitive giftag

    ; stall the previous pass
    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )             ; Wait for GS to finish
    nop                                                 xgkick      VI01

    ; render with the appropriate settings
    nop                                                 iaddiu      VI11, VI05, 1
    nop                                                 sq.xyzw     VF09xyzw, 0(VI11)
    nop                                                 sq.xyzw     VF10xyzw, 1(VI11)
    nop                                                 sq.xyzw     VF11xyzw, 2(VI11)
    nop                                                 sq.xyzw     VF12xyzw, 3(VI11)
    nop                                                 sq.xyzw     VF13xyzw, 4(VI11)
    nop                                                 sq.xyzw     VF14xyzw, 5(VI11)
    nop                                                 sq.xyzw     VF15xyzw, 6(VI11)
    nop                                                 iaddiu      VI01, vi00, ~(1 << 14)
    nop                                                 iand        VI03, VI03, VI01
    nop                                                 b           SELFILLUM_RET
    nop                                                 xgkick      VI11

;==============================================================================
;
;   Character shadow passes - comes from the alpha channel of the z-buffer
;
;==============================================================================

SHADOW:







    addw.z          VF12z,    vf00z,      vf00w       loi         3.5         ; 1792/512
    addi.xy         VF09xy,   vf00xy,     i           loi         0.001953125 ; 1/512
    addi.z          VF09z,    vf00z,      i           iadd        VI08, VI05, vi00
    nop                                                 iadd        VI09, VI08, VI02
    nop                                                 iadd        VI09, VI09, VI02
    nop                                                 iadd        VI09, VI09, VI02
    nop                                                 iadd        VI09, VI09, VI02

    ; stall if uv1 is in use
    nop                                                 iaddiu      VI01, vi00, (1 << 14)
    nop                                                 iand        VI01, VI03, VI01
    nop                                                 nop
    nop                                                 ibeq        VI01, vi00, SHADOW_DONT_STALL
    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )
    nop                                                 xgkick      VI01
SHADOW_DONT_STALL:

    ; loop preamble
    nop                                                 lq.xy       VF10xy,   11(VI08)                   ;               ; load pos0
    nop                                                 nop                                                     ;               ;
    nop                                                 nop                                                     ;               ;

    nop                                                 nop                                                     ;
    itof4.xy        VF11xy,   VF10xy                nop                                                     ; pos0->float
    nop                                                 lq.xy       VF10xy,   (11+4)(VI08)   ;               ; load pos1
    nop                                                 nop                                                     ;               ;
    suba.xy         acc,        vf00xy,     VF09xy    nop                                                     ; scissor uv0   ;

SHADOW_LOOP:
    maddz.xy        VF12xy,   VF11xy,   VF09z     iaddiu      VI08, VI08, 4                   ; scale uv0     ; vptr++
    itof4.xy        VF11xy,   VF10xy                nop                                                     ; pos1->float
    nop                                                 lq.xy       VF10xy,   (11+4)(VI08)   ;               ; load pos2
    nop                                                 ibne        VI08, VI09, SHADOW_LOOP                     ;               ; loop
    suba.xy         acc,        vf00xy,     VF09xy    sq.xyz      VF12xyz,  (8-4)(VI08)   ; scissor uv1   ; store uv0








    ; load up the registers with the appropriate settings
    nop                                                 iaddiu      VI08, vi00, 0x7FFF
    nop                                                 iaddiu      VI08, VI08, 1
    nop                                                 ior         VI10, VI08, VI02
    nop                                                 iaddiu      VI09, vi00, 5
    nop                                                 lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )(vi00)
    nop                                                 lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 13 )(vi00)
    nop                                                 lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 15 )(vi00)
    nop                                                 lq.xyzw     VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 18 )(vi00)
    nop                                                 lq.xyzw     VF13xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 11 )(vi00)
    nop                                                 lq.xyzw     VF14xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 5 )(vi00)
    nop                                                 lq.yw       VF15yw,   ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 21 )(vi00)
    nop                                                 mfir.x      VF09x,    VI09                ; fill in the reg load giftag
    nop                                                 iaddiu      VI01, vi00, 0x121           ; turn mipping off
    nop                                                 mfir.x      VF14x,    VI01                ; turn mipping off
    nop                                                 iaddiu      VI01, vi00, 0x51F2        ; fill in the primitive giftag
    nop                                                 mfir.x      VF15x,    VI10                ; fill in the primitive giftag
    nop                                                 mfir.z      VF15z,    VI01                ; fill in the primitive giftag

    ; stall the previous pass
    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )             ; Wait for GS to finish
    nop                                                 xgkick      VI01

    ; render with the appropriate settings
    nop                                                 iaddiu      VI11, VI05, 1
    nop                                                 sq.xyzw     VF09xyzw, 0(VI11)
    nop                                                 sq.xyzw     VF10xyzw, 1(VI11)
    nop                                                 sq.xyzw     VF11xyzw, 2(VI11)
    nop                                                 sq.xyzw     VF12xyzw, 3(VI11)
    nop                                                 sq.xyzw     VF13xyzw, 4(VI11)
    nop                                                 sq.xyzw     VF14xyzw, 5(VI11)
    nop                                                 sq.xyzw     VF15xyzw, 6(VI11)
    nop                                                 iaddiu      VI01, vi00, (1 << 14)
    nop                                                 ior         VI03, VI03, VI01
    nop                                                 b           SHADOW_RET
    nop                                                 xgkick      VI11

;==============================================================================
;
;   Projected Shadow Passes - comes from artist-specified textures
;
;==============================================================================















    
PROJ_SHADOW_1:
    nop                                                 lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 30 )(vi00)
    nop                                                 lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 31 )(vi00)
    nop                                                 lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 32 )(vi00)
    nop                                                 lq.xyzw     VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 33 )(vi00)
    nop                                                 bal         VI15, DO_PROJ_SHADOW
    nop                                                 lq.xyzw     VF31xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 28 )(vi00)
    nop                                                 b           PROJ_SHADOW_1_RET
    nop                                                 nop

PROJ_SHADOW_2:    
    nop                                                 lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 34 )(vi00)
    nop                                                 lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 35 )(vi00)
    nop                                                 lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 36 )(vi00)
    nop                                                 lq.xyzw     VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 37 )(vi00)
    nop                                                 bal         VI15, DO_PROJ_SHADOW
    nop                                                 lq.xyzw     VF31xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 29 )(vi00)
    nop                                                 b           PROJ_SHADOW_2_RET
    nop                                                 nop
    
DO_PROJ_SHADOW:
    ; concatenate the projection and l2w matrices while simultaneoustly
    ; setting up our buffer pointers and doing a conditional sync stall
    mulaw.xyzw      acc,        VF12xyzw, VF05w     iadd        VI08, VI06, vi00
    maddaz.xyzw     acc,        VF11xyzw, VF05z     iadd        VI09, VI05, vi00
    madday.xyzw     acc,        VF10xyzw, VF05y     iadd        VI10, VI09, VI02
    maddx.xyzw      VF13xyzw, VF09xyzw, VF05x     iadd        VI10, VI10, VI02
    mulaw.xyzw      acc,        VF12xyzw, VF06w     iadd        VI10, VI10, VI02
    maddaz.xyzw     acc,        VF11xyzw, VF06z     iadd        VI10, VI10, VI02
    madday.xyzw     acc,        VF10xyzw, VF06y     nop
    maddx.xyzw      VF14xyzw, VF09xyzw, VF06x     nop
    mulaw.xyzw      acc,        VF12xyzw, VF07w     nop
    maddaz.xyzw     acc,        VF11xyzw, VF07z     nop
    madday.xyzw     acc,        VF10xyzw, VF07y     nop
    maddx.xyzw      VF15xyzw, VF09xyzw, VF07x     iaddiu      VI01, vi00, (1 << 14)
    mulaw.xyzw      acc,        VF12xyzw, VF08w     iand        VI01, VI03, VI01
    maddaz.xyzw     acc,        VF11xyzw, VF08z     nop
    madday.xyzw     acc,        VF10xyzw, VF08y     ibeq        VI01, vi00, PROJ_SHADOW_DONT_STALL
    maddx.xyzw      VF16xyzw, VF09xyzw, VF08x     iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )
    nop                                                 xgkick      VI01
PROJ_SHADOW_DONT_STALL:

    ; unoptimized loop (nops indicate where stalls would occur)
PROJ_SHADOW_LOOP:
    nop                                                 lq.xyz      VF17xyz,  11(VI08)           ; load vert in local space
    nop                                                 lq.z        VF18z,    9(VI09)           ; load Q from screen space data
    nop                                                 nop
    mulaw.xyw       acc,        VF16xyw,  vf00w       nop                                             ; vert * proj matrix
    maddaz.xyw      acc,        VF15xyw,  VF17z     nop                                             ; vert * proj matrix
    madday.xyw      acc,        VF14xyw,  VF17y     nop                                             ; vert * proj matrix
    maddx.xyw       VF18xyw,  VF13xyw,  VF17x     nop                                             ; vert * proj matrix
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    mulz.xy         VF18xy,   VF18xy,   VF18z     nop                                             ; pre-mult xy by original Q
    mulw.z          VF18z,    VF18z,    VF18w     nop                                             ; q = original q times projected q
    nop                                                 iaddiu      VI09, VI09, 4           ; pOut++
    nop                                                 iaddiu      VI08, VI08, 4           ; pNxt++
    nop                                                 ibne        VI09, VI10, PROJ_SHADOW_LOOP        ; loop
    nop                                                 sq.xyz      VF18xyz,  8-4(VI09) ; store result

    ; load up the registers with the appropriate settings
    nop                                                 iaddiu      VI08, vi00, 0x7FFF
    nop                                                 iaddiu      VI08, VI08, 1
    nop                                                 ior         VI10, VI08, VI02
    nop                                                 iaddiu      VI09, vi00, 5
    nop                                                 lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )(vi00)
    nop                                                 lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 13 )(vi00)
    nop                                                 lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 15 )(vi00)
    nop                                                 lq.xyzw     VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 18 )(vi00)
    nop                                                 lq.xyzw     VF14xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 5 )(vi00)
    nop                                                 lq.yw       VF15yw,   ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 21 )(vi00)
    nop                                                 mfir.x      VF09x,    VI09                ; fill in the reg load giftag
    nop                                                 iaddiu      VI01, vi00, 0x121           ; turn mipping off
    nop                                                 mfir.x      VF14x,    VI01                ; turn mipping off
    nop                                                 iaddiu      VI01, vi00, 0x51F2        ; fill in the primitive giftag
    nop                                                 mfir.x      VF15x,    VI10                ; fill in the primitive giftag
    nop                                                 mfir.z      VF15z,    VI01                ; fill in the primitive giftag

    ; stall the previous pass
    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )             ; Wait for GS to finish
    nop                                                 xgkick      VI01

    ; render with the appropriate settings    
    nop                                                 iaddiu      VI11, VI05, 1
    nop                                                 sq.xyzw     VF09xyzw, 0(VI11)
    nop                                                 sq.xyzw     VF10xyzw, 1(VI11)
    nop                                                 sq.xyzw     VF11xyzw, 2(VI11)
    nop                                                 sq.xyzw     VF12xyzw, 3(VI11)
    nop                                                 sq.xyzw     VF31xyzw, 4(VI11)
    nop                                                 sq.xyzw     VF14xyzw, 5(VI11)
    nop                                                 sq.xyzw     VF15xyzw, 6(VI11)
    nop                                                 iaddiu      VI01, vi00, (1 << 14)
    nop                                                 ior         VI03, VI03, VI01
    nop                                                 jr          VI15
    nop                                                 xgkick      VI11

















;==============================================================================
;
;   Projected Spotlight Pass
;
;   The math behind the spotlight will (a) transform and project the vertex
;   position into the projector's space, and (b) calculate an alpha value based
;   on distance and incident angle.
;
;   The microcode is performing this code (in an unoptimized form):
;       a) Transform vert (Note that after transform Q is stored in W, and Z
;          is a parametric value between the projector's near and far planes.
;          This is different from a traditional projection matrix, but makes
;          the microcode happy.)
;       b) The transformed vert becomes our UVs. Set up Q such that the
;          hardware will handle perspective-correction properly. You can find
;          this math in some papers on the net, but it boils down to:
;          st = xy*original q
;          q  = original q * transformed q
;       c) Clamp the distance to avoid negatives.
;       d) Convert the normal to float
;       e) Calculate Dir.Dot( Normal ). (Dir still isn't normalized at this point!)
;       f) Clamp Dot to zero to avoid negatives
;       g) Scale Dot by 32*128. (32 to finish off the float conversion and 128
;          is the alpha range)
;       h) Multiply dot by 1/length(dir) and add ambient of 64
;       i) Scale that intensity by the distance (from step (a))
;       j) Convert intensity to fixed-point
;       k) move intensity from x to w
;       l) store results
;
;   In microcode, the above stuff gets really really confusing (optimized or unoptimized).
;   Your best bet is to lay it out in Excel and color code the registers being
;   used.
;
;==============================================================================

SPOTLIGHT:












































    addw.xyz        VF14xyz,  vf00xyz,    vf00w       lq.xyzw     VF13xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 23 )(vi00)         ; load up constant      ; load spotlight pos
    addw.xyz        VF26xyz,  vf00xyz,    vf00w       lq.xyzw     VF22xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 27 )(vi00)        ; load up constant      ; load proj matrix
    nop                                                 lq.xyzw     VF21xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 26 )(vi00)        ;                       ; load proj matrix
    nop                                                 lq.xyzw     VF20xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 25 )(vi00)        ;                       ; load proj matrix
    sub.xyz         VF13xyz,  VF13xyz,  VF08xyz   lq.xyzw     VF19xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 24 )(vi00)        ; spos->local-space     ; load proj matrix
    mulaw.xyzw      acc,        VF22xyzw, VF08w     iadd        VI08, VI06, vi00                            ; proj*l2w              ; setup next ptr
    maddaz.xyzw     acc,        VF21xyzw, VF08z     iadd        VI09, VI05, vi00                            ; proj*l2w              ; setup vptr
    madday.xyzw     acc,        VF20xyzw, VF08y     iadd        VI10, VI09, VI02                            ; proj*l2w              ; setup end marker
    maddx.xyzw      VF12xyzw, VF19xyzw, VF08x     iadd        VI10, VI10, VI02                            ; proj*l2w              ; setup end marker
    mulaw.xyzw      acc,        VF22xyzw, VF07w     iadd        VI10, VI10, VI02                            ; proj*l2w              ; setup end marker
    maddaz.xyzw     acc,        VF21xyzw, VF07z     iadd        VI10, VI10, VI02                            ; proj*l2w              ; setup end marker
    madday.xyzw     acc,        VF20xyzw, VF07y     nop                                                     ; proj*l2w
    maddx.xyzw      VF11xyzw, VF19xyzw, VF07x     nop                                                     ; proj*l2w
    mulaw.xyzw      acc,        VF22xyzw, VF06w     nop                                                     ; proj*l2w
    maddaz.xyzw     acc,        VF21xyzw, VF06z     nop                                                     ; proj*l2w
    madday.xyzw     acc,        VF20xyzw, VF06y     nop                                                     ; proj*l2w
    maddx.xyzw      VF10xyzw, VF19xyzw, VF06x     nop                                                     ; proj*l2w
    mulaw.xyzw      acc,        VF22xyzw, VF05w     nop                                                     ; proj*l2w
    maddaz.xyzw     acc,        VF21xyzw, VF05z     nop                                                     ; proj*l2w
    madday.xyzw     acc,        VF20xyzw, VF05y     nop                                                     ; proj*l2w
    maddx.xyzw      VF09xyzw, VF19xyzw, VF05x     nop                                                     ; proj*l2w
    mul.xyz         VF23xyz,  VF05xyz,  VF13xyz   nop                                                     ; spos->local-space
    mul.xyz         VF24xyz,  VF06xyz,  VF13xyz   nop                                                     ; spos->local-space
    mul.xyz         VF25xyz,  VF07xyz,  VF13xyz   loi         64.0                                        ; spos->local-space     ; i = ambient
    addi.y          VF14y,    vf00y,      i           loi         4096.0                                      ; cons.y = ambient      ; i = scale
    addi.z          VF14z,    vf00z,      i           nop                                                     ; cons.z = scale
    addaz.x         acc,        VF23x,    VF23z     nop                                                     ; spos->local-space
    maddy.x         VF13x,    VF26x,    VF23y     nop                                                     ; spos->local-space
    addaz.y         acc,        VF24y,    VF24z     nop                                                     ; spos->local-space
    maddx.y         VF13y,    VF26y,    VF24x     nop                                                     ; spos->local-space
    adday.z         acc,        VF25z,    VF25y     nop                                                     ; spos->local-space
    maddx.z         VF13z,    VF26z,    VF25x     nop                                                     ; spos->local-space

    ; loop preamble
    nop                                                 lq.xyzw     VF17xyzw, 11(VI08)                       ;                       ; load pos 0
    nop                                                 nop                                                         ;                       ;
    nop                                                 nop                                                         ;                       ;
    nop                                                 nop                                                         ;                       ;
    sub.xyz         VF23xyz,  VF13xyz,  VF17xyz   nop                                                         ; calc dir 0            ;
    nop                                                 nop                                                         ;                       ;
    nop                                                 nop                                                         ;                       ;
    nop                                                 nop                                                         ;                       ;
    nop                                                 eleng       p,          VF23                                ;                       ; start length 0
    mulaw.xyzw      acc,        VF12xyzw, vf00w       nop                                                         ; xform vert 0          ;
    maddaz.xyzw     acc,        VF11xyzw, VF17z     lq.xyzw     VF15xyzw, 8(VI09)                    ; xform vert 0          ; load normal 0
    nop                                                 nop                                                         ;                       ;
    nop                                                 nop                                                         ;                       ;
    madday.xyzw     acc,        VF10xyzw, VF17y     nop                                                         ; xform vert 0          ;
    itof12.xyz      VF25xyz,  VF15xyz               move.xyzw   VF21xyzw, VF15xyzw                          ; normal->float 0       ; first copy normal 0
    maddx.xyzw      VF18xyzw, VF09xyzw, VF17x     nop                                                         ; xform vert 0          ;
    nop                                                 lq.z        VF15z,    9(VI09)                       ;                       ; load orig q 0
    nop                                                 lq.xyzw     VF17xyzw, 11+(4*1)(VI08)     ;                       ; load pos 1
    mul.xyz         VF25xyz,  VF25xyz,  VF23xyz   nop                                                         ; dot*normal 0          ;
    nop                                                 nop                                                         ;                       ;

    nop                                                 nop                                                         ;                       ;
    nop                                                 move.xyzw   VF19xyzw, VF18xyzw                          ;                       ; copy trans vert 0
    sub.xyz         VF23xyz,  VF13xyz,  VF17xyz   nop                                                         ; calc dir 1            ;
    mulz.xy         VF20xy,   VF18xy,   VF15z     nop                                                         ; project stq 0         ;
    addaz.x         acc,        VF25x,    VF25z     nop                                                         ; add dot comp. 0       ;
    nop                                                 move.xyzw   VF16xyzw, VF15xyzw                          ;                       ; copy orig q 0
    maddy.x         VF25x,    VF14x,    VF25y     eleng       p,          VF23                                ; add dot comp. 0       ; start length 1
    mulaw.xyzw      acc,        VF12xyzw, vf00w       mfp.w       VF18w,    p                                   ; xform vert 1          ; get length 0
    maddaz.xyzw     acc,        VF11xyzw, VF17z     lq.xyzw     VF15xyzw, 8+(4*1)(VI09)  ; xform vert 1          ; load normal 1
    nop                                                 nop                                                         ;                       ;
    maxx.x          VF26x,    VF25x,    vf00x       move.xyzw   VF22xzyw, VF21xyzw                          ; clamp dot 0           ; second copy normal 0
    madday.xyzw     acc,        VF10xyzw, VF17y     div         q, vf00w,   VF18w                             ; xform vert 1          ; 1/length 0
    itof12.xyz      VF25xyz,  VF15xyz               move.xyzw   VF21xyzw, VF15xyzw                          ; normal->float 1       ; first copy normal 1
    maddx.xyzw      VF18xyzw, VF09xyzw, VF17x     nop                                                         ; xform vert 1          ;
    mulz.x          VF26x,    VF26x,    VF14z     lq.z        VF15z,    9+(4*1)(VI09)     ; scale int 0           ; load orig q 1
    mulw.z          VF20z,    VF16z,    VF19w     lq.xyzw     VF17xyzw, 11+(4*2)(VI08)     ; project q 0           ; load pos 2
    mul.xyz         VF25xyz,  VF25xyz,  VF23xyz   nop                                                         ; dot*normal 1          ;
    maxx.z          VF24z,    VF19z,    vf00x       nop                                                         ; clamp dist 0          ;
    
SPOTLIGHT_LOOP:
    mulaq.x         acc,        VF26x,    q           nop                                                         ; normalize int 0       ;
    maddy.x         VF26x,    VF14x,    VF14y     move.xyzw   VF19xyzw, VF18xyzw                          ; add ambient 0         ; copy trans vert 1
    sub.xyz         VF23xyz,  VF13xyz,  VF17xyz   sq.xyz      VF20xyz,  8(VI09)                       ; calc dir 2            ; store stq 0
    mulz.xy         VF20xy,   VF18xy,   VF15z     iaddiu      VI09, VI09, 4                       ; project stq 1         ; sout++
    addaz.x         acc,        VF25x,    VF25z     iaddiu      VI08, VI08, 4                       ; add dot comp. 1       ; snxt++
    mulz.x          VF26x,    VF26x,    VF24z     move.xyzw   VF16xyzw, VF15xyzw                          ; int*dist 0            ; copy orig q 1
    maddy.x         VF25x,    VF14x,    VF25y     eleng       p,          VF23                                ; add dot comp. 1       ; start length 2
    mulaw.xyzw      acc,        VF12xyzw, vf00w       mfp.w       VF18w,    p                                   ; xform vert 2          ; get length 1
    maddaz.xyzw     acc,        VF11xyzw, VF17z     lq.xyzw     VF15xyzw, 8+(4*1)(VI09)  ; xform vert 2          ; load normal 2
    ftoi0.x         VF27x,    VF26x                 sq.xyzw     VF22xyzw, 8-(4*1)(VI08)  ; int->fixed 0          ; backup normal 0
    maxx.x          VF26x,    VF25x,    vf00x       move.xyzw   VF22xzyw, VF21xyzw                          ; clamp dot 1           ; second copy normal 1
    madday.xyzw     acc,        VF10xyzw, VF17y     div         q, vf00w,   VF18w                             ; xform vert 2          ; 1/length 1
    itof12.xyz      VF25xyz,  VF15xyz               move.xyzw   VF21xyzw, VF15xyzw                          ; normal->float 2       ; first copy normal 2
    maddx.xyzw      VF18xyzw, VF09xyzw, VF17x     mr32.xyzw   VF27xyzw, VF27xyzw                          ; xform vert 2          ; int x->w 0
    mulz.x          VF26x,    VF26x,    VF14z     lq.z        VF15z,    9+(4*1)(VI09)     ; scale int 1           ; load orig q 2
    mulw.z          VF20z,    VF16z,    VF19w     lq.xyzw     VF17xyzw, 11+(4*2)(VI08)     ; project q 1           ; load pos 3
    mul.xyz         VF25xyz,  VF25xyz,  VF23xyz   ibne        VI09, VI10, SPOTLIGHT_LOOP                      ; dot*normal 2          ; loop
    maxx.z          VF24z,    VF19z,    vf00x       sq.w        VF27w,    9-(4*1)(VI09)     ; clamp dist 1          ; store int 0








































    ;--------------------------------------------------------------------------
    ; kick the spotlight alpha into the frame buffer alpha channel
    ;--------------------------------------------------------------------------

    ; load up the registers with the appropriate settings
    nop                                                 iaddiu      VI08, vi00, 0x7FFF
    nop                                                 iaddiu      VI08, VI08, 1
    nop                                                 ior         VI10, VI08, VI02
    nop                                                 iaddiu      VI09, vi00, 5
    nop                                                 lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )(vi00)
    nop                                                 lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 13 )(vi00)
    nop                                                 lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 16 )(vi00)
    nop                                                 lq.xyzw     VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 6 )(vi00)
    nop                                                 lq.xyzw     VF13xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 12 )(vi00)
    nop                                                 lq.xyzw     VF14xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 5 )(vi00)
    nop                                                 lq.yw       VF15yw,   ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 21 )(vi00)
    nop                                                 mfir.x      VF09x,    VI09                ; fill in the reg load giftag
    nop                                                 iaddiu      VI01, vi00, 0x121           ; turn off mipping
    nop                                                 mfir.x      VF14x,    VI01                ; turn off mipping
    nop                                                 iaddiu      VI01, vi00, 0x5F12        ; fill in the primitive giftag
    nop                                                 mfir.x      VF15x,    VI10                ; fill in the primitive giftag
    nop                                                 mfir.z      VF15z,    VI01                ; fill in the primitive giftag

    ; stall the previous pass
    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )             ; Wait for GS to finish
    nop                                                 xgkick      VI01

    ; render with the appropriate settings
    nop                                                 iaddiu      VI11, VI05, 1
    nop                                                 sq.xyzw     VF09xyzw, 0(VI11)
    nop                                                 sq.xyzw     VF10xyzw, 1(VI11)
    nop                                                 sq.xyzw     VF11xyzw, 2(VI11)
    nop                                                 sq.xyzw     VF12xyzw, 3(VI11)
    nop                                                 sq.xyzw     VF13xyzw, 4(VI11)
    nop                                                 sq.xyzw     VF14xyzw, 5(VI11)
    nop                                                 sq.xyzw     VF15xyzw, 6(VI11)
    nop                                                 nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 xgkick      VI11

    ;--------------------------------------------------------------------------
    ; re-render the diffuse texture with full color into the frame buffer using
    ; destination alpha
    ;--------------------------------------------------------------------------

    ; load up the registers with the appropriate settings
    nop                                                 iaddiu      VI08, vi00, 0x7FFF
    nop                                                 iaddiu      VI08, VI08, 1
    nop                                                 ior         VI10, VI08, VI02
    nop                                                 iaddiu      VI09, vi00, 5
    nop                                                 lq.xyzw     VF09xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )(vi00)
    nop                                                 lq.xyzw     VF10xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 13 )(vi00)
    nop                                                 lq.xyzw     VF11xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 14 )(vi00)
    nop                                                 lq.xyzw     VF12xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 19 )(vi00)
    nop                                                 lq.xyzw     VF13xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 2 )(vi00)
    nop                                                 lq.xyzw     VF14xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 5 )(vi00)
    nop                                                 lq.yw       VF15yw,   ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 20 )(vi00)
    nop                                                 mfir.x      VF09x,    VI09                ; fill in the reg load giftag
    nop                                                 iaddiu      VI01, vi00, 0x512F        ; fill in the primitive giftag
    nop                                                 mfir.x      VF15x,    VI10                ; fill in the primitive giftag
    nop                                                 mfir.z      VF15z,    VI01                ; fill in the primitive giftag

    ; stall the previous pass
    nop                                                 iaddiu      VI01, vi00, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 38 )     ; Wait for GS to finish
    nop                                                 xgkick      VI01

    ; render with the appropriate settings
    nop                                                 iaddiu      VI11, VI05, 1
    nop                                                 sq.xyzw     VF09xyzw, 0(VI11)
    nop                                                 sq.xyzw     VF10xyzw, 1(VI11)
    nop                                                 sq.xyzw     VF11xyzw, 2(VI11)
    nop                                                 sq.xyzw     VF12xyzw, 3(VI11)
    nop                                                 sq.xyzw     VF13xyzw, 4(VI11)
    nop                                                 sq.xyzw     VF14xyzw, 5(VI11)
    nop                                                 sq.xyzw     VF15xyzw, 6(VI11)
    nop                                                 nop
    nop                                                 nop
    nop                                                 b           SPOTLIGHT_RET
    nop                                                 xgkick      VI11

;==============================================================================
;
;   Distortion material pass
;
;   This code will generate uvs based on the screen space position and the
;   normal. The basic idea is to warp the screen based on the input normals.
;   At the edges of the character, hopefully the normals will be pointed
;   to the left and right.
;
;==============================================================================



















DISTORTION:
    nop                                                 lq.xyzw     VF16xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 8 )(vi00)
    nop                                                 lq.xyzw     VF17xyzw, ( ( ( ( 0 + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + (8 + (80 * 4)) ) + 9 )(vi00)
    nop                                                 loi         1792.0
    addi.x          VF15x,    vf00x,      i           loi         0.001953125
    addi.y          VF15y,    vf00y,      i           loi         0.5
    addi.z          VF15z,    vf00z,      i           mr32.xyzw   VF18xyzw, VF16xyzw
    addw.z          VF14z,    vf00z,      vf00w       iadd        VI08, VI06, vi00
    nop                                                 iadd        VI09, VI05, vi00
    nop                                                 iadd        VI10, VI09, VI02
    nop                                                 mr32.xyzw   VF18xyzw, VF18xyzw
    nop                                                 iadd        VI10, VI10, VI02
    mulax.xy        acc,        VF16xy,   VF07x     iadd        VI10, VI10, VI02
    madday.xy       acc,        VF17xy,   VF07y     iadd        VI10, VI10, VI02
    maddz.xy        VF21xy,   VF18xy,   VF07z     iaddiu      VI01, vi00, 0x80
    mulax.xy        acc,        VF16xy,   VF06x     nop
    madday.xy       acc,        VF17xy,   VF06y     nop
    maddz.xy        VF20xy,   VF18xy,   VF06z     nop
    mulax.xy        acc,        VF16xy,   VF05x     nop
    madday.xy       acc,        VF17xy,   VF05y     nop
    maddz.xy        VF19xy,   VF18xy,   VF05z     nop

DISTORTION_LOOP:
    nop                                                 lq.xy       VF09xy,   11(VI09)
    nop                                                 lq.xyz      VF11xyz,  8(VI09)
    nop                                                 iaddiu      VI08, VI08, 4
    nop                                                 iaddiu      VI09, VI09, 4
    itof4.xy        VF10xy,   VF09xy                nop
    itof12.xyz      VF12xyz,  VF11xyz               nop
    nop                                                 nop
    nop                                                 nop
    subx.xy         VF10xy,   VF10xy,   VF15x     nop
    mulaz.xy        acc,        VF21xy,   VF12z     nop
    madday.xy       acc,        VF20xy,   VF12y     nop
    maddx.xy        VF13xy,   VF19xy,   VF12x     nop
    nop                                                 nop
    nop                                                 nop
    mulay.xy        acc,        VF10xy,   VF15y     nop
    maddz.xy        VF14xy,   VF13xy,   VF15z     nop
    nop                                                 nop
    nop                                                 nop
    nop                                                 ibne        VI09, VI10, DISTORTION_LOOP
    nop                                                 sq.xyz      VF14xyz,  (9-4)(VI09)

    nop                                                 b           DISTORTION_RET
    nop                                                 nop



















VU1_MATERIAL_CODE_END:
