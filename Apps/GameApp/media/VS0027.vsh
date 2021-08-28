;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Vertex fragment: kVS_PROJ_SHADOW2
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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;==----------------------------------------------------------------------------
;
;   Transform vertex
;
;==----------------------------------------------------------------------------
                                 ;
    mul r0,c-32,r11.x            ;   dp4 r0.x,v0,c-32
    mad r0,c-31,r11.y,r0         ;   dp4 r0.y,v0,c-31
    mad r0,c-30,r11.z,r0         ;   dp4 r0.z,v0,c-30
    mad r0,c-29,r11.w,r0         ;   dp4 r0.w,v0,c-29

; ------------------------------ ; SET WHITE VERTEX ---------------------------
                                 ;
    sge oD0,v0,v0                ;	oD0 = 1,1,1,1
                                 ;
; ------------------------------ ; TRANSFORM WORLD TO PROJ SPACE --------------
                                 ;
    mul r2,c-36,r11.x            ;   dp4 r0.x,v0,c-36
    mad r2,c-35,r11.y,r2         ;   dp4 r0.y,v0,c-35
    mad r2,c-34,r11.z,r2         ;   dp4 r0.z,v0,c-34
    mad r2,c-33,r11.w,r2         ;   dp4 r0.w,v0,c-33
                                 ;
; ------------------------------ ; CREATE DISTORTION UVs ---------------------
;                                ;
;   rcp r1.w ,r9.w               ; input r9 = pos in clip space
;   mul r9.xy,r9.xy,r1.w         ; ...divide by w gives -1 to 1
;                                ;
;   mad r9.xy,r9.xy,c-1.x,c-1.x  ; r9 *= 0.5, r9 += 0.5 -- force 0 to 1
;   sub r9.y ,c-1.z,r9.y         ; y = 1-y
;                                ;
;   mul r9.xy,c-2.zw,r9.xy       ; r9.xy = uv in screenspace
;   add r9.xy,r9.xy,c-1.xx       ; get into 0.5 to 511.5 range
;                                ;
; ------------------------------ ; SAVE COORDS --------------------------------
;                                ;
;   mul oT0.xy,r9.xy,r9.w        ; de-project uvs
;   mov oT0.w ,r9.w              ; clip-space w
    mov oT0,r0                   ; t1 = shadow0
    mov oT1,r2                   ; t2 = shadow1
