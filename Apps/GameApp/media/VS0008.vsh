;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;	Vertex fragment: kVS_OT0_CUBE
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
    ;   Transform point to eye space
    ;
    mul r3, c-5, v0.x       ;   dp4 r3.x,v0, c-5
    mad r3, c-4, v0.y, r3   ;   dp4 r3.y,v0, c-4
    mad r3, c-3, v0.z, r3   ;   dp4 r3.z,v0, c-3
    mad r3, c-2, v0.w, r3   ;   dp4 r3.w,v0, c-2
    ;
    ;	Scroll UVs
    ;
	add oT1.xyz,r3,c-9

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
