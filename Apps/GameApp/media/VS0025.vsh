;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   VS0025: kVS_OT0_INTENSITY
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
    sge oD0,v0,v0                ; white = colour
                                 ;
; ------------------------------ ; CREATE INTENSITY UVs ----------------------
                                 ;
    rcp r1.w ,r9.w               ; input r9 = pos in clip space
    mul r9.xy,r9.xy,r1.w         ; ...divide by w gives -1 to 1
                                 ;
    mad r9.xy,r9.xy,c-1.x,c-1.x  ; r9 *= 0.5, r9 += 0.5 -- force 0 to 1
                                 ;
    mul r9.xy,c-2.zw,r9.xy       ; r9.xy = uv in screenspace
    sub r9.y ,c-2. w,r9. y       ; invert y
                                 ;
; ------------------------------ ; WRITE INTENSITY UVs -----------------------
                                 ;
    mul oT1.xy,r9.xy,r9.w        ; de-project uvs
    mov oT1.w ,r9.w              ;
                                 ;
; ------------------------------ ; WRITE DIFFUSE UVs -------------------------
                                 ;
    mov oT0.xy,v9                ;
