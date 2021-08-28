;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;	VS0002.VSH: kVS_OD0_DIRECTIONAL
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
;   Apply lighting
;
;==----------------------------------------------------------------------------

    ;--------------------------------------------------------------------------
    ;
    ;   Calculate intensities
    ;
    dp3  r1.x,-c84,r10 ; Light 0 - r2 comes from previous shader fragment
    dp3  r1.y,-c85,r10 ; Light 1
    dp3  r1.z,-c86,r10 ; Light 2
    dp3  r1.w,-c87,r10 ; Light 3
    ;
    ;	Clamp intensities
    ;
    slt  r3,  r1,  r1
    max  r1.x,r1.x,r3
    max  r1.y,r1.y,r3
    max  r1.z,r1.z,r3
    max  r1.w,r1.w,r3

    ;--------------------------------------------------------------------------
    ;
    ;   Calculate colours
    ;
	dp4  r0.x,c88,r1 ; Light 0
	dp4  r0.y,c89,r1 ; Light 1
	dp4  r0.z,c90,r1 ; Light 2
	dp4  r0.w,c91,r1 ; Light 3

	add  oD0,r0,r5
