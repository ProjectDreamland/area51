;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Vertex fragment: kVS_OPOS_SKINNED
;
;   v0.xyz    = Position    c00-c79 = matrix palette
;   v1.xyz    = Normal      c92-c95 = world to clip
;   v2.xy     = Uv          c88-c91 = light dir mtx
;   v2.zw     = Weight      c84-c87 = light col mtx
;   v3.xy     = Bone        c83     = ambient
;
;                           c-1     = useful constants
;                           c-5     = env transform
;                           c-6     = fog info
;                       c-95 to -32 = fog table
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
                                    ;
    sge oD1.xyz,v0,v0               ;   oD1.xyz = 1.0   (no fog)
    slt oD1.w  ,v0,v0               ;   oD1.w   = 0.0
    mov  r5    ,c83                 ;	r5      = ambient
    add  r5,r5,c-50                 ;	c-4	= brightness
                                    ;
;==----------------------------------------------------------------------------
;
;   Apply skin to vertex
;
;==----------------------------------------------------------------------------
                                    ;
                                    ;   TRANSFORM THROUGH BONES
                                    ;
    mov a0.x,v3.x                   ;   Bone 0
    mul r0,c[a0.x+0],v0.x           ;   dp4 r0.x,v0,c[a0.x+0]
    mad r0,c[a0.x+1],v0.y,r0        ;   dp4 r0.y,v0,c[a0.x+1]
    mad r0,c[a0.x+2],v0.z,r0        ;   dp4 r0.z,v0,c[a0.x+2]
    mad r0,c[a0.x+3],v0.w,r0        ;   dp4 r0.w,v0,c[a0.x+3]
                                    ;
    mov a0.x,v3.y                   ;   Bone 1
    mul r1,c[a0.x+0],v0.x           ;   dp4 r1.x,v0,c[a0.x+0]
    mad r1,c[a0.x+1],v0.y,r1        ;   dp4 r1.y,v0,c[a0.x+1]
    mad r1,c[a0.x+2],v0.z,r1        ;   dp4 r1.z,v0,c[a0.x+2]
    mad r1,c[a0.x+3],v0.w,r1        ;   dp4 r1.w,v0,c[a0.x+3]
                                    ;
                                    ;   ADD WEIGHTS
    mul r0.xyz,r0.xyz,v2.z          ;
    mul r1.xyz,r1.xyz,v2.w          ;
    add r0.xyz,r0.xyz,r1.xyz        ;
    sge r0.w,v0,v0                  ;
    mov r11,r0                      ;   R11 = pos in world space
                                    ;
                                    ;   TRANSFORM WORLD TO CLIP
                                    ;
    mul r1,c92,r0.x                 ;   dp4 r1.x, r0, c92
    mad r1,c93,r0.y,r1              ;   dp4 r1.y, r0, c93
    mad r1,c94,r0.z,r1              ;   dp4 r1.z, r0, c94
    mad r1,c95,r0.w,r1              ;   dp4 r1.w, r0, c95
                                    ;
    mov oPos,r1                     ;
    mov r9,r1                       ;   R9 = pos in clip space
                                    ;
;==----------------------------------------------------------------------------
;
;   Apply skin to normal: r10 = result
;
;==----------------------------------------------------------------------------
                                    ;
                                    ;   TRANSFORM BONES
                                    ;
    mov a0.x,v3.x                   ;   Bone 0
    mul r0.xyz,c[a0.x+0],v1.x       ;   dp3 r0.x,v1,c[a0.x+0]
    mad r0.xyz,c[a0.x+1],v1.y,r0    ;   dp3 r0.y,v1,c[a0.x+1]
    mad r0.xyz,c[a0.x+2],v1.z,r0    ;   dp3 r0.z,v1,c[a0.x+2]
                                    ;
    mov a0.x,v3.y                   ;   Bone 1
    mul r1.xyz,c[a0.x+0],v1.x       ;   dp3 r1.x,v1,c[a0.x+0]
    mad r1.xyz,c[a0.x+1],v1.y,r1    ;   dp3 r1.y,v1,c[a0.x+1]
    mad r1.xyz,c[a0.x+2],v1.z,r1    ;   dp3 r1.z,v1,c[a0.x+2]
                                    ;
                                    ;   ADD WEIGHTS
    mul  r0.xyz,r0.xyz,v2.z         ;
    mul  r1.xyz,r1.xyz,v2.w         ;
                                    ;   SAVE OFF NORMAL
    add r10.xyz,r0.xyz,r1.xyz       ;
    mov r10.w,  r0.w                ;   R10 = normal in world space
