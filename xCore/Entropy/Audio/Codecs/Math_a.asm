;############################################################################
;##                                                                        ##
;##  MATH_A.ASM                                                            ##
;##                                                                        ##
;##  Miles Sound System                                                    ##
;##                                                                        ##
;##  IMDCT and polyphase-filter support routines                           ##
;##                                                                        ##
;##  Version 1.00 of 2-Mar-99: Initial, derived from algorithms by         ##
;##                            Byeong Gi Lee, Jeff Tsay, Francois          ##
;##                            Raymond-Boyer, K. Konstantinides, Mikko     ##
;##                            Tommila, et al.                             ##
;##                                                                        ##
;##  80386/AMD K6-3D ASM source compatible with Microsoft Assembler v6.13  ##
;##  or later                                                              ##
;##                                                                        ##
;##  Author: John Miles                                                    ##
;##                                                                        ##
;############################################################################
;##                                                                        ##
;##  Copyright (C) RAD Game Tools, Inc.                                    ##
;##                                                                        ##
;##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
;##                                                                        ##
;############################################################################

                OPTION SCOPED           ;Enable local labels
                .586                    ;Enable Pentium instructions
                .MMX                    ;Enable MMX instructions

                IFDEF AMD
                  .K3D                  ;Enable 3DNow opcodes if building AMD version
                ENDIF

;                IFDEF DPMI
                  .MODEL FLAT,C
;                ELSE
;                  .MODEL FLAT,STDCALL
;                ENDIF

                .DATA

IFDEF AMD
amd_data_start LABEL BYTE
public amd_data_start
ELSE
x86_data_start LABEL BYTE
public x86_data_start
ENDIF


sin30           dd 0.500000000F         ;sin(pi/6)
cos30           dd 0.866025403F         ;cos(pi/6)

dct3t1          dd 1.931851653F       
dct3t2          dd 0.707106781F
dct3t3          dd 0.517638090F
dct3t4          dd 0.504314480F
dct3t5          dd 0.541196100F
dct3t6          dd 0.630236207F
dct3t7          dd 0.821339815F
dct3t8          dd 1.306562965F
dct3t9          dd 3.830648788F
dct3t10         dd -0.793353340F
dct3t11         dd -0.608761429F
dct3t12         dd -0.923879532F
dct3t13         dd -0.382683432F
dct3t14         dd -0.991444861F
dct3t15         dd -0.130526192F
dct3t16         dd  1.000000000F
dct3t17         dd  0.382683432F
dct3t18         dd  0.608761429F
dct3t19         dd -0.793353340F
dct3t20         dd -0.923879532F
dct3t21         dd -0.991444861F
dct3t22         dd  0.130526192F

dct9t0          dd 1.8793852415718F
dct9t1          dd 1.532088886238F
dct9t2          dd 0.34729635533386F
dct9t3          dd -0.34729635533386F
dct9t4          dd -1.8793852415718F
dct9t5          dd 1.532088886238F
dct9t6          dd -1.532088886238F
dct9t7          dd 0.34729635533386F
dct9t8          dd -1.8793852415718F
dct9t9          dd 1.732050808F
dct9t10         dd 1.9696155060244F
dct9t11         dd 1.2855752193731F
dct9t12         dd 0.68404028665134F
dct9t13         dd 1.2855752193731F
dct9t14         dd -0.68404028665134F
dct9t15         dd 1.9696155060244F
dct9t16         dd 0.68404028665134F
dct9t17         dd 1.9696155060244F
dct9t18         dd -1.2855752193731F
dct9t19         dd 0.707106781F

dct36t0         dd 0.501909918F
dct36t1         dd -0.250238171F
dct36t2         dd -5.731396405F

dct36t3         dd 0.517638090F
dct36t4         dd -0.25215724F
dct36t5         dd -1.915324394F
dct36t6         dd 0.551688959F
dct36t7         dd -0.256069878F
dct36t8         dd -1.155056579F
dct36t9         dd 0.610387294F
dct36t10        dd -0.262132281F
dct36t11        dd -0.831377381F
dct36t12        dd 0.871723397F
dct36t13        dd -0.281845486F
dct36t14        dd -0.541420142F
dct36t15        dd 1.183100792F
dct36t16        dd -0.296422261F
dct36t17        dd -0.465289749F
dct36t18        dd 1.931851653F
dct36t19        dd -0.315118103F
dct36t20        dd -0.410669907F
dct36t21        dd 5.736856623F
dct36t22        dd -0.339085426F
dct36t23        dd -0.370046808F
dct36t24        dd 0.707106781F
dct36t25        dd -0.541196100F
dct36t26        dd -1.306562965F

IFDEF AMD
amd_data_end LABEL BYTE
public amd_data_end
ELSE
x86_data_end LABEL BYTE
public x86_data_end
ENDIF

                .CODE

                IFDEF AMD

          NAME MATHAMD

          PUBLIC AMD_poly_filter
                  PUBLIC AMD_dewindow_and_write
                  PUBLIC AMD_IMDCT_3x12
                  PUBLIC AMD_IMDCT_1x36
                ELSE

          NAME MATHX86

                  PUBLIC x86_poly_filter
                  PUBLIC x86_dewindow_and_write
                  PUBLIC x86_IMDCT_3x12
                  PUBLIC x86_IMDCT_1x36
                ENDIF

fdup            MACRO
                fld st(0)               ;dup top of stack
                ENDM

_FEMMS          MACRO                   ;emit femms on AMD only
                IFDEF AMD
                  femms
                ENDIF
                ENDM

IFDEF AMD
amd_code_start LABEL BYTE
public amd_code_start
ELSE
x86_code_start LABEL BYTE
public x86_code_start
ENDIF

;#############################################################################
;##                                                                         ##
;## 1x36 IMDCT for window types 0, 1, and 3 (long windows)                  ##
;##                                                                         ##
;#############################################################################

                ;
                ;C = C + B
                ;B = B + A
                ;

CASCADE_ADD     MACRO A,B,C

                IFDEF AMD

                  movd mm0,[esi+(A*4)]
                  movd mm1,[esi+(B*4)]
                  movd mm2,[esi+(C*4)]
                  pfadd mm2,mm1
                  pfadd mm1,mm0
                  movd [esi+(C*4)],mm2
                  movd [esi+(B*4)],mm1

                ELSE

                  fld DWORD PTR [esi+(A*4)]
                  fld DWORD PTR [esi+(B*4)]
                  fld DWORD PTR [esi+(C*4)]
                  fadd st,st(1)
                  fxch st(1)
                  faddp st(2),st
                  fstp DWORD PTR [esi+(C*4)]
                  fstp DWORD PTR [esi+(B*4)]

                ENDIF

                ENDM

                ;
                ;dest = base + (a*fa) + (b*fb) + (c*fc)
                ;

MAC3            MACRO base,a,b,c,fa,fb,fc,dest

                IFDEF AMD

                  movd mm0,[esi+((a)*4)]
                  movd mm1,[esi+((b)*4)]
                  movd mm2,[esi+((c)*4)]
                  movd mm3,fa
                  movd mm4,fb
                  movd mm5,fc
                  pfmul mm3,mm0
                  pfmul mm4,mm1
                  pfmul mm5,mm2
                  movq mm0,base
                  pfadd mm0,mm3
                  pfadd mm4,mm5
                  pfadd mm0,mm4
                  movd [edi+((dest)*4)],mm0

                ELSE

                  fld DWORD PTR [esi+((a)*4)]
                  fld DWORD PTR [esi+((b)*4)]
                  fld DWORD PTR [esi+((c)*4)]
                  fmul fc
                  fxch st(2)
                  fmul fa
                  fxch st(1)
                  fmul fb
                  fld base
                  faddp st(1),st
                  faddp st(1),st
                  faddp st(1),st
                  fstp DWORD PTR [edi+((dest)*4)]

                ENDIF

                ENDM

                ;
                ;9-point IDCT
                ;

IDCT_9          MACRO src,dest

                IFDEF AMD

                  movd mm6,[esi+((src+0)*4)]
                  movd mm7,[esi+((src+12)*4)]
                  pfadd mm6,mm6
                  pfadd mm7,mm6
                  
                  MAC3 mm7,4+src,8+src,16+src,dct9t0,dct9t1,dct9t2,0+dest
                  MAC3 mm7,4+src,8+src,16+src,dct9t3,dct9t4,dct9t5,2+dest
                  MAC3 mm7,4+src,8+src,16+src,dct9t6,dct9t7,dct9t8,3+dest

                  movd mm0,[esi+((src+4)*4)]
                  movd mm1,[esi+((src+8)*4)]
                  movd mm2,[esi+((src+12)*4)]
                  movd mm3,[esi+((src+16)*4)]

                  movq mm4,mm6
                  movd mm5,[esi+((src+0)*4)]
                  pfadd mm4,mm0
                  pfsub mm5,mm0
                  pfsub mm4,mm1
                  pfadd mm5,mm1
                  pfsub mm4,mm2
                  pfsub mm5,mm2
                  pfsub mm4,mm2
                  pfadd mm5,mm3
                  pfsub mm4,mm3
                  movd [edi+((dest+4)*4)],mm5
                  movd [edi+((dest+1)*4)],mm4

                  movd mm0,dct9t9
                  movd mm6,[esi+((src+6)*4)]
                  pfmul mm6,mm0
                  pxor mm7,mm7
                  pfsub mm7,mm6

                  MAC3 mm6,2+src,10+src,14+src,dct9t10,dct9t11,dct9t12,5+dest
                  MAC3 mm7,2+src,10+src,14+src,dct9t13,dct9t14,dct9t15,6+dest
                  MAC3 mm7,2+src,10+src,14+src,dct9t16,dct9t17,dct9t18,7+dest
                  
                  movd mm0,[esi+((src+2)*4)]
                  movd mm1,[esi+((src+10)*4)]
                  movd mm2,[esi+((src+14)*4)]
                  movd mm3,dct9t9
                  pfsub mm0,mm1
                  pfsub mm0,mm2
                  pfmul mm0,mm3
                  movd [edi+((dest+8)*4)],mm0

                ELSE 

                  fld DWORD PTR [esi+((src+0)*4)]
                  fadd st,st(0)
                  fst a
                  fadd DWORD PTR [esi+((src+12)*4)]
                  fstp b
                  
                  MAC3 b,4+src,8+src,16+src,dct9t0,dct9t1,dct9t2,0+dest
                  MAC3 b,4+src,8+src,16+src,dct9t3,dct9t4,dct9t5,2+dest
                  MAC3 b,4+src,8+src,16+src,dct9t6,dct9t7,dct9t8,3+dest

                  fld a
                  fadd DWORD PTR [esi+((src+4)*4)]
                  fsub DWORD PTR [esi+((src+8)*4)]
                  fsub DWORD PTR [esi+((src+12)*4)]
                  fsub DWORD PTR [esi+((src+12)*4)]
                  fsub DWORD PTR [esi+((src+16)*4)]
                  fstp DWORD PTR [edi+((dest+1)*4)]
                  
                  fld DWORD PTR [esi+((src+0)*4)]
                  fsub DWORD PTR [esi+((src+4)*4)]
                  fadd DWORD PTR [esi+((src+8)*4)]
                  fsub DWORD PTR [esi+((src+12)*4)]
                  fadd DWORD PTR [esi+((src+16)*4)]
                  fstp DWORD PTR [edi+((dest+4)*4)]
                  
                  fld DWORD PTR [esi+((src+6)*4)]
                  fmul dct9t9
                  fst a
                  fchs
                  fstp b
                  
                  MAC3 a,2+src,10+src,14+src,dct9t10,dct9t11,dct9t12,5+dest
                  MAC3 b,2+src,10+src,14+src,dct9t13,dct9t14,dct9t15,6+dest
                  MAC3 b,2+src,10+src,14+src,dct9t16,dct9t17,dct9t18,7+dest
                  
                  fld DWORD PTR [esi+((src+2)*4)]
                  fsub DWORD PTR [esi+((src+10)*4)]
                  fsub DWORD PTR [esi+((src+14)*4)]
                  fmul dct9t9
                  fstp DWORD PTR [edi+((dest+8)*4)]

                ENDIF

                ENDM

                ;
                ;a = (src1[0] + src2[0])
                ;b = (src1[9] + src2[9]) * f1
                ;
                ;dest1 = (a + b) * f2
                ;dest2 = (a - b) * f3
                ;

COMBINE_ADD     MACRO src1,src2,f1,f2,f3,dest1,dest2

                IFDEF AMD

                  movd mm0,[edi+(src1*4)]
                  movd mm1,[edi+((src1+9)*4)]
                  movd mm2,[edi+(src2*4)]
                  movd mm3,[edi+((src2+9)*4)]

                  movd mm4,f1
                  movd mm5,f2
                  movd mm6,f3

                  pfadd mm1,mm3
                  pfadd mm0,mm2
                  pfmul mm1,mm4

                  movq mm2,mm0
                  pfsub mm0,mm1
                  pfadd mm2,mm1
                  pfmul mm0,mm6
                  pfmul mm2,mm5
                  movd DWORD PTR [ebx+(dest2*4)],mm0
                  movd DWORD PTR [ebx+(dest1*4)],mm2
                  
                ELSE

                  fld DWORD PTR [edi+((src1+9)*4)]
                  fadd DWORD PTR [edi+((src2+9)*4)]
                  fld DWORD PTR [edi+(src1*4)]
                  fadd DWORD PTR [edi+(src2*4)]
                  fxch st(1)
                  fmul f1
                         
                  fld st(1)
                  fadd st,st(1)
                  fmul f2
                  fstp DWORD PTR [ebx+(dest1*4)]
                  
                  fsubp st(1),st
                  fmul f3
                  fstp DWORD PTR [ebx+(dest2*4)]

                ENDIF

                ENDM

                ;
                ;a = (src1[0] - src2[0])
                ;b = (src1[9] - src2[9]) * f1
                ;
                ;dest1 = (a + b) * f2
                ;dest2 = (a - b) * f3
                ;

COMBINE_SUB     MACRO src1,src2,f1,f2,f3,dest1,dest2

                IFDEF AMD

                  movd mm0,[edi+(src1*4)]
                  movd mm1,[edi+((src1+9)*4)]
                  movd mm2,[edi+(src2*4)]
                  movd mm3,[edi+((src2+9)*4)]

                  movd mm4,f1
                  movd mm5,f2
                  movd mm6,f3

                  pfsub mm1,mm3
                  pfsub mm0,mm2
                  pfmul mm1,mm4

                  movq mm2,mm0
                  pfsub mm0,mm1
                  pfadd mm2,mm1
                  pfmul mm0,mm6
                  pfmul mm2,mm5
                  movd DWORD PTR [ebx+(dest2*4)],mm0
                  movd DWORD PTR [ebx+(dest1*4)],mm2
                  
                ELSE

                  fld DWORD PTR [edi+((src1+9)*4)]
                  fsub DWORD PTR [edi+((src2+9)*4)]
                  fld DWORD PTR [edi+(src1*4)]
                  fsub DWORD PTR [edi+(src2*4)]
                  fxch st(1)
                  fmul f1
                         
                  fld st(1)
                  fadd st,st(1)
                  fmul f2
                  fstp DWORD PTR [ebx+(dest1*4)]
                  
                  fsubp st(1),st
                  fmul f3
                  fstp DWORD PTR [ebx+(dest2*4)]

                ENDIF
                  
                ENDM

                ;
                ;A = (-B * C) + D
                ;E = ( F * G) - H
                ;

WINDOW_0110     MACRO A,B,C,D,E,F,G,H

                IFDEF AMD

                  movd mm0,[ebx+(B*4)]
                  movd mm1,[esi+(C*4)]
                  movd mm2,[ecx+(D*4)]
                  movd mm3,[ebx+(F*4)]
                  movd mm4,[esi+(G*4)]
                  movd mm5,[ecx+(H*4)]
                  pfsubr mm0,mm7
                  pfmul mm3,mm4
                  pfmul mm0,mm1
                  pfsub mm3,mm5
                  pfadd mm0,mm2
                  movd [edi+(E*4)],mm3
                  movd [edi+(A*4)],mm0

                ELSE

                  fld DWORD PTR [ebx+(F*4)]
                  fld DWORD PTR [ebx+(B*4)]
                  fchs
                  fmul DWORD PTR [esi+(C*4)]
                  fadd DWORD PTR [ecx+(D*4)]
                  fxch st(1)

                  fmul DWORD PTR [esi+(G*4)]
                  fsub DWORD PTR [ecx+(H*4)]
                  fstp DWORD PTR [edi+(E*4)]
                  fstp DWORD PTR [edi+(A*4)]
                  
                ENDIF

                ENDM

                ;
                ;WINDOW_0100: A = (-B * C) + D
                ;             E = (-F * G) - H
                ;

WINDOW_0100     MACRO A,B,C,D,E,F,G,H

                IFDEF AMD

                  movd mm0,[ebx+(B*4)]
                  movd mm1,[esi+(C*4)]
                  movd mm2,[ecx+(D*4)]
                  movd mm3,[ebx+(F*4)]
                  movd mm4,[esi+(G*4)]
                  movd mm5,[ecx+(H*4)]
                  pfsubr mm3,mm7
                  pfsubr mm0,mm7
                  pfmul mm3,mm4
                  pfmul mm0,mm1
                  pfsub mm3,mm5
                  pfadd mm0,mm2
                  movd [edi+(E*4)],mm3
                  movd [edi+(A*4)],mm0

                ELSE

                  fld DWORD PTR [ebx+(F*4)]
                  fld DWORD PTR [ebx+(B*4)]
                  fchs
                  fmul DWORD PTR [esi+(C*4)]
                  fadd DWORD PTR [ecx+(D*4)]
                  fxch st(1)
                  
                  fchs
                  fmul DWORD PTR [esi+(G*4)]
                  fsub DWORD PTR [ecx+(H*4)]
                  fstp DWORD PTR [edi+(E*4)]
                  fstp DWORD PTR [edi+(A*4)]

                ENDIF

                ENDM

                ;
                ;WINDOW_1100: A = ( B * C) + D
                ;             E = (-F * G) - H
                ;

WINDOW_1100     MACRO A,B,C,D,E,F,G,H

                IFDEF AMD

                  movd mm0,[ebx+(B*4)]
                  movd mm1,[esi+(C*4)]
                  movd mm2,[ecx+(D*4)]
                  movd mm3,[ebx+(F*4)]
                  movd mm4,[esi+(G*4)]
                  pfsubr mm3,mm7
                  movd mm5,[ecx+(H*4)]
                  pfmul mm3,mm4
                  pfmul mm0,mm1
                  pfsub mm3,mm5
                  pfadd mm0,mm2
                  movd [edi+(E*4)],mm3
                  movd [edi+(A*4)],mm0

                ELSE

                  fld DWORD PTR [ebx+(F*4)]
                  fld DWORD PTR [ebx+(B*4)]
                  fmul DWORD PTR [esi+(C*4)]
                  fadd DWORD PTR [ecx+(D*4)]
                  fxch st(1)

                  fchs
                  fmul DWORD PTR [esi+(G*4)]
                  fsub DWORD PTR [ecx+(H*4)]
                  fstp DWORD PTR [edi+(E*4)]
                  fstp DWORD PTR [edi+(A*4)]

                ENDIF

                ENDM

                ;
                ;WINDOW_0101: A = (-B * C) + D
                ;             E = (-F * G) + H
                ;

WINDOW_0101     MACRO A,B,C,D,E,F,G,H

                IFDEF AMD

                  movd mm0,[ebx+(B*4)]
                  movd mm1,[esi+(C*4)]
                  movd mm2,[ecx+(D*4)]
                  movd mm3,[ebx+(F*4)]
                  movd mm4,[esi+(G*4)]
                  movd mm5,[ecx+(H*4)]
                  pfsubr mm3,mm7
                  pfsubr mm0,mm7
                  pfmul mm3,mm4
                  pfmul mm0,mm1
                  pfadd mm3,mm5
                  pfadd mm0,mm2
                  movd [edi+(E*4)],mm3
                  movd [edi+(A*4)],mm0

                ELSE

                  fld DWORD PTR [ebx+(F*4)]
                  fld DWORD PTR [ebx+(B*4)]
                  fchs
                  fmul DWORD PTR [esi+(C*4)]
                  fadd DWORD PTR [ecx+(D*4)]
                  fxch st(1)
                 
                  fchs
                  fmul DWORD PTR [esi+(G*4)]
                  fadd DWORD PTR [ecx+(H*4)]
                  fstp DWORD PTR [edi+(E*4)]
                  fstp DWORD PTR [edi+(A*4)]

                ENDIF

                ENDM

                ;
                ;WINDOW_0111: A = (-B * C) + D
                ;             E = ( F * G) + H
                ;

WINDOW_0111     MACRO A,B,C,D,E,F,G,H

                IFDEF AMD

                  movd mm0,[ebx+(B*4)]
                  movd mm1,[esi+(C*4)]
                  movd mm2,[ecx+(D*4)]
                  movd mm3,[ebx+(F*4)]
                  movd mm4,[esi+(G*4)]
                  movd mm5,[ecx+(H*4)]
                  pfsubr mm0,mm7
                  pfmul mm3,mm4
                  pfmul mm0,mm1
                  pfadd mm3,mm5
                  pfadd mm0,mm2
                  movd [edi+(E*4)],mm3
                  movd [edi+(A*4)],mm0

                ELSE

                  fld DWORD PTR [ebx+(F*4)]
                  fld DWORD PTR [ebx+(B*4)]
                  fchs
                  fmul DWORD PTR [esi+(C*4)]
                  fadd DWORD PTR [ecx+(D*4)]
                  fxch st(1)
                
                  fmul DWORD PTR [esi+(G*4)]
                  fadd DWORD PTR [ecx+(H*4)]
                  fstp DWORD PTR [edi+(E*4)]
                  fstp DWORD PTR [edi+(A*4)]

                ENDIF

                ENDM

                ;
                ;WINDOW_1111: A = ( B * C) + D
                ;             E = ( F * G) + H
                ;

WINDOW_1111     MACRO A,B,C,D,E,F,G,H

                IFDEF AMD

                  movd mm0,[ebx+(B*4)]
                  movd mm1,[esi+(C*4)]
                  movd mm2,[ecx+(D*4)]
                  movd mm3,[ebx+(F*4)]
                  movd mm4,[esi+(G*4)]
                  movd mm5,[ecx+(H*4)]
                  pfmul mm3,mm4
                  pfmul mm0,mm1
                  pfadd mm3,mm5
                  pfadd mm0,mm2
                  movd [edi+(E*4)],mm3
                  movd [edi+(A*4)],mm0

                ELSE

                  fld DWORD PTR [ebx+(F*4)]
                  fld DWORD PTR [ebx+(B*4)]
                  fmul DWORD PTR [esi+(C*4)]
                  fadd DWORD PTR [ecx+(D*4)]
                  fxch st(1)

                  fmul DWORD PTR [esi+(G*4)]
                  fadd DWORD PTR [ecx+(H*4)]
                  fstp DWORD PTR [edi+(E*4)]
                  fstp DWORD PTR [edi+(A*4)]

                ENDIF

                ENDM

                ;
                ;p1 = m1*m2
                ;p2 = m3*m4
                ;

MULT_PAIR       MACRO m1,m2,p1,m3,m4,p2

                IFDEF AMD

                  movd mm0,m1
                  movd mm1,m2
                  movd mm2,m3
                  movd mm3,m4
                  pfmul mm0,mm1
                  pfmul mm2,mm3
                  movd p1,mm0
                  movd p2,mm2

                ELSE

                  fld DWORD PTR m1
                  fld DWORD PTR m3
                  fmul DWORD PTR m4
                  fxch st(1)
                  fmul DWORD PTR m2
                  fxch st(1)
                  fstp DWORD PTR p2
                  fstp DWORD PTR p1

                ENDIF
                
                ENDM

                ;
                ;p1 = m1*m2
                ;

MULT            MACRO m1,m2,p1

                IFDEF AMD

                  movd mm0,m1
                  movd mm1,m2
                  pfmul mm0,mm1
                  movd p1,mm0

                ELSE

                  fld DWORD PTR m1
                  fmul DWORD PTR m2
                  fstp DWORD PTR p1

                ENDIF

                ENDM

                IFDEF AMD
AMD_IMDCT_1x36    PROC \
                  USES ebx esi edi \
                  lpIn:PTR,\
                  dwSB:DWORD,\
                  lpResult:PTR,\
                  lpSave:PTR,\
                  lpWindow:PTR
                ELSE
x86_IMDCT_1x36    PROC \
                  USES ebx esi edi \
                  lpIn:PTR,\
                  dwSB:DWORD,\
                  lpResult:PTR,\
                  lpSave:PTR,\
                  lpWindow:PTR
                ENDIF

                LOCAL a
                LOCAL b
                LOCAL temp1[18]
                LOCAL temp2[18]

                mov esi,[lpIn]
                lea edi,temp1
                lea ebx,temp2

                _FEMMS

                CASCADE_ADD 15,16,17
                CASCADE_ADD 13,14,15
                CASCADE_ADD 11,12,13
                CASCADE_ADD 9,10,11
                CASCADE_ADD 7,8,9
                CASCADE_ADD 5,6,7
                CASCADE_ADD 3,4,5
                CASCADE_ADD 1,2,3

                IFDEF AMD

                  movd mm0,[esi+(1*4)]
                  movd mm1,[esi+(0*4)]
                  pfadd mm0,mm1
                  movd [esi+(1*4)],mm0

                ELSE

                  fld DWORD PTR [esi+(1*4)]
                  fadd DWORD PTR [esi+(0*4)]
                  fstp DWORD PTR [esi+(1*4)]

                ENDIF

                CASCADE_ADD 13,15,17
                CASCADE_ADD 9,11,13
                CASCADE_ADD 5,7,9
                CASCADE_ADD 1,3,5

                IDCT_9 0,0
                IDCT_9 1,9

                COMBINE_ADD 0,5, dct36t0,dct36t1,dct36t2,    0,17
                COMBINE_ADD 1,8, dct36t3,dct36t4,dct36t5,    1,16
                COMBINE_ADD 2,6, dct36t6,dct36t7,dct36t8,    2,15
                COMBINE_ADD 3,7, dct36t9,dct36t10,dct36t11,  3,14

                COMBINE_SUB 3,7, dct36t12,dct36t13,dct36t14, 5,12
                COMBINE_SUB 2,6, dct36t15,dct36t16,dct36t17, 6,11
                COMBINE_SUB 1,8, dct36t18,dct36t19,dct36t20, 7,10
                COMBINE_SUB 0,5, dct36t21,dct36t22,dct36t23, 8,9

                IFDEF AMD

                  movd mm0,dct36t24
                  movd mm1,dct36t25
                  movd mm2,dct36t26

                  movd mm3,[edi+(13*4)]
                  movd mm4,[edi+(4*4)]

                  pfmul mm3,mm0
                  movq mm5,mm4

                  pfadd mm4,mm3
                  pfsub mm5,mm3
                  pfmul mm4,mm1
                  pfmul mm5,mm2

                  movd [ebx+(4*4)],mm4
                  movd [ebx+(13*4)],mm5

                  pxor mm7,mm7          ;(used to change sign in window routines below)

                ELSE

                  fld DWORD PTR [edi+(13*4)]
                  fmul dct36t24
                  fstp b
                         
                  fld DWORD PTR [edi+(4*4)]
                  fadd b
                  fmul dct36t25
                  fstp DWORD PTR [ebx+(4*4)]
                  
                  fld DWORD PTR [edi+(4*4)]
                  fsub b
                  fmul dct36t26
                  fstp DWORD PTR [ebx+(13*4)]

                ENDIF

                mov esi,[lpWindow]
                mov edi,[lpResult]
                mov ecx,[lpSave]

                test [dwSB],1
                jz __even_window

                WINDOW_0110 0,9,0,0,1,10,1,1
                WINDOW_0110 2,11,2,2,3,12,3,3
                WINDOW_0110 4,13,4,4,5,14,5,5
                WINDOW_0110 6,15,6,6,7,16,7,7
                WINDOW_0100 8,17,8,8,9,17,9,9
                WINDOW_1100 10,16,10,10,11,15,11,11
                WINDOW_1100 12,14,12,12,13,13,13,13
                WINDOW_1100 14,12,14,14,15,11,15,15
                WINDOW_1100 16,10,16,16,17,9,17,17
                jmp __save

__even_window:  WINDOW_0101 0,9,0,0,1,10,1,1
                WINDOW_0101 2,11,2,2,3,12,3,3
                WINDOW_0101 4,13,4,4,5,14,5,5
                WINDOW_0101 6,15,6,6,7,16,7,7
                WINDOW_0111 8,17,8,8,9,17,9,9
                WINDOW_1111 10,16,10,10,11,15,11,11
                WINDOW_1111 12,14,12,12,13,13,13,13
                WINDOW_1111 14,12,14,14,15,11,15,15
                WINDOW_1111 16,10,16,16,17,9,17,17

__save:         MULT_PAIR [ebx+(8*4)], [esi+(18*4)], [ecx+(0*4)],\
                          [ebx+(7*4)], [esi+(19*4)], [ecx+(1*4)]
                MULT_PAIR [ebx+(6*4)], [esi+(20*4)], [ecx+(2*4)],\
                          [ebx+(5*4)], [esi+(21*4)], [ecx+(3*4)]
                MULT_PAIR [ebx+(4*4)], [esi+(22*4)], [ecx+(4*4)],\
                          [ebx+(3*4)], [esi+(23*4)], [ecx+(5*4)]
                MULT_PAIR [ebx+(2*4)], [esi+(24*4)], [ecx+(6*4)],\
                          [ebx+(1*4)], [esi+(25*4)], [ecx+(7*4)]
                MULT_PAIR [ebx+(0*4)], [esi+(26*4)], [ecx+(8*4)],\
                          [ebx+(0*4)], [esi+(27*4)], [ecx+(9*4)]
                MULT_PAIR [ebx+(1*4)], [esi+(28*4)], [ecx+(10*4)],\
                          [ebx+(2*4)], [esi+(29*4)], [ecx+(11*4)]
                MULT_PAIR [ebx+(3*4)], [esi+(30*4)], [ecx+(12*4)],\
                          [ebx+(4*4)], [esi+(31*4)], [ecx+(13*4)]
                MULT_PAIR [ebx+(5*4)], [esi+(32*4)], [ecx+(14*4)],\
                          [ebx+(6*4)], [esi+(33*4)], [ecx+(15*4)]
                MULT_PAIR [ebx+(7*4)], [esi+(34*4)], [ecx+(16*4)],\
                          [ebx+(8*4)], [esi+(35*4)], [ecx+(17*4)]

                _FEMMS

                ret

                IFDEF AMD
AMD_IMDCT_1x36    ENDP
                ELSE
x86_IMDCT_1x36    ENDP
                ENDIF

;#############################################################################
;##                                                                         ##
;## 3x12 IMDCT for window type 2 (short windows)                            ##
;##                                                                         ##
;#############################################################################

                ;
                ;dest += src
                ;

ACCUM           MACRO dest,src

                IFDEF AMD

                  movd mm0,dest
                  movd mm1,src
                  pfadd mm0,mm1
                  movd dest,mm0

                ELSE

                  fld DWORD PTR dest
                  fadd DWORD PTR src
                  fstp DWORD PTR dest

                ENDIF

                ENDM

                ;
                ;dest1 += src1
                ;dest2 += src2
                ;

ACCUM_PAIR      MACRO dest1,src1,dest2,src2

                IFDEF AMD

                  movd mm0,dest1
                  movd mm1,src1
                  movd mm2,dest2
                  movd mm3,src2
                  pfadd mm0,mm1
                  pfadd mm2,mm3
                  movd dest1,mm0
                  movd dest2,mm2

                ELSE

                  fld DWORD PTR dest1
                  fld DWORD PTR dest2
                  fadd DWORD PTR src2
                  fxch st(1)
                  fadd DWORD PTR src1
                  fxch st(1)
                  fstp DWORD PTR dest2
                  fstp DWORD PTR dest1

                ENDIF

                ENDM

                ;
                ;t1 = in2 * cos(pi/6)
                ;t2 = in1 * sin(pi/6)
                ;t3 = in3 + t2;
                ;out1 = in3 - in1;
                ;out2 = t3 + t1;
                ;out3 = t3 - t1;
                ;

IDCT_3          MACRO in1,in2,in3,out1,out2,out3

                IFDEF AMD

                  movd mm3,in3
                  movd mm2,in2
                  movd mm1,in1
                  movq mm5,mm3
                  movd mm6,sin30
                  movd mm7,cos30
                  pfsub mm5,mm1
                  pfmul mm1,mm6 ;mm1=t2
                  pfmul mm2,mm7 ;mm2=t1
                  pfadd mm3,mm1 ;mm3=t3
                  movq mm0,mm2
                  pfadd mm2,mm3
                  pfsub mm3,mm0
                  movd out1,mm5
                  movd out2,mm2
                  movd out3,mm3

                ELSE

                  fld DWORD PTR in1
                  fld DWORD PTR in2
                  fmul cos30
                  fxch st(1)
                  fmul sin30
                  fld DWORD PTR in3
                  faddp st(1),st
                  fdup
                  fld DWORD PTR in3
                  fsub DWORD PTR in1
                  fstp DWORD PTR out1
                  fadd st,st(2)   
                  fstp DWORD PTR out2
                  fsubrp st(1),st
                  fstp DWORD PTR out3

                ENDIF

                ENDM

                IFDEF AMD
AMD_IMDCT_3x12    PROC \
                  USES ebx esi edi \
                  lpIn:PTR,\
                  dwSB:DWORD,\
                  lpResult:PTR,\
                  lpSave:PTR
                ELSE
x86_IMDCT_3x12    PROC \
                  USES ebx esi edi \
                  lpIn:PTR,\
                  dwSB:DWORD,\
                  lpResult:PTR,\
                  lpSave:PTR
                ENDIF

                LOCAL loop_end
                LOCAL temp[16]
                LOCAL output[36]

                fldz
                I = 0
                REPT 17

                  fst QWORD PTR output[I]

                I = I + 8
                ENDM
                fstp QWORD PTR output[I]

                _FEMMS

                mov esi,[lpIn]
                lea edi,output
                
                lea eax,[esi][18 * SIZE DWORD]
                mov loop_end,eax

__for_DCT:      ACCUM_PAIR [esi+(5*4)], [esi+(4*4)], [esi+(4*4)], [esi+(3*4)]
                ACCUM_PAIR [esi+(3*4)], [esi+(2*4)], [esi+(2*4)], [esi+(1*4)]
                ACCUM_PAIR [esi+(1*4)], [esi+(0*4)], [esi+(5*4)], [esi+(3*4)]
                ACCUM      [esi+(3*4)], [esi+(1*4)]

                IDCT_3 [esi+(4*4)], [esi+(2*4)], [esi+(0*4)], temp[1*4], temp[0*4], temp[2*4]
                IDCT_3 [esi+(5*4)], [esi+(3*4)], [esi+(1*4)], temp[4*4], temp[5*4], temp[3*4]

                MULT_PAIR temp[3*4],dct3t1,temp[3*4],temp[4*4],dct3t2,temp[4*4]
                MULT      temp[5*4],dct3t3,temp[5*4]

                IFDEF AMD

                  movd mm0,temp[0*4]
                  movd mm1,temp[1*4]
                  movd mm2,temp[2*4]
                  movd mm6,temp[5*4]
                  movd mm7,temp[4*4]
                  movq mm3,mm0
                  movq mm4,mm1
                  movq mm5,mm2
                  pfadd mm0,mm6
                  pfsub mm3,mm6
                  movd temp[0*4],mm0
                  movd temp[5*4],mm3
                  pfadd mm1,mm7
                  pfsub mm4,mm7
                  movd mm6,temp[3*4]
                  movd temp[1*4],mm1
                  movd temp[4*4],mm4
                  pfadd mm2,mm6
                  pfsub mm5,mm6
                  movd temp[2*4],mm2
                  movd temp[3*4],mm5

                ELSE

                  fld DWORD PTR temp[0*4]
                  fdup
                  fadd DWORD PTR temp[5*4]
                  fstp DWORD PTR temp[0*4]
                  fsub DWORD PTR temp[5*4]
                  fstp DWORD PTR temp[5*4]
                  
                  fld DWORD PTR temp[1*4]
                  fdup
                  fadd DWORD PTR temp[4*4]
                  fstp DWORD PTR temp[1*4]
                  fsub DWORD PTR temp[4*4]
                  fstp DWORD PTR temp[4*4]
                  
                  fld DWORD PTR temp[2*4]
                  fdup
                  fadd DWORD PTR temp[3*4]
                  fstp DWORD PTR temp[2*4]
                  fsub DWORD PTR temp[3*4]
                  fstp DWORD PTR temp[3*4]

                ENDIF

                MULT_PAIR temp[0*4],dct3t4,temp[0*4],temp[1*4],dct3t5,temp[1*4]
                MULT_PAIR temp[2*4],dct3t6,temp[2*4],temp[3*4],dct3t7,temp[3*4]
                MULT_PAIR temp[4*4],dct3t8,temp[4*4],temp[5*4],dct3t9,temp[5*4]

                MULT_PAIR dct3t10,temp[0*4],temp[8*4],dct3t11,temp[0*4],temp[9*4]
                MULT_PAIR dct3t12,temp[1*4],temp[7*4],dct3t13,temp[1*4],temp[10*4]
                MULT_PAIR dct3t14,temp[2*4],temp[6*4],dct3t15,temp[2*4],temp[11*4]
                MULT_PAIR dct3t16,temp[3*4],temp[0*4],dct3t17,temp[4*4],temp[1*4] 
                MULT_PAIR dct3t18,temp[5*4],temp[2*4],dct3t19,temp[5*4],temp[3*4] 
                MULT_PAIR dct3t20,temp[4*4],temp[4*4],dct3t21,temp[0*4],temp[5*4] 
                MULT      dct3t22,temp[0*4],temp[0*4]

                ACCUM_PAIR [edi+(6*4)], temp[0*4], [edi+(7*4)], temp[1*4]
                ACCUM_PAIR [edi+(8*4)], temp[2*4], [edi+(9*4)], temp[3*4]
                ACCUM_PAIR [edi+(10*4)],temp[4*4], [edi+(11*4)],temp[5*4]
                ACCUM_PAIR [edi+(12*4)],temp[6*4], [edi+(13*4)],temp[7*4]
                ACCUM_PAIR [edi+(14*4)],temp[8*4], [edi+(15*4)],temp[9*4]
                ACCUM_PAIR [edi+(16*4)],temp[10*4],[edi+(17*4)],temp[11*4]

                add esi,6 * SIZE DWORD
                add edi,6 * SIZE DWORD
                cmp esi,loop_end
                jne __for_DCT

                _FEMMS

                lea esi,output
                mov edi,[lpResult]
                mov ebx,[lpSave]

                lea eax,[esi][18 * SIZE DWORD]
                mov loop_end,eax

                test [dwSB],1
                jz __even_overlap

__odd_overlap:  
                I = 0
                REPT 9
                
                  fld DWORD PTR [esi+I]
                  fadd DWORD PTR [ebx+I]
                  fstp DWORD PTR [edi+I]
                
                  fld DWORD PTR [esi+SIZE DWORD+I]
                  fchs
                  fsub DWORD PTR [ebx+SIZE DWORD+I]
                  fstp DWORD PTR [edi+SIZE DWORD+I]

                I = I + 8
                ENDM
                jmp __overlap_done

__even_overlap: 
                I = 0
                REPT 18

                  fld DWORD PTR [esi+I]
                  fadd DWORD PTR [ebx+I]
                  fstp DWORD PTR [edi+I]

                I = I + 4
                ENDM

__overlap_done: add ebx,18 * SIZE DWORD
                add esi,18 * SIZE DWORD
                add edi,18 * SIZE DWORD

                I = 0
                REPT 18

                  fld DWORD PTR [esi+I]
                  fstp DWORD PTR [ebx+I-(18 * SIZE DWORD)]

                I = I + 4
                ENDM
                ret

                IFDEF AMD
AMD_IMDCT_3x12    ENDP
                ELSE
x86_IMDCT_3x12    ENDP
                ENDIF

;#############################################################################
;##                                                                         ##
;## IDCT reordering and butterflies for polyphase filter                    ##
;##                                                                         ##
;#############################################################################

                ;
                ;B = (A - C) * D; d1 = A + C;
                ;F = (E - G) * H; d2 = E + G;
                ;
                ;Separation between successive A,C,E,G elements determined
                ;by stride -- 18 in the case of the initial r[][18] array,
                ;1 in the case of the intermediate arrays
                ;

                IFDEF AMD

REORD_PAIR      MACRO stride,dest1,A,B,C,D,dest2,E,F,G,H

                movd mm1,DWORD PTR [esi+(A*stride*4)]
                movd mm3,DWORD PTR [esi+(C*stride*4)]
                movd mm4,DWORD PTR [ebx+(D*4)]
                movq mm2,mm1
                pfsub mm1,mm3

                movd mm5,DWORD PTR [esi+(C*stride*4)]
                movd mm6,DWORD PTR [esi+(G*stride*4)]

                pfmul mm1,mm4
                pfadd mm2,mm5

                movd DWORD PTR [edi+(B*4)],mm1
                movd DWORD PTR [edi+(dest1*4)],mm2
                
                movd mm1,DWORD PTR [esi+(E*stride*4)]
                movd mm7,DWORD PTR [ebx+(H*4)]
                movq mm2,mm1
                pfsub mm1,mm6
                movd mm0,DWORD PTR [esi+(G*stride*4)]

                pfmul mm1,mm7
                pfadd mm2,mm0

                movd DWORD PTR [edi+(F*4)],mm1
                movd DWORD PTR [edi+(dest2*4)],mm2

                ENDM

                ELSE

REORD_PAIR      MACRO stride,dest1,A,B,C,D,dest2,E,F,G,H

                fld DWORD PTR [esi+(E*stride*4)]    
                fdup
                fld DWORD PTR [esi+(A*stride*4)]    
                fdup                            

                fsub DWORD PTR [esi+(C*stride*4)]   
                fmul DWORD PTR [ebx+(D*4)]
                fxch st(2)
                fsub DWORD PTR [esi+(G*stride*4)]   
                fmul DWORD PTR [ebx+(H*4)]
                fxch st(2)                     

                fstp DWORD PTR [edi+(B*4)]
                fadd DWORD PTR [esi+(C*stride*4)]
                fstp DWORD PTR [edi+(dest1*4)]

                fstp DWORD PTR [edi+(F*4)]
                fadd DWORD PTR [esi+(G*stride*4)]
                fstp DWORD PTR [edi+(dest2*4)]

                ENDM

                ENDIF

                IFDEF AMD
AMD_poly_filter   PROC \
                  USES ebx esi edi \
                  lpSrc:PTR,\
                  lpBArray:PTR,\
                  dwPhase:DWORD,\
                  lpOut1:PTR,\
                  lpOut2:PTR
                ELSE
x86_poly_filter   PROC \
                  USES ebx esi edi \
                  lpSrc:PTR,\
                  lpBArray:PTR,\
                  dwPhase:DWORD,\
                  lpOut1:PTR,\
                  lpOut2:PTR
                ENDIF

                LOCAL temp1[32]
                LOCAL temp2[16]

                _FEMMS

                mov eax,[dwPhase]
                mov esi,[lpSrc]
                lea edi,temp1
                mov ebx,[lpBArray]
                lea esi,[esi][eax*4]

                REORD_PAIR 18, 0,0,16,31,1,    1,1,17,30,3
                REORD_PAIR 18, 3,2,19,29,5,    2,3,18,28,7
                REORD_PAIR 18, 6,4,22,27,9,    7,5,23,26,11
                REORD_PAIR 18, 5,6,21,25,13,   4,7,20,24,15
                REORD_PAIR 18, 12,8,28,23,17,  13,9,29,22,19
                REORD_PAIR 18, 15,10,31,21,21, 14,11,30,20,23
                REORD_PAIR 18, 10,12,26,19,25, 11,13,27,18,27
                REORD_PAIR 18, 9,14,25,17,29,  8,15,24,16,31

                lea edi,temp2
                lea esi,temp1

                REORD_PAIR 1, 0,0,8,8,2,     1,1,9,9,6
                REORD_PAIR 1, 2,2,10,10,14,  3,3,11,11,10
                REORD_PAIR 1, 4,4,12,12,30,  5,5,13,13,26
                REORD_PAIR 1, 6,6,14,14,18,  7,7,15,15,22

                lea edi,temp1
                lea esi,temp2

                REORD_PAIR 1, 0,0,4,4,4,     1,1,5,5,12
                REORD_PAIR 1, 2,2,6,6,28,    3,3,7,7,20
                REORD_PAIR 1, 8,8,12,12,4,   9,9,13,13,12
                REORD_PAIR 1, 10,10,14,14,   28,11,11,15,15,20

                lea edi,temp2
                lea esi,temp1

                REORD_PAIR 1, 0,0,2,2,8,     1,1,3,3,24
                REORD_PAIR 1, 4,4,6,6,8,     5,5,7,7,24
                REORD_PAIR 1, 8,8,10,10,8,   9,9,11,11,24
                REORD_PAIR 1, 12,12,14,14,8, 13,13,15,15,24

                IFDEF AMD

                  movd mm7,[ebx+(16*4)]
                  movd mm0,[edi+(0*4)]
                  movd mm1,[edi+(1*4)]
                  pxor mm2,mm2
                  movd mm3,[edi+(0*4)]
                  pfsub mm2,mm0
                  movd mm4,[edi+(1*4)]
                  pfsub mm2,mm1
                  pfsub mm3,mm4
                  pfadd mm2,mm2
                  pfmul mm3,mm7
                  movd [esi+(0*4)],mm2
                  movd [esi+(1*4)],mm3

                  movd mm0,[edi+(2*4)]
                  movd mm1,[edi+(3*4)]
                  movq mm2,mm0
                  pfadd mm0,mm1
                  movd mm3,[edi+(4*4)]
                  movd [esi+(2*4)],mm0
                  pfsub mm2,mm1
                  movd mm4,[edi+(5*4)]
                  pfmul mm2,mm7
                  movq mm5,mm3
                  pfsub mm2,mm0
                  pfadd mm3,mm4
                  movd [esi+(3*4)],mm2
                  movd [esi+(4*4)],mm3

                  movd mm0,[edi+(6*4)]
                  pfsub mm5,mm4
                  movd mm1,[edi+(7*4)]
                  pfmul mm5,mm7
                  pfadd mm0,mm1
                  pfadd mm5,mm3
                  movd [esi+(5*4)],mm5
                  movd [esi+(6*4)],mm0

                  movd mm0,[edi+(8*4)]
                  movd mm1,[edi+(9*4)]
                  movq mm2,mm0
                  movd mm5,[edi+(6*4)]
                  pfadd mm0,mm1
                  movd mm6,[edi+(7*4)]
                  movd [esi+(8*4)],mm0
                  pfsub mm2,mm1
                  movd mm4,[esi+(5*4)]
                  pfmul mm2,mm7
                  pfsub mm5,mm6
                  movd [esi+(9*4)],mm2
                  pfmul mm5,mm7
                  pfadd mm2,mm0
                  pfsub mm5,mm4
                  movd mm3,[edi+(10*4)]
                  movd mm4,[edi+(11*4)]
                  pfsub mm3,mm4
                  movd [esi+(7*4)],mm5
                  pfmul mm3,mm7
                  pfadd mm3,mm2
                  movd [esi+(11*4)],mm3

                  movd mm0,[edi+(10*4)]
                  movd mm1,[edi+(11*4)]
                  movd mm2,[edi+(12*4)]
                  movd mm3,[edi+(13*4)]
                  movq mm4,mm2
                  pfadd mm2,mm3
                  pfadd mm0,mm1
                  movd [esi+(12*4)],mm2
                  pfsub mm4,mm3
                  movd [esi+(10*4)],mm0
                  pfmul mm4,mm7
                  pfadd mm4,mm2
                  movd mm5,[esi+(8*4)]
                  movd mm6,[esi+(9*4)]
                  pfsub mm4,mm5
                  pfsub mm4,mm6
                  movd [esi+(13*4)],mm4

                  movd mm0,[edi+(14*4)]
                  movd mm1,[edi+(15*4)]
                  movd mm2,[esi+(8*4)]
                  movd mm3,[esi+(10*4)]
                  movd mm5,[esi+(11*4)]
                  movq mm4,mm0
                  pfadd mm0,mm1
                  pfsub mm4,mm1
                  pfsub mm0,mm2
                  pfmul mm4,mm7
                  pfsub mm0,mm3
                  pfsub mm4,mm5
                  movd [esi+(14*4)],mm0
                  movd [esi+(15*4)],mm4

                  mov edi,[lpOut1]

                  movd mm0,[esi+(1*4)]
                  movd mm1,[esi+(9*4)]
                  movd mm2,[esi+(14*4)]
                  movd [edi+(0*16*4)],mm0
                  pfsub mm1,mm2
                  movd [edi+(2*16*4)],mm1

                  movd mm0,[esi+(5*4)]
                  movd mm1,[esi+(13*4)]
                  movd mm2,[esi+(6*4)]
                  movd mm3,[esi+(10*4)]
                  pfsub mm0,mm2
                  pfsub mm1,mm3
                  movd [edi+(4*16*4)],mm0
                  movd [edi+(6*16*4)],mm1

                  movd mm0,[esi+(3*4)]
                  movd mm1,[esi+(11*4)]
                  movd mm2,[esi+(7*4)]
                  movd mm3,[esi+(8*4)]
                  movd mm4,[esi+(9*4)]
                  pfsub mm1,mm3
                  movd mm5,[esi+(13*4)]
                  pfsub mm1,mm4
                  movd [edi+(8*16*4)],mm0
                  pfsub mm1,mm5
                  movd [edi+(12*16*4)],mm2
                  movd [edi+(10*16*4)],mm1

                  movd mm0,[esi+(15*4)]
                  movd mm1,[esi+(0*4)]
                  movd [edi+(14*16*4)],mm0

                  mov edi,[lpOut2]

                  pxor mm6,mm6
                  movd [edi+(16*16*4)],mm1

                  movd mm0,[esi+(8*4)]
                  movd mm3,[esi+(12*4)]
                  movq mm2,mm0
                  movq mm4,mm0
                  movd mm1,[esi+(4*4)]
                  pfsubr mm0,mm6
                  pfsubr mm1,mm6
                  movd [edi+(14*16*4)],mm0

                  pfsub mm2,mm3
                  movd mm0,[esi+(2*4)]
                  movd [edi+(10*16*4)],mm2
                  pfsubr mm0,mm6

                  movd mm5,[esi+(10*4)]
                  pfsub mm3,mm4
                  movd [edi+(8*16*4)],mm0
                  pfsub mm3,mm5
                  movd [edi+(12*16*4)],mm1
                  movd [edi+(6*16*4)],mm3

                  movd mm0,[esi+(4*4)]
                  movd mm1,[esi+(6*4)]
                  movd mm2,[esi+(14*4)]
                  movd mm3,[esi+(1*4)]
                  pfsub mm0,mm1
                  pfsubr mm2,mm6
                  pfsubr mm3,mm6
                  movd [edi+(4*16*4)],mm0
                  movd [edi+(2*16*4)],mm2
                  movd [edi+(0*16*4)],mm3

                ELSE

                  fld DWORD PTR [edi+(0*4)]
                  fchs
                  fsub DWORD PTR [edi+(1*4)]
                  fadd st,st(0)
                  fstp DWORD PTR [esi+(0*4)]

                  fld DWORD PTR [edi+(0*4)]
                  fsub DWORD PTR [edi+(1*4)]
                  fmul DWORD PTR [ebx+(16*4)]
                  fstp DWORD PTR [esi+(1*4)]

                  fld DWORD PTR [edi+(2*4)]
                  fadd DWORD PTR [edi+(3*4)]
                  fld DWORD PTR [edi+(2*4)]
                  fsub DWORD PTR [edi+(3*4)]
                  fmul DWORD PTR [ebx+(16*4)]
                  fsub st,st(1)
                  fstp DWORD PTR [esi+(3*4)]
                  fstp DWORD PTR [esi+(2*4)]

                  fld DWORD PTR [edi+(4*4)]
                  fadd DWORD PTR [edi+(5*4)]
                  fld DWORD PTR [edi+(4*4)]
                  fsub DWORD PTR [edi+(5*4)]
                  fmul DWORD PTR [ebx+(16*4)]
                  fadd st,st(1)
                  fstp DWORD PTR [esi+(5*4)]
                  fstp DWORD PTR [esi+(4*4)]

                  fld DWORD PTR [edi+(6*4)]
                  fadd DWORD PTR [edi+(7*4)]
                  fstp DWORD PTR [esi+(6*4)]

                  fld DWORD PTR [edi+(6*4)]
                  fsub DWORD PTR [edi+(7*4)]
                  fmul DWORD PTR [ebx+(16*4)]
                  fsub DWORD PTR [esi+(5*4)]
                  fstp DWORD PTR [esi+(7*4)]

                  fld DWORD PTR [edi+(8*4)]
                  fadd DWORD PTR [edi+(9*4)]
                  fst DWORD PTR [esi+(8*4)]
                  fld DWORD PTR [edi+(8*4)]
                  fsub DWORD PTR [edi+(9*4)]
                  fmul DWORD PTR [ebx+(16*4)]
                  fst DWORD PTR [esi+(9*4)]
                  faddp st(1),st
                  fld DWORD PTR [edi+(10*4)]
                  fsub DWORD PTR [edi+(11*4)]
                  fmul DWORD PTR [ebx+(16*4)]
                  faddp st(1),st
                  fstp DWORD PTR [esi+(11*4)]

                  fld DWORD PTR [edi+(10*4)]
                  fadd DWORD PTR [edi+(11*4)]
                  fstp DWORD PTR [esi+(10*4)]

                  fld DWORD PTR [edi+(12*4)]
                  fadd DWORD PTR [edi+(13*4)]
                  fstp DWORD PTR [esi+(12*4)]
                  fld DWORD PTR [edi+(12*4)]
                  fsub DWORD PTR [edi+(13*4)]
                  fmul DWORD PTR [ebx+(16*4)]
                  fadd DWORD PTR [esi+(12*4)]
                  fsub DWORD PTR [esi+(8*4)]
                  fsub DWORD PTR [esi+(9*4)]
                  fstp DWORD PTR [esi+(13*4)]

                  fld DWORD PTR [edi+(14*4)]
                  fadd DWORD PTR [edi+(15*4)]
                  fsub DWORD PTR [esi+(8*4)]
                  fsub DWORD PTR [esi+(10*4)]
                  fstp DWORD PTR [esi+(14*4)]

                  fld DWORD PTR [edi+(14*4)]
                  fsub DWORD PTR [edi+(15*4)]
                  fmul DWORD PTR [ebx+(16*4)]
                  fsub DWORD PTR [esi+(11*4)]
                  fstp DWORD PTR [esi+(15*4)]

                  mov edi,[lpOut1]

                  fld DWORD PTR [esi+(1*4)]
                  fstp DWORD PTR [edi+(0*16*4)]

                  fld DWORD PTR [esi+(9*4)]
                  fsub DWORD PTR [esi+(14*4)]
                  fstp DWORD PTR [edi+(2*16*4)]

                  fld DWORD PTR [esi+(5*4)]
                  fsub DWORD PTR [esi+(6*4)]
                  fstp DWORD PTR [edi+(4*16*4)]

                  fld DWORD PTR [esi+(13*4)]
                  fsub DWORD PTR [esi+(10*4)]
                  fstp DWORD PTR [edi+(6*16*4)]

                  fld DWORD PTR [esi+(3*4)]
                  fstp DWORD PTR [edi+(8*16*4)]

                  fld DWORD PTR [esi+(11*4)]
                  fsub DWORD PTR [esi+(8*4)]
                  fsub DWORD PTR [esi+(9*4)]
                  fsub DWORD PTR [esi+(13*4)]
                  fstp DWORD PTR [edi+(10*16*4)]

                  fld DWORD PTR [esi+(7*4)]
                  fstp DWORD PTR [edi+(12*16*4)]

                  fld DWORD PTR [esi+(15*4)]
                  fstp DWORD PTR [edi+(14*16*4)]

                  mov edi,[lpOut2]

                  fld DWORD PTR [esi+(0*4)]
                  fstp DWORD PTR [edi+(16*16*4)]

                  fld DWORD PTR [esi+(8*4)]
                  fchs
                  fstp DWORD PTR [edi+(14*16*4)]

                  fld DWORD PTR [esi+(4*4)]
                  fchs
                  fstp DWORD PTR [edi+(12*16*4)]

                  fld DWORD PTR [esi+(8*4)]
                  fsub DWORD PTR [esi+(12*4)]
                  fstp DWORD PTR [edi+(10*16*4)]

                  fld DWORD PTR [esi+(2*4)]
                  fchs
                  fstp DWORD PTR [edi+(8*16*4)]

                  fld DWORD PTR [esi+(12*4)]
                  fsub DWORD PTR [esi+(8*4)]
                  fsub DWORD PTR [esi+(10*4)]
                  fstp DWORD PTR [edi+(6*16*4)]

                  fld DWORD PTR [esi+(4*4)]
                  fsub DWORD PTR [esi+(6*4)]
                  fstp DWORD PTR [edi+(4*16*4)]

                  fld DWORD PTR [esi+(14*4)]
                  fchs
                  fstp DWORD PTR [edi+(2*16*4)]

                  fld DWORD PTR [esi+(1*4)]
                  fchs
                  fstp DWORD PTR [edi+(0*16*4)]

                ENDIF

                lea edi,temp2
                lea esi,temp1

                REORD_PAIR 1, 0,16,8,24,2,   1,17,9,25,6
                REORD_PAIR 1, 2,18,10,26,14, 3,19,11,27,10
                REORD_PAIR 1, 4,20,12,28,30, 5,21,13,29,26
                REORD_PAIR 1, 6,22,14,30,18, 7,23,15,31,22

                lea edi,temp1
                lea esi,temp2

                REORD_PAIR 1, 16,0,20,4,4,    17,1,21,5,12
                REORD_PAIR 1, 18,2,22,6,28,   19,3,23,7,20
                REORD_PAIR 1, 24,8,28,12,4,   25,9,29,13,12
                REORD_PAIR 1, 26,10,30,14,28, 27,11,31,15,20

                lea edi,temp2
                lea esi,temp1

                REORD_PAIR 1, 0,16,2,18,8, 1,17,3,19,24
                REORD_PAIR 1, 4,20,6,22,8, 5,21,7,23,24
                REORD_PAIR 1, 8,24,10,26,8, 9,25,11,27,24
                REORD_PAIR 1, 12,28,14,30,8, 13,29,15,31,24

                _FEMMS

                fld DWORD PTR [edi+(0*4)]
                fadd DWORD PTR [edi+(1*4)]
                fstp DWORD PTR [esi+(16*4)]

                fld DWORD PTR [edi+(0*4)]
                fsub DWORD PTR [edi+(1*4)]
                fmul DWORD PTR [ebx+(16*4)]
                fstp DWORD PTR [esi+(17*4)]

                fld DWORD PTR [edi+(2*4)]
                fadd DWORD PTR [edi+(3*4)]
                fstp DWORD PTR [esi+(18*4)]

                fld DWORD PTR [edi+(2*4)]
                fsub DWORD PTR [edi+(3*4)]
                fmul DWORD PTR [ebx+(16*4)]
                fstp DWORD PTR [esi+(19*4)]

                fld DWORD PTR [edi+(4*4)]
                fadd DWORD PTR [edi+(5*4)]
                fadd DWORD PTR [esi+(16*4)]
                fstp DWORD PTR [esi+(20*4)]

                fld DWORD PTR [edi+(4*4)]
                fsub DWORD PTR [edi+(5*4)]
                fmul DWORD PTR [ebx+(16*4)]
                fadd DWORD PTR [esi+(17*4)]
                fstp DWORD PTR [esi+(21*4)]
                
                fld DWORD PTR [edi+(6*4)]
                fadd DWORD PTR [edi+(7*4)]
                fadd DWORD PTR [esi+(16*4)]
                fadd DWORD PTR [esi+(18*4)]
                fstp DWORD PTR [esi+(22*4)]

                fld DWORD PTR [edi+(6*4)]
                fsub DWORD PTR [edi+(7*4)]
                fmul DWORD PTR [ebx+(16*4)]
                fadd DWORD PTR [esi+(16*4)]
                fadd DWORD PTR [esi+(17*4)]
                fadd DWORD PTR [esi+(19*4)]
                fstp DWORD PTR [esi+(23*4)]

                fld DWORD PTR [edi+(8*4)]
                fadd DWORD PTR [edi+(9*4)]
                fstp DWORD PTR [esi+(24*4)]

                fld DWORD PTR [edi+(8*4)]
                fsub DWORD PTR [edi+(9*4)]
                fmul DWORD PTR [ebx+(16*4)]
                fstp DWORD PTR [esi+(25*4)]

                fld DWORD PTR [edi+(10*4)]
                fadd DWORD PTR [edi+(11*4)]
                fadd DWORD PTR [esi+(24*4)]
                fstp DWORD PTR [esi+(26*4)]

                fld DWORD PTR [edi+(10*4)]
                fsub DWORD PTR [edi+(11*4)]
                fmul DWORD PTR [ebx+(16*4)]
                fadd DWORD PTR [esi+(24*4)]
                fadd DWORD PTR [esi+(25*4)]
                fstp DWORD PTR [esi+(27*4)]

                fld DWORD PTR [edi+(12*4)]
                fadd DWORD PTR [edi+(13*4)]
                fsub DWORD PTR [esi+(20*4)]
                fstp DWORD PTR [esi+(28*4)]

                fld DWORD PTR [edi+(12*4)]
                fsub DWORD PTR [edi+(13*4)]
                fmul DWORD PTR [ebx+(16*4)]
                fadd DWORD PTR [esi+(28*4)]
                fsub DWORD PTR [esi+(21*4)]
                fstp DWORD PTR [esi+(29*4)]

                fld DWORD PTR [edi+(14*4)]
                fadd DWORD PTR [edi+(15*4)]
                fsub DWORD PTR [esi+(22*4)]
                fstp DWORD PTR [esi+(30*4)]

                fld DWORD PTR [edi+(14*4)]
                fsub DWORD PTR [edi+(15*4)]
                fmul DWORD PTR [ebx+(16*4)]
                fsub DWORD PTR [esi+(23*4)]
                fstp DWORD PTR [esi+(31*4)]

                mov edi,[lpOut2]

                fld DWORD PTR [esi+(30*4)]
                fchs
                fstp DWORD PTR [edi+(1*16*4)]

                fld DWORD PTR [esi+(26*4)]
                fsub DWORD PTR [esi+(22*4)]
                fstp DWORD PTR [edi+(3*16*4)]

                fld DWORD PTR [esi+(18*4)]
                fadd DWORD PTR [esi+(20*4)]
                fsub DWORD PTR [esi+(26*4)]
                fstp DWORD PTR [edi+(5*16*4)]

                fld DWORD PTR [esi+(28*4)]
                fsub DWORD PTR [esi+(18*4)]
                fstp DWORD PTR [edi+(7*16*4)]

                fld DWORD PTR [esi+(28*4)]
                fchs
                fstp DWORD PTR [edi+(9*16*4)]

                fld DWORD PTR [esi+(24*4)]
                fsub DWORD PTR [esi+(20*4)]
                fstp DWORD PTR [edi+(11*16*4)]

                fld DWORD PTR [esi+(16*4)]
                fsub DWORD PTR [esi+(24*4)]
                fstp DWORD PTR [edi+(13*16*4)]

                fld DWORD PTR [esi+(16*4)]
                fchs
                fstp DWORD PTR [edi+(15*16*4)]

                mov edi,[lpOut1]

                fld DWORD PTR [esi+(31*4)]
                fstp DWORD PTR [edi+(15*16*4)]

                fld DWORD PTR [esi+(23*4)]
                fsub DWORD PTR [esi+(27*4)]
                fstp DWORD PTR [edi+(13*16*4)]

                fld DWORD PTR [esi+(27*4)]
                fsub DWORD PTR [esi+(19*4)]
                fsub DWORD PTR [esi+(20*4)]
                fsub DWORD PTR [esi+(21*4)]
                fstp DWORD PTR [edi+(11*16*4)]

                fld DWORD PTR [esi+(19*4)]
                fsub DWORD PTR [esi+(29*4)]
                fstp DWORD PTR [edi+(9*16*4)]

                fld DWORD PTR [esi+(29*4)]
                fsub DWORD PTR [esi+(18*4)]
                fstp DWORD PTR [edi+(7*16*4)]
                
                fld DWORD PTR [esi+(18*4)]
                fadd DWORD PTR [esi+(20*4)]
                fadd DWORD PTR [esi+(21*4)]
                fsub DWORD PTR [esi+(25*4)]
                fsub DWORD PTR [esi+(26*4)]
                fstp DWORD PTR [edi+(5*16*4)]

                fld DWORD PTR [esi+(26*4)]
                fadd DWORD PTR [esi+(25*4)]
                fsub DWORD PTR [esi+(17*4)]
                fsub DWORD PTR [esi+(22*4)]
                fstp DWORD PTR [edi+(3*16*4)]

                fld DWORD PTR [esi+(17*4)]
                fsub DWORD PTR [esi+(30*4)]
                fstp DWORD PTR [edi+(1*16*4)]

                ret

                IFDEF AMD
AMD_poly_filter   ENDP
                ELSE
x86_poly_filter   ENDP
                ENDIF

;#############################################################################
;##                                                                         ##
;## Apply inverse window and write sample data                              ##
;##                                                                         ##
;#############################################################################

MAC_INIT_PAIR   MACRO f1,f2,f3,f4

                IFDEF AMD

                  movd mm1,DWORD PTR f1
                  movd mm2,DWORD PTR f2
                  movd mm3,DWORD PTR f3
                  movd mm4,DWORD PTR f4
                  pfmul mm1,mm2
                  pfmul mm3,mm4

                ELSE

                 fld DWORD PTR f3
                 fmul DWORD PTR f4

                 fld DWORD PTR f1
                 fmul DWORD PTR f2

                ENDIF

                ENDM

MAC_PAIR        MACRO f1,f2,f3,f4

                IFDEF AMD

                  movd mm0,DWORD PTR f1
                  movd mm5,DWORD PTR f2
                  movd mm6,DWORD PTR f3
                  movd mm7,DWORD PTR f4
                  pfmul mm0,mm5
                  pfmul mm6,mm7
                  pfadd mm1,mm0
                  pfadd mm3,mm6

                ELSE

                  fld DWORD PTR f1
                  fld DWORD PTR f3  
                                    
                  fmul DWORD PTR f4
                  fxch st(1)        
                  fmul DWORD PTR f2 

                  faddp st(2),st
                  faddp st(2),st

                ENDIF

                ENDM

MAC_END_SUM     MACRO
                LOCAL __t1,__t2,__c1,__c2,__done

                IFDEF AMD

                  pfadd mm1,mm3
                  pf2id mm1,mm1
                  movd temp,mm1

                ELSE

                  faddp st(1),st
                  fistp temp

                ENDIF

                mov ecx,temp
                cmp ecx,32767
                jg __c1
__t1:           cmp ecx,-32768
                jl __c2
__t2:           mov WORD PTR [edi],cx
                add edi,[dwSampleStep]
                jmp __done
__c1:           mov ecx,32767
                jmp __t1
__c2:           mov ecx,-32768
                jmp __t2
__done:

                ENDM

MAC_END_DIF     MACRO
                LOCAL __t1,__t2,__c1,__c2,__done

                IFDEF AMD

                  pfsub mm1,mm3
                  pf2id mm1,mm1
                  movd temp,mm1

                ELSE

                  fsubrp st(1),st
                  fistp temp

                ENDIF

                mov ecx,temp
                cmp ecx,32767
                jg __c1
                cmp ecx,-32768
                jl __c2
                mov WORD PTR [edi],cx
                add edi,[dwSampleStep]
                jmp __done

__c1:           mov WORD PTR [edi],32767
                add edi,[dwSampleStep]
                jmp __done

__c2:           mov WORD PTR [edi],-32768
                add edi,[dwSampleStep]
;                jmp __done
__done:

                ENDM

                IFDEF AMD
AMD_dewindow_and_write PROC \
                  USES ebx esi edi \
                  lpU:PTR,\
                  lpDewindow:PTR,\
                  dwStart:DWORD,\
                  lpSamples:PTR,\
                  dwSampleStep:DWORD,\
                  dwDiv:DWORD
                ELSE
x86_dewindow_and_write PROC \
                  USES ebx esi edi \
                  lpU:PTR,\
                  lpDewindow:PTR,\
                  dwStart:DWORD,\
                  lpSamples:PTR,\
                  dwSampleStep:DWORD,\
                  dwDiv:DWORD
                ENDIF

                LOCAL index
                LOCAL loops
                LOCAL temp

                mov esi,[lpU]
                mov edi,[lpSamples]

                mov eax,[dwStart]
                shl eax,2
                mov index,eax

                mov ebx,[lpDewindow]
                add ebx,16*4
                sub ebx,eax

                ;
                ;ESI = u        (8.24)
                ;EBX = dewindow (24.8)
                ;EDI = output samples (S16)
                ;

                _FEMMS

                ;
                ;First 16 samples
                ;

                lea eax,[esi][16*16*4]
                mov loops,eax

__for_loops:    MAC_INIT_PAIR [esi],[ebx],[esi+4],[ebx+4]

                CNT = 8
                REPT 7

                  MAC_PAIR [esi+CNT],[ebx+CNT],[esi+CNT+4],[ebx+CNT+4]

                CNT = CNT + 8
                ENDM

                add ebx,32*4
                add esi,16*4

                MAC_END_SUM

                cmp esi,loops
                jne __for_loops

                test [dwDiv],1
                jz __even_segment

                ;
                ;Odd segment, 17th sample
                ;

                MAC_INIT_PAIR [esi],[ebx],[esi+8],[ebx+8]

                CNT = 16
                REPT 3

                  MAC_PAIR [esi+CNT],[ebx+CNT],[esi+CNT+8],[ebx+CNT+8]

                CNT = CNT + 16
                ENDM

                MAC_END_SUM

                mov eax,index
                lea ebx,[ebx][eax*2][-48*4]

                ;
                ;Odd segment, last 15 samples
                ;

                lea eax,[esi][-15*16*4]
                mov loops,eax

__odd_loops:    sub esi,16*4

                ASCEND = 0*4
                DESCEND = 15*4

                MAC_INIT_PAIR [esi+ASCEND+4],[ebx+DESCEND-4],[esi+ASCEND],[ebx+DESCEND]

                REPT 7

                  DESCEND = DESCEND - 8
                  ASCEND = ASCEND + 8

                  MAC_PAIR [esi+ASCEND+4],[ebx+DESCEND-4],[esi+ASCEND],[ebx+DESCEND]

                ENDM

                sub ebx,32*4

                MAC_END_DIF

                cmp esi,loops
                jne __odd_loops

                jmp __exit

                ;
                ;Even segment, 17th sample
                ;

__even_segment:
                MAC_INIT_PAIR [esi+4],[ebx+4],[esi+12],[ebx+12]

                CNT = 16
                REPT 3

                  MAC_PAIR [esi+CNT+4],[ebx+CNT+4],[esi+CNT+12],[ebx+CNT+12]

                CNT = CNT + 16
                ENDM

                MAC_END_SUM

                mov eax,index
                lea ebx,[ebx][eax*2][-48*4]

                ;
                ;Even segment, last 15 samples
                ;

                lea eax,[esi][-15*16*4]
                mov loops,eax

__even_loops:   sub esi,16*4

                ASCEND = 0*4
                DESCEND = 15*4

                MAC_INIT_PAIR [esi+ASCEND],[ebx+DESCEND],[esi+ASCEND+4],[ebx+DESCEND-4]

                REPT 7

                  DESCEND = DESCEND - 8
                  ASCEND = ASCEND + 8

                  MAC_PAIR [esi+ASCEND],[ebx+DESCEND],[esi+ASCEND+4],[ebx+DESCEND-4]

                ENDM

                sub ebx,32*4

                MAC_END_DIF

                cmp esi,loops
                jne __even_loops

__exit:         _FEMMS

                ret

                IFDEF AMD
AMD_dewindow_and_write ENDP
                ELSE
x86_dewindow_and_write ENDP
                ENDIF

db 13,10,13,10
db 'Miles Sound System',13,10
db 'Copyright (C) 1991-99 RAD Game Tools, Inc.',13,10,13,10

                ;
                ;End of locked code
                ;

IFDEF AMD
amd_code_end LABEL BYTE
public amd_code_end
ELSE
x86_code_end LABEL BYTE
public x86_code_end
ENDIF

;#############################################################################
;##                                                                         ##
;## Lock all code and data in AILSSA module                                 ##
;##                                                                         ##
;#############################################################################

IFDEF AMD

EXTRN x86_data_start:BYTE,x86_data_end:BYTE,x86_code_start:BYTE,x86_code_end:BYTE

;        IFDEF DPMI

AIL_vmm_lock_range PROTO C,P1:NEAR PTR,P2:NEAR PTR

MATHA_VMM_lock  PROC C \
        USES ebx esi edi ds es

                invoke AIL_vmm_lock_range,OFFSET amd_data_start,OFFSET amd_data_end
                invoke AIL_vmm_lock_range,OFFSET amd_code_start,OFFSET amd_code_end

                invoke AIL_vmm_lock_range,OFFSET x86_data_start,OFFSET x86_data_end
                invoke AIL_vmm_lock_range,OFFSET x86_code_start,OFFSET x86_code_end

                ret

MATHA_VMM_lock  ENDP

;                ENDIF

ENDIF

                END


