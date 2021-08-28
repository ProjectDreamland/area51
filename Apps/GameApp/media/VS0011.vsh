;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Vertex fragment: kVS_RIGID_SHADOW_CASTER
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
    ;   Transform to projective space
    ;
    mul r0, c-31, v0.x		;   dp4 r0.x, v0, c-31
    mad r0, c-30, v0.y, r0	;   dp4 r0.y, v0, c-30
    mad r0, c-29, v0.z, r0	;   dp4 r0.z, v0, c-29
    mad r0, c-28, v0.w, r0	;   dp4 r0.w, v0, c-28
    ;
    mov oPos, r0
    ;
	mov oD0,c-27
