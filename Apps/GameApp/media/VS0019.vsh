;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;	Vertex fragment: kVS_OD0_BLITTER
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
;   Load texture coords
;
;==----------------------------------------------------------------------------

    sge  r0,v0,v0
    sub oT0.y ,c0.y,v2.y
    mov oT0.x ,v2.x
    mov oD0,r0
