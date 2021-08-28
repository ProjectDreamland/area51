;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Vertex fragment: kVS_SKINNED_SHADOW_CASTER
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

;==----------------------------------------------------------------------------
;
;   Transform vertex
;
;==----------------------------------------------------------------------------

    ;--------------------------------------------------------------------------
    ;
    ;   Transform through bones
    ;
    mov a0.x,v3.x                   ;   BONE 0
    mul r0,c[a0.x+0],v0.x           ;   dp4 r0.x,v0,c[a0.x+0]
    mad r0,c[a0.x+1],v0.y,r0        ;   dp4 r0.y,v0,c[a0.x+1]
    mad r0,c[a0.x+2],v0.z,r0        ;   dp4 r0.z,v0,c[a0.x+2]
    mad r0,c[a0.x+3],v0.w,r0        ;   dp4 r0.w,v0,c[a0.x+3]
    ;
    mov a0.x,v3.y                   ;   BONE 1
    mul r1,c[a0.x+0],v0.x           ;   dp4 r1.x,v0,c[a0.x+0]
    mad r1,c[a0.x+1],v0.y,r1        ;   dp4 r1.y,v0,c[a0.x+1]
    mad r1,c[a0.x+2],v0.z,r1        ;   dp4 r1.z,v0,c[a0.x+2]
    mad r1,c[a0.x+3],v0.w,r1        ;   dp4 r1.w,v0,c[a0.x+3]

    ;--------------------------------------------------------------------------
    ;
    ;   Add weights
    ;
    mul  r0.xyz, r0.xyz, v2.z
    mul  r1.xyz, r1.xyz, v2.w
    add  r0.xyz, r0.xyz, r1.xyz
    sge  r0.w  , v0    , v0

    ;--------------------------------------------------------------------------
    ;
    ;   Transform world to projector
    ;
    mul r1,c-31,r0.x                ;   dp4 r1.x, r0, c-31
    mad r1,c-30,r0.y,r1             ;   dp4 r1.y, r0, c-30
    mad r1,c-29,r0.z,r1             ;   dp4 r1.z, r0, c-29
    mad r1,c-28,r0.w,r1             ;   dp4 r1.w, r0, c-28
    ;
    mov oPos, r1
