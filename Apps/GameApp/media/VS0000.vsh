;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   VS0000.VSH: oPos_Rigid
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                            ;
    sge oD1.xyz,v0,v0       ;   oD1.xyz = 1.0   (no fog)
    slt oD1.w  ,v0,v0       ;   oD1.w   = 0.0
    add  r5,v3,c-50         ;   r5      = baked lighting (c-4 = brightness)
                            ;
;==----------------------------------------------------------------------------
;
;   Transform vertex
;
;==----------------------------------------------------------------------------
                            ;
                            ;   TRANSFORM WORLD TO CLIP
                            ;
    mul r0,c92,v0.x         ;   dp4 r0.x,v0,c92
    mad r0,c93,v0.y,r0      ;   dp4 r0.y,v0,c93
    mad r0,c94,v0.z,r0      ;   dp4 r0.z,v0,c94
    mad r0,c95,v0.w,r0      ;   dp4 r0.w,v0,c95
                            ;
    mov oPos,r0             ;
    mov r9,r0               ;   r9 = pos in clip space
                            ;
; ------------------------- ;   TRANSFORM LOCAL TO WORLD/VIEW
                            ;
    mul r11,c0,v0.x         ;   dp4 r11.x,v0,c0
    mad r11,c1,v0.y,r11     ;   dp4 r11.y,v0,c1
    mad r11,c2,v0.z,r11     ;   dp4 r11.z,v0,c2
    mad r11,c3,v0.w,r11     ;   dp4 r11.w,v0,c3
                            ;
                            ;   r11 = pos in world/view space
                            ;
; ------------------------- ;   TRANSFORM NORMAL TO WORLD/VIEW SPACE
                            ;
    mul r10.xyz,c0,v1.x     ;   dp3 r10.x,v1,c0
    mad r10.xyz,c1,v1.y,r10 ;   dp3 r10.y,v1,c1
    mad r10.xyz,c2,v1.z,r10 ;   dp3 r10.z,v1,c2
    sge r10.w  ,v1,v1       ;
                            ;   r10 = normal in world/view space
