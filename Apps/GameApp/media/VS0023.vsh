;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Vertex fragment: kVS_LIGHTMAP_TEX2
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
;   Set UV
;
;==----------------------------------------------------------------------------
                                ;
    rcp  r1.w,r9.w              ;   R9 = pos in clip space			w=0.00181426
    mul  r1.xy,r9.xy,r1.w       ;   r1.xy = r9.xy/r9.w				x=0.125,y=0.0625
                                ;
    mul  r1.xy,r1.xy,c-26		;	r1.x *= 0.5f, r1.y *=-0.5f		x=0.0625,y=-0.03125
    add  r1.xy,r1.xy,c-25.xy	;	r1.x += 0.5f, r1.y += 0.5f		x=0.5625,y= 0.46875
    mul  r1.xy,r1.xy,c-24.xy	;
								;
    mul oT2.xy,r1.xy,r9.w       ;	de-project uvs( we'll let r9.w do it )
    mov oT2.zw,r9.zw            ;
