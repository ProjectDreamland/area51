;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Vertex fragment: kVS_PROJ
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
    ;   Transform world to proj space
    ;
    mul  r0,c-36,r11.x      ;   dp4 r0.x,v0,c-36
    mad  r0,c-35,r11.y,r0   ;   dp4 r0.y,v0,c-35
    mad  r0,c-34,r11.z,r0   ;   dp4 r0.z,v0,c-34
    mad  r0,c-33,r11.w,r0   ;   dp4 r0.w,v0,c-33

    mov oT3,r0

    ;--------------------------------------------------------------------------
    ;
    ;   Attenuation
    ;
    sub  r1,c-32.xyz,r11    ;   r1   = Lpos-Vpos (in world space)
    dp3  r1.w,r1,r1         ;   (normalise r1)
    rsq  r1.w, r1.w
    mul  r1,r1,r1.w         ;   r1   = negative light dir

    dp3  r3.w,r10,r1        ;   r3   = N dot -L
    add  r3.w,r3.w,c-32.w   ;   r3   = r3 + 0.5 (crank up intensity to compensate for improper falloff)
    mul oD0.w,r3.w,r0.z     ;   d0.a = intensity*attenuation
