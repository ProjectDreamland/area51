; back port from GOGO-no coda 2.24b by Takehiro TOMINAGA

; GOGO-no-coda
;	Copyright (C) 1999 shigeo
;	special thanks to Keiichi SAKAI

%include "nasm.h"

	globaldef fht_SSE
	globaldef fft_side_SSE
	externdef costab_fft
	externdef sintab_fft

	segment_data
	align 16
Q_MMPP	dd	0x0,0x0,0x80000000,0x80000000
Q_MPMP	dd	0x0,0x80000000,0x0,0x80000000
Q_002	dd	0.02236068, 0.02236068, 0.02236068, 0.02236068
D_SQRT2	dd	1.414213562,1.414213562
S_025	dd	0.25
S_05	DD	0.5
S_00005	DD	0.0005

	segment_code
;------------------------------------------------------------------------
;	by K. SAKAI
;	99/08/18	PIII 23k[clk]
;	99/08/19	命令順序入れ換え PIII 22k[clk]
;	99/08/20	bit reversal を旧午後から移植した PIII 17k[clk]
;	99/08/23	一部 unroll PIII 14k[clk]
;	99/11/12	clean up
;
;void fht_SSE(float *fz, int n);
	align 16
fht_SSE:
	push	ebx
	push	esi
	push	edi
	push	ebp
%assign _P 4*4

	;2つ目のループ
	mov	eax,[esp+_P+4]	;eax=fz
	mov	ebp,[esp+_P+8]	;=n
	shl	ebp,2
	add	ebp,eax		; fn  = fz + n, この関数終了まで不変

	xor	ecx,ecx		; ecx=k=0
	xor	eax,eax
	mov	al,4		; =k1=1*(sizeof float)	// 4, 16, 64, 256,...
	xor	edx,edx
	mov	dl,12		; =k3=3*k1
	jmp	short .lp2

	align	16
.lp2:				; do{
	add	cl,2		; k  += 2;
	shl	eax,2
	shl	edx,2

	mov	esi,[esp+_P+4]	;esi=fi=fz
	mov	edi,eax
	shr	edi,1
	add	edi,esi		; edi=gi=fi+ki/2

; たかだか2並列しか期待できない部分はFPUのほうが速い。
	movss	xmm7,[D_SQRT2]
	jmp	short .lp20

	align	16
.lp20:				; do{
;                       f0     = fi[0 ] + fi[k1];
;                       f2     = fi[k2] + fi[k3];
;                       f1     = fi[0 ] - fi[k1];
;                       f3     = fi[k2] - fi[k3];
;                       fi[0 ] = f0     + f2;
;                       fi[k1] = f1     + f3;
;                       fi[k2] = f0     - f2;
;                       fi[k3] = f1     - f3;
	fld	dword [esi]
	fadd	dword [esi+eax]
	fld	dword [esi+eax*2]
	fadd	dword [esi+edx]

	fld	dword [esi]
	fsub	dword [esi+eax]
	fld	dword [esi+eax*2]
	fsub	dword [esi+edx]

	fld	st1
	fadd	st0,st1
	fstp	dword [esi+eax]
	fsubp	st1,st0
	fstp	dword [esi+edx]

	fld	st1
	fadd	st0,st1
	fstp	dword [esi]
	fsubp	st1,st0
	fstp	dword [esi+eax*2]

	lea	esi,[esi + eax*4]	; = fi += (k1 * 4);
;	add	esi,eax
;	add	esi,edx
;                       g0     = gi[0 ] + gi[k1];
;                       g2     = SQRT2  * gi[k2];
;                       g1     = gi[0 ] - gi[k1];
;                       g3     = SQRT2  * gi[k3];
;                       gi[0 ] = g0     + g2;
;                       gi[k2] = g0     - g2;
;                       gi[k1] = g1     + g3;
;                       gi[k3] = g1     - g3;
	fld	dword [edi]
	fadd	dword [edi+eax]
	fld	dword [D_SQRT2]
	fmul	dword [edi+eax*2]

	fld	dword [edi]
	fsub	dword [edi+eax]
	fld	dword [D_SQRT2]
	fmul	dword [edi+edx]

	fld	st1
	fadd	st0,st1
	fstp	dword [edi+eax]
	fsubp	st1,st0
	fstp	dword [edi+edx]

	fld	st1
	fadd	st0,st1
	fstp	dword [edi]
	fsubp	st1,st0
	fstp	dword [edi+eax*2]

	lea	edi,[edi + eax*4]	; = gi += (k1 * 4);
	cmp	esi,ebp
	jl	near .lp20		; while (fi<fn);

;               i = 1; //for (i=1;i<kx;i++){
;                       c1 = 1.0*t_c - 0.0*t_s;
;                       s1 = 0.0*t_c + 1.0*t_s;
	movss	xmm6,[costab_fft + ecx*4]
	movss	xmm1,[sintab_fft + ecx*4]
	shufps	xmm6,xmm1,0x00	; = {s1, s1, c1, c1}
	shufps	xmm6,xmm6,0x28	; = {+c1, +s1, +s1, +c1}
;                       c2 = c1*c1 - s1*s1;
;                       s2 = c1*s1 + s1*c1;
	movaps	xmm4,xmm6
	movaps	xmm7,xmm6
	unpcklps	xmm4,xmm4	; = {s1, s1, c1, c1}
	shufps	xmm7,xmm7,0x41
	mulps	xmm4,xmm6	; = {s1*c1, s1*s1, c1*s1, c1*c1}
	xorps	xmm7,[Q_MMPP]	; = {-s1, -c1, +c1, +s1}
	movhlps	xmm3,xmm4
	xorps	xmm3,[Q_MPMP]
	subps	xmm4,xmm3	; = {--, --, s2, c2}
	movlhps	xmm4,xmm4	; = {+s2, +c2, +s2, +c2}
	movaps	xmm5,xmm4
	shufps	xmm5,xmm5,0x11
	xorps	xmm5,[Q_MPMP]	; = {-c2, +s2, -c2, +s2}
	mov	esi,[esp+_P+4]	; = fz
	lea	edi,[esi + eax - 4]	; edi = gi = fz +k1-i
	add	esi,4		; esi = fi = fz + i
	jmp	short .lp21

	align	16
.lp21:				; do{
;                               a       = c2*fi[k1] + s2*gi[k1];
;                               b       = s2*fi[k1] - c2*gi[k1];
;                               c       = c2*fi[k3] + s2*gi[k3];
;                               d       = s2*fi[k3] - c2*gi[k3];
;                               f0      = fi[0 ]        + a;
;                               g0      = gi[0 ]        + b;
;                               f2      = fi[k1 * 2]    + c;
;                               g2      = gi[k1 * 2]    + d;
;                               f1      = fi[0 ]        - a;
;                               g1      = gi[0 ]        - b;
;                               f3      = fi[k1 * 2]    - c;
;                               g3      = gi[k1 * 2]    - d;

	movss	xmm0,[esi + eax]	; = fi[k1]
	movss	xmm2,[esi + edx]	; = fi[k3]
	shufps	xmm0,xmm2,0x00	; = {fi[k3], fi[k3], fi[k1], fi[k1]}
	movss	xmm1,[edi + eax]	; = fi[k1]
	movss	xmm3,[edi + edx]	; = fi[k3]
	shufps	xmm1,xmm3,0x00	; = {gi[k3], gi[k3], gi[k1], gi[k1]}
	movss	xmm2,[esi]		; = fi[0]
	mulps	xmm0,xmm4		; *= {+s2, +c2, +s2, +c2}
	movss	xmm3,[esi + eax*2]	; = fi[k2]
	unpcklps	xmm2,xmm3	; = {--, --, fi[k2], fi[0]}
	mulps	xmm1,xmm5		; *= {-c2, +s2, -c2, +s2}
	movss	xmm3,[edi + eax*2]	; = gi[k2]
	addps	xmm0,xmm1		; = {d, c, b, a}
	movss	xmm1,[edi]		; = gi[0]
	unpcklps	xmm1,xmm3	; = {--,  --, gi[k2], gi[0]}
	unpcklps	xmm2,xmm1	; = {gi[k2], fi[k2], gi[0], fi[0]}
	movaps	xmm1,xmm2
	addps	xmm1,xmm0	; = {g2, f2, g0, f0}
	subps	xmm2,xmm0	; = {g3, f3, g1, f1}

;                               a       = c1*f2     + s1*g3;
;                               c       = s1*g2     + c1*f3;
;                               b       = s1*f2     - c1*g3;
;                               d       = c1*g2     - s1*f3;
;                               fi[0 ]  = f0        + a;
;                               gi[0 ]  = g0        + c;
;                               gi[k1]  = g1        + b;
;                               fi[k1]  = f1        + d;
;                               fi[k1 * 2]  = f0    - a;
;                               gi[k1 * 2]  = g0    - c;
;                               gi[k3]      = g1    - b;
;                               fi[k3]      = f1    - d;
	movaps	xmm3,xmm1
	movhlps	xmm1,xmm1	; = {g2, f2, g2, f2}
	shufps	xmm3,xmm2,0x14	; = {f1, g1, g0, f0}
	mulps	xmm1,xmm6	; *= {+c1, +s1, +s1, +c1}
	shufps	xmm2,xmm2,0xBB	; = {f3, g3, f3, g3}
	mulps	xmm2,xmm7	; *= {-s1, -c1, +c1, +s1}
	addps	xmm1,xmm2	; = {d, b, c, a}
	movaps	xmm2,xmm3
	addps	xmm3,xmm1	; = {fi[k1], gi[k1], gi[0], fi[0]}
	subps	xmm2,xmm1	; = {fi[k3], gi[k3], gi[k1*2], fi[k1*2]}
	movhlps	xmm0,xmm3
	movss	[esi],xmm3
	shufps	xmm3,xmm3,0x55
	movss	[edi+eax],xmm0
	shufps	xmm0,xmm0,0x55
	movss	[edi],xmm3
	movss	[esi+eax],xmm0
	movhlps	xmm0,xmm2
	movss	[esi+eax*2],xmm2
	shufps	xmm2,xmm2,0x55
	movss	[edi+edx],xmm0
	shufps	xmm0,xmm0,0x55
	movss	[edi+eax*2],xmm2
	lea	edi,[edi + eax*4] ; gi += (k1 * 4);
	movss	[esi+edx],xmm0
	lea	esi,[esi + eax*4] ; fi += (k1 * 4);
	cmp	esi,ebp
	jl	near .lp21		; while (fi<fn);
; unroll前のdo loopは43+4命令

; 最内周ではないforループをunrollingした
; kx=   2,   8,  32,  128
; k4=  16,  64, 256, 1024
;       0, 6/2,30/2,126/2
; at here
;	xmm6 = {--, --, s1, c1}
;               c3 = c1; s3 = s1;
	xor	ebx,ebx
	mov	bl,4*4		; = i = 4
	cmp	ebx,eax		; i < k1
	jnl	near .F22

	shufps	xmm6,xmm6,0x14	; = {c1, s1, s1, c1}
	jmp	short .F220

	align	16
;               for (i=4;i<k1;i+=4){ // for (i=2;i<k1/2;i+=2){
.lp22:
	shufps	xmm6,xmm6,0x69	; xmm6 = {c3, s3, s3, c3}
.F220:
; at here, xmm6 is {c3, s3, s3, c3}
;                       c1 = c3*t_c - s3*t_s;
;                       s1 = c3*t_s + s3*t_c;
	movss	xmm0,[costab_fft + ecx*4]
	movss	xmm1,[sintab_fft + ecx*4]
	shufps	xmm0,xmm1,0x00	; = {t_s, t_s, t_c, t_c}
	mulps	xmm6,xmm0
	movhlps	xmm4,xmm6
	xorps	xmm4,[Q_MPMP]
	subps	xmm6,xmm4	; = {--, --, s1, c1}

;                       c3 = c1*t_c - s1*t_s;
;                       s3 = s1*t_c + c1*t_s;
	shufps	xmm6,xmm6,0x14	; = {c1, s1, s1, c1}
	mulps	xmm0,xmm6
	movhlps	xmm3,xmm0
	xorps	xmm3,[Q_MPMP]
	subps	xmm0,xmm3	; = {--, --, s3, c3}

	unpcklps	xmm6,xmm0	; xmm6 = {s3, s1, c3, c1}
	shufps	xmm6,xmm6,0xB4	; xmm6 = {s1, s3, c3, c1}

;                       c2 = c1*c1 - s1*s1;
;                       c4 = c3*c3 - s3*s3;
;                       s4 = s3*c3 + s3*c3;
;                       s2 = s1*c1 + s1*c1;
	movaps	xmm7,xmm6
	movaps	xmm4,xmm6
	shufps	xmm7,xmm7,0x14
	shufps	xmm4,xmm4,0xEB
	xorps	xmm4,[Q_MMPP]	; = {-c3,-c1, s3, s1}
	mulps	xmm7,xmm6
	mulps	xmm4,xmm6
	shufps	xmm4,xmm4,0x1B
	addps	xmm7,xmm4	; xmm7 = {s2, s4, c4, c2}

;                       fi = fz +i;
;                       gi = fz +k1-i;
	mov	edi,[esp+_P+4]	; = fz
	mov	esi,ebx
	shr	esi,1
	sub	edi,esi		; edi = fz - i/2
	lea	esi,[edi + ebx]	; esi = fi = fz +i/2
	add	edi,eax		; edi = gi = fz +k1-i/2
	sub	edi,4
;                       do{
.lp220:
; unroll後のdo loopは51+4命令
;                               a       = c2*fi[k1  ] + s2*gi[k1  ];
;                               e       = c4*fi[k1+1] + s4*gi[k1-1];
;                               f       = s4*fi[k1+1] - c4*gi[k1-1];
;                               b       = s2*fi[k1  ] - c2*gi[k1  ];
;                               c       = c2*fi[k3  ] + s2*gi[k3  ];
;                               g       = c4*fi[k3+1] + s4*gi[k3-1];
;                               h       = s4*fi[k3+1] - c4*gi[k3-1];
;                               d       = s2*fi[k3  ] - c2*gi[k3  ];

	movaps	xmm4,xmm7	; xmm7 = {s2, s4, c4, c2}
	shufps	xmm4,xmm4,0x1B
	xorps	xmm4,[Q_MMPP]
	movlps	xmm0,[esi+eax]
	movlps	xmm1,[edi+eax]
	movlps	xmm2,[esi+edx]
	movlps	xmm3,[edi+edx]
	shufps	xmm0,xmm0,0x14
	shufps	xmm1,xmm1,0x41
	shufps	xmm2,xmm2,0x14
	shufps	xmm3,xmm3,0x41
	mulps	xmm0,xmm7
	mulps	xmm1,xmm4
	mulps	xmm2,xmm7
	mulps	xmm3,xmm4
	addps	xmm0,xmm1	; xmm0 = {b, f, e, a}
	addps	xmm2,xmm3	; xmm2 = {d, h, g, c}
;17

;                               f0      = fi[0   ]    + a;
;                               f4      = fi[0 +1]    + e;
;                               g4      = gi[0 -1]    + f;
;                               g0      = gi[0   ]    + b;
;                               f1      = fi[0   ]    - a;
;                               f5      = fi[0 +1]    - e;
;                               g5      = gi[0 -1]    - f;
;                               g1      = gi[0   ]    - b;
;                               f2      = fi[k2  ]    + c;
;                               f6      = fi[k2+1]    + g;
;                               g6      = gi[k2-1]    + h;
;                               g2      = gi[k2  ]    + d;
;                               f3      = fi[k2  ]    - c;
;                               f7      = fi[k2+1]    - g;
;                               g7      = gi[k2-1]    - h;
;                               g3      = gi[k2  ]    - d;
	movlps	xmm4,[esi      ]
	movhps	xmm4,[edi      ]
	movaps	xmm1,xmm4
	subps	xmm1,xmm0	; xmm1 = {g1, g5, f5, f1}
	movlps	xmm5,[esi+eax*2]
	movhps	xmm5,[edi+eax*2]
	movaps	xmm3,xmm5
	subps	xmm3,xmm2	; xmm3 = {g3, g7, f7, f3}
	addps	xmm0,xmm4	; xmm0 = {g0, g4, f4, f0}
	addps	xmm2,xmm5	; xmm2 = {g2, g6, f6, f2}
;10

;                               a       = c1*f2     + s1*g3;	順*順 + 逆*逆
;                               e       = c3*f6     + s3*g7;
;                               g       = s3*g6     + c3*f7;
;                               c       = s1*g2     + c1*f3;
;                               d       = c1*g2     - s1*f3;	順*逆 - 逆*順
;                               h       = c3*g6     - s3*f7;
;                               f       = s3*f6     - c3*g7;
;                               b       = s1*f2     - c1*g3;

	movaps	xmm5,xmm6	; xmm6 = {s1, s3, c3, c1}
	shufps	xmm5,xmm5,0x1B	; = {c1, c3, s3, s1}
	movaps	xmm4,xmm2
	mulps	xmm4,xmm6
	shufps	xmm2,xmm2,0x1B	; xmm2 = {f2, f6, g6, g2}
	mulps	xmm2,xmm6
	mulps	xmm5,xmm3
	mulps	xmm3,xmm6
	shufps	xmm3,xmm3,0x1B
	addps	xmm4,xmm3	; = {c, g, e, a}
	subps	xmm2,xmm5	; = {b, f, h, d}
;10

;                               fi[0   ]  = f0        + a;
;                               fi[0 +1]  = f4        + e;
;                               gi[0 -1]  = g4        + g;
;                               gi[0   ]  = g0        + c;
;                               fi[k2  ]  = f0        - a;
;                               fi[k2+1]  = f4        - e;
;                               gi[k2-1]  = g4        - g;
;                               gi[k2  ]  = g0        - c;
;                               fi[k1  ]  = f1        + d;
;                               fi[k1+1]  = f5        + h;
;                               gi[k1-1]  = g5        + f;
;                               gi[k1  ]  = g1        + b;
;                               fi[k3  ]  = f1        - d;
;                               fi[k3+1]  = f5        - h;
;                               gi[k3-1]  = g5        - f;
;                               gi[k3  ]  = g1        - b;
	movaps	xmm3,xmm0
	subps	xmm0,xmm4
	movlps	[esi+eax*2],xmm0
	movhps	[edi+eax*2],xmm0
	addps	xmm4,xmm3
	movlps	[esi      ],xmm4
	movhps	[edi      ],xmm4

	movaps	xmm5,xmm1
	subps	xmm1,xmm2
	movlps	[esi+edx  ],xmm1
	movhps	[edi+edx  ],xmm1
	addps	xmm2,xmm5
	movlps	[esi+eax  ],xmm2
	movhps	[edi+eax  ],xmm2
; 14
;                               gi     += k4;
;                               fi     += k4;
	lea	edi,[edi + eax*4] ; gi += (k1 * 4);
	lea	esi,[esi + eax*4] ; fi += (k1 * 4);
	cmp	esi,ebp
	jl	near .lp220		; while (fi<fn);
;                       } while (fi<fn);

	add	ebx,4*4		; i+= 4
	cmp	ebx,eax		; i < k1
	jl	near .lp22
;               }
.F22:

	cmp	eax,[esp+_P+8]	; while ((k1 * 4)<n);
	jl	near .lp2

	pop	ebp
	pop	edi
	pop	esi
	pop	ebx
	ret

;------------------------------------------------------------------------
;	99/11/12	Initial version for SSE by K. SAKAI, 4300clk@P3
; This routine is very slow when wsamp_r_int is not aligned to 16byte boundary.
;
;void fft_side_SSE( float in[2][1024], int s, float *ret)
;        energy = (in[0][512] - in[1][512])^2;
;        energy = (in[0][1024-s] - in[1][1024-s])^2;
;        for (i=s,j=1024-s;i<512;i++,j--){
;                a = in[0][i] - in[1][i];
;                energy += a*a;
;                b = in[0][j-1] - in[1][j-1];
;                energy += b*b;
;        }
;        *ret = energy * 0.25;

	align	16
fft_side_SSE:
	mov	ecx,[esp+8]	; = i = s
	mov	edx,1024
	sub	edx,ecx		; = j
	mov	eax,[esp+4]	; = in
	movss	xmm7,[eax+1024*0*4+512*4]
	movss	xmm1,[eax+1024*1*4+512*4]
	subss	xmm7,xmm1
	mulss	xmm7,xmm7
	movss	xmm2,[eax+1024*0*4+edx*4]
	movss	xmm3,[eax+1024*1*4+edx*4]
	subss	xmm2,xmm3
	mulss	xmm2,xmm2
	addss	xmm7,xmm2

	test	cl,1
	jz	.even

.odd:	dec	edx
	movss	xmm0,[eax+1024*0*4+ecx*4]
	movss	xmm1,[eax+1024*1*4+ecx*4]
	inc	ecx
	movss	xmm2,[eax+1024*0*4+edx*4]
	movss	xmm3,[eax+1024*1*4+edx*4]
	cmp	ecx,edx
	subss	xmm0,xmm1
	subss	xmm2,xmm3
	mulss	xmm0,xmm0
	mulss	xmm2,xmm2
	addss	xmm7,xmm0
	addss	xmm7,xmm2
	je	near .exit1

.even:	test	cl,2
	jz	.f0
	sub	edx,2
	movlps	xmm0,[eax+1024*0*4+ecx*4]
	movlps	xmm1,[eax+1024*1*4+ecx*4]
	add	ecx,2
	movhps	xmm0,[eax+1024*0*4+edx*4]
	movhps	xmm1,[eax+1024*1*4+edx*4]
	cmp	ecx,edx
	subps	xmm0,xmm1
	mulps	xmm0,xmm0
	addps	xmm7,xmm0
	je	.exit4
	jmp	short .f0

	align	16
.f0:
.lp0:
	sub	edx,4
	movaps	xmm0,[eax+1024*0*4+ecx*4]
	movaps	xmm1,[eax+1024*1*4+ecx*4]
	add	ecx,4
	subps	xmm0,xmm1
	mulps	xmm0,xmm0
	addps	xmm7,xmm0
	movaps	xmm2,[eax+1024*0*4+edx*4]
	movaps	xmm3,[eax+1024*1*4+edx*4]
	cmp	ecx,edx
	subps	xmm2,xmm3
	mulps	xmm2,xmm2
	addps	xmm7,xmm2
	jne	.lp0

.exit4:	movhlps	xmm6,xmm7
	addps	xmm7,xmm6
	movaps	xmm6,xmm7
	shufps	xmm6,xmm6,01010101B
	addss	xmm7,xmm6

.exit1:	mulss	xmm7,[S_025]
	mov	eax,[esp+12]
	movss	[eax],xmm7
	ret


	end
