;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   VS0017: kVS_OD1_FOG
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
    sge r1.x,v0,v0                  ; r1.x = 1.0f
                                    ;
;   rcp r2.z,r9.w                   ; r2.z = 1/clip.w
;   mul r2.z,r9.z,r2.z              ; r2.z = clip.z * (1/clip.w)
;   max r2.z,r2.z,c-47.x            ; r2.z = max(r2.z,fog start)
;   min r2.z,r2.z,r1.x              ; r2.z = min(r2.z,1)
                                    ;
	mov r2.z,r9.w
    mov r1.y,r2.z                   ; r1.y = clip.z
    mul r1.z,r2.z,r2.z              ; r1.z = clip.z^2
    mul r1.w,r2.z,r1.z              ; r1.w = clip.z^3
                                    ;
;   mul r0,r1.x,c-46                ; DO POLYNOMIAL APPROXIMATION
;   mad r0,r1.y,c-45,r0             ;
;   mad r0,r1.z,c-44,r0             ;
;   mad r0,r1.w,c-43,r0             ;
                                    ;
    dp4 r0,r1,c-43                  ; DO POLYNOMIAL APPROXIMATION
                                    ;
    mov oD1,r0.w                    ; Load up the fog colour
