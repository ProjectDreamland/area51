;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   VS0017: kVS_OT0_DISTORTION
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
    sge oD0,v0,v0                  ; white = colour
                                   ;
; -------------------------------- ; CREATE DISTORTION UVs ---------------------
                                   ;
    rcp r1.w ,r9.w                 ; input r9 = pos in clip space
    mul r9.xy,r9.xy,r1.w           ; ...divide by w gives -1 to 1
                                   ;
    mad r9.xy,r9.xy,c-1.x,c-1.x    ; r9 *= 0.5, r9 += 0.5 -- force 0 to 1
    sub r9.y,c-1.z,r9.y            ; invert y
                                   ;
    mad r9.xy,c-2.zw,r9.xy,c-2.xy  ; r9.xy = uv in screenspace
                                   ;
; -------------------------------- ; GET NORMAL IN EYE SPACE -------------------
                                   ;
    mul r1.xy,c-5,r10.x            ; dp3 r1.x, r10, c-5
    mad r1.xy,c-4,r10.y,r1         ; dp3 r1.y, r10, c-4
    mad r1.xy,c-3,r10.z,r1         ; dp3 r1.z, r10, c-3
                                   ;
    dp3 r1.w ,r1.xy,r1.xy          ; Normalise result
    rsq r1.w ,r1.w                 ;
    mul r1.xy,r1.xy,r1.ww          ;
                                   ;
; -------------------------------- ; WRITE DISTORTION UVs ----------------------
                                   ;
    mad  r9.xy,r1.xy,c-1.ww,r9.xy  ; scale normal
    mul oT0.xy,r9.xy,r9.ww         ; de-project uvs
    mov oT0.w ,r9.w                ;
                                   ;
; -------------------------------- ; WRITE ENV MAP UVs -------------------------
                                   ;
    mad oT1.xy,r1.xy,c-1.xx,c-1.xx ; r1 *= 0.5, r1 += 0.5
