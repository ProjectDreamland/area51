;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;   Vertex fragment: kVS_OT0_ENV
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

    ;--------------------------------------------------------------------------
    ;
    ;   Transform normal
    ;
    mul r1.xyz,c-5,r10.x    ; dp3 r1.x,r10,c-5
    mad r1.xyz,c-4,r10.y,r1 ; dp3 r1.y,r10,c-4
    mad r1.xyz,c-3,r10.z,r1 ; dp3 r1.z,r10,c-3

    ;--------------------------------------------------------------------------
    ;
    ;   Normalise result
    ;
    dp3 r1.w ,r1,r1
    rsq r1.w ,r1.w
    mul r1,r1,r1.w
    ;
    ;   Force 0-1
    ;
    mul  r1   ,r1,c-1.x ; r1 *= 0.5
    add oT1.xy,r1,c-1.x ; r1 += 0.5

    ;--------------------------------------------------------------------------
    ;
    ;   Update diffuse coords
    ;
	mul  r2.xy,v2.xy,c-8.xy
	add  r2.xy,r2.xy,c-9.xy ; scrolling uvs
	mov oT0.xy,r2

    ;--------------------------------------------------------------------------
    ;
    ;   Update detail coords
    ;
	mul  r3.xy,v2,c-9.zw ; scaling
	mov oT2.xy,r3
