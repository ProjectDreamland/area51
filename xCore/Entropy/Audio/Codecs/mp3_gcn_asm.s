//############################################################################
//##                                                                        ##
//##  MATH_A.C                                                              ##
//##                                                                        ##
//##  Miles Sound System                                                    ##
//##                                                                        ##
//##  IMDCT and polyphase-filter support routines converted from PC Asm     ##
//##                                                                        ##
//##  Version 1.00 of 2-Mar-99: Initial, derived from algorithms by         ##
//##                            Byeong Gi Lee, Jeff Tsay, Francois          ##
//##                            Raymond-Boyer, K. Konstantinides, Mikko     ##
//##                            Tommila, et al.                             ##
//##  Version 1.01 of 2-Jul-01: Converted to PPC assembly                   ##
//##                                                                        ##
//##  Author: Jeff Roberts                                                  ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

//#include "mss.h"
//#include "mp3dec.h"
#include "ppc-asm.h"
    .sdata
sin30           :    .float   0.500000000; // sin(pi/6)
cos30           :    .float   0.866025403; // cos(pi/6)

SQRT3           : .float  1.732050808;
SQRT2D2         : .float  0.707106781;
SQRT6PSQRT2D2   : .float  1.931851653;
SQRT6MSQRT2D2   : .float  0.517638090;  

dct3t4          : .float  0.504314480;
dct3t5          : .float  0.541196100;
dct3t6          : .float  0.630236207;
dct3t7          : .float  0.821339815;
dct3t8          : .float  1.306562965;
dct3t9          : .float  3.830648788;
dct3t10         : .float -0.793353340;
dct3t11         : .float -0.608761429;
dct3t12         : .float -0.923879532;
dct3t13         : .float -0.382683432;
dct3t14         : .float -0.991444861;
dct3t15         : .float -0.130526192;
dct3t17         : .float  0.382683432;
dct3t18         : .float  0.608761429;
dct3t19         : .float -0.793353340;
dct3t20         : .float -0.923879532;
dct3t21         : .float -0.991444861;
dct3t22         : .float  0.130526192;

dct9t0          : .float  1.8793852415718;
dct9t1          : .float  1.532088886238;
dct9t2          : .float  0.34729635533386;
dct9t10         : .float  1.9696155060244;
dct9t11         : .float  1.2855752193731;
dct9t12         : .float  0.68404028665134;

dct36t0         : .float  0.501909918;
dct36t1         : .float -0.250238171;
dct36t2         : .float -5.731396405;

dct36t4         : .float -0.25215724;
dct36t5         : .float -1.915324394;
dct36t6         : .float  0.551688959;
dct36t7         : .float -0.256069878;
dct36t8         : .float -1.155056579;
dct36t9         : .float  0.610387294;
dct36t10        : .float -0.262132281;
dct36t11        : .float -0.831377381;
dct36t12        : .float  0.871723397;
dct36t13        : .float -0.281845486;
dct36t14        : .float -0.541420142;
dct36t15        : .float  1.183100792;
dct36t16        : .float -0.296422261;
dct36t17        : .float -0.465289749;
dct36t19        : .float -0.315118103;
dct36t20        : .float -0.410669907;
dct36t21        : .float  5.736856623;
dct36t22        : .float -0.339085426;
dct36t23        : .float -0.370046808;
dct36t25        : .float -0.541196100;
dct36t26        : .float -1.306562965;

#define fp0 f0
#define fp1 f1
#define fp2 f2
#define fp3 f3
#define fp4 f4
#define fp5 f5
#define fp6 f6
#define fp7 f7
#define fp8 f8
#define fp9 f9
#define fp10 f10
#define fp11 f11
#define fp12 f12
#define fp13 f13
#define fp14 f14
#define fp15 f15
#define fp16 f16
#define fp17 f17
#define fp18 f18
#define fp19 f19
#define fp20 f20
#define fp21 f21
#define fp22 f22
#define fp23 f23
#define fp24 f24
#define fp25 f25
#define fp26 f26
#define fp27 f27
#define fp28 f28
#define fp29 f29
#define fp30 f30
#define fp31 f31

#define RTOC 
// This has to be here otherwise the linker fucks up the symbol table somehow.
    .section    .line
    .4byte    0
    .4byte    0
    .previous

    .file __FILE__

        .text
//#############################################################################
//##                                                                         ##
//## 1x36 IMDCT for window types 0, 1, and 3 (long windows)                  ##
//##                                                                         ##
//#############################################################################

#define FUNCTION(x,framesize)       \
        .global x;                  \
        x: stwu sp,-(framesize+8)(sp)   

#define END_FUNCTION(x)             \
        lwz sp,0(sp);               \
        blr                         

#define r0  0

FUNCTION(PPC_IMDCT_1x36,0)

#define input   r3
#define sb      r4
#define result  r5
#define save    r6
#define window  r7

#if 0
void AILCALL PPC_IMDCT_1x36    (register F32 FAR *input,   // r3
                                register S32      sb,      // r4
                                register F32 FAR *result,  // r5
                                register F32 FAR *save,    // r6
                                register F32 FAR *window)  // r7
{
#endif
  #define srcp    r3
  #define lastp   r6
  #define windp   r7
  #define destp   r5

    lfs fp18, (0*4)(srcp)
    lfs fp1, (1*4)(srcp)
    lfs fp2, (2*4)(srcp)
    lfs fp3, (3*4)(srcp)
    lfs fp4, (4*4)(srcp)
    lfs fp5, (5*4)(srcp)
    lfs fp6, (6*4)(srcp)
    lfs fp7, (7*4)(srcp)
    lfs fp8, (8*4)(srcp)
    lfs fp9, (9*4)(srcp)
    lfs fp10, (10*4)(srcp)
    lfs fp11, (11*4)(srcp)
    lfs fp12, (12*4)(srcp)
    lfs fp13, (13*4)(srcp)
    lfs fp14, (14*4)(srcp)
    lfs fp15, (15*4)(srcp)
    lfs fp16, (16*4)(srcp)
    lfs fp17, (17*4)(srcp)
    
    fadds fp17, fp17, fp16
    fadds fp30, fp16, fp15
    fadds fp15, fp15, fp14
    fadds fp14, fp14, fp13
    fadds fp13, fp13, fp12
    fadds fp28, fp12, fp11
    fadds fp11, fp11, fp10
    fadds fp10, fp10, fp9
    fadds fp9, fp9, fp8
    fadds fp16, fp8, fp7
    fadds fp7, fp7, fp6
    fadds fp6, fp6, fp5
    fadds fp5, fp5, fp4
    fadds fp24, fp4, fp3
    fadds fp3, fp3, fp2
    fadds fp26, fp2, fp1
    fadds fp19, fp1, fp18
    
    fadds fp31, fp17, fp15
    fadds fp15, fp15, fp13
    fadds fp29, fp13, fp11
    fadds fp11, fp11, fp9
    fadds fp17, fp9, fp7
    fadds fp7, fp7, fp5
    fadds fp25, fp5, fp3
    fadds fp27, fp3, fp19
    
    stfs fp6, (0*4)(srcp)
    stfs fp7, (1*4)(srcp)
    stfs fp10, (2*4)(srcp)
    stfs fp11, (3*4)(srcp)
    stfs fp14, (4*4)(srcp)
    stfs fp15, (5*4)(srcp)

    //
    // do the five pointer
    //

    lfs fp10, dct9t0@sda21(0)
    lfs fp11, dct9t1@sda21(0)
    lfs fp12, dct9t2@sda21(0)
    lfs fp15, SQRT2D2@sda21(0)

    fadds fp22, fp18, fp18
    fadds fp23, fp19, fp19
    fadds fp20, fp28, fp22
    fadds fp21, fp29, fp23

    fmuls  fp0, fp16, fp11
    fmuls  fp1, fp17, fp11
    fmadds fp0, fp24, fp10, fp0
    fmadds fp1, fp25, fp10, fp1
    fmadds fp13, fp30, fp12, fp20
    fmadds fp14, fp31, fp12, fp21
    fadds  fp0, fp0, fp13
    fadds  fp1, fp1, fp14

    fadds  fp2, fp22, fp24
    fadds  fp3, fp23, fp25
    fadds  fp13, fp28, fp30
    fadds  fp14, fp29, fp31
    fsubs  fp2, fp2, fp16
    fsubs  fp3, fp3, fp17
    fsubs  fp2, fp2, fp13
    fsubs  fp3, fp3, fp14
    fsubs  fp2, fp2, fp28
    fsubs  fp3, fp3, fp29

    fmuls  fp4, fp16, fp10
    fmuls  fp5, fp17, fp10
    fmadds fp4, fp24, fp12, fp4
    fmadds fp5, fp25, fp12, fp5
    fmadds fp13, fp30, fp11, fp20
    fmadds fp14, fp31, fp11, fp21
    fsubs  fp4, fp13, fp4
    fsubs  fp5, fp14, fp5

    fmuls  fp6, fp30, fp10
    fmuls  fp7, fp31, fp10
    fmadds fp6, fp24, fp11, fp6
    fmadds fp7, fp25, fp11, fp7
    fmadds fp13, fp16, fp12, fp20
    fmadds fp14, fp17, fp12, fp21
    fsubs  fp6, fp13, fp6
    fsubs  fp7, fp14, fp7

    fsubs  fp9, fp19, fp25
    fsubs  fp8, fp18, fp24
    fsubs  fp14, fp17, fp29
    fsubs  fp13, fp16, fp28
    fadds  fp9, fp9, fp31
    fadds  fp8, fp8, fp30
    fadds  fp9, fp9, fp14
    fadds  fp8, fp8, fp13
    fmuls  fp9, fp9, fp15

    //
    // do four points
    //

    lfs fp18, SQRT3@sda21(0)
    lfs fp24, (0*4)(srcp)
    lfs fp25, (1*4)(srcp)
    lfs fp28, (2*4)(srcp)
    lfs fp29, (3*4)(srcp)
    lfs fp30, (4*4)(srcp)
    lfs fp31, (5*4)(srcp)
    fmuls fp24, fp24, fp18
    fmuls fp25, fp25, fp18

    lfs fp21, dct9t10@sda21(0)
    lfs fp22, dct9t11@sda21(0)
    lfs fp23, dct9t12@sda21(0)

    fmuls fp19, fp28, fp22
    fmuls fp20, fp29, fp22
    fmadds fp10, fp26, fp21, fp24
    fmadds fp11, fp27, fp21, fp25
    fmadds fp19, fp30, fp23, fp19
    fmadds fp20, fp31, fp23, fp20
    fadds fp10, fp10, fp19
    fadds fp11, fp11, fp20

    fsubs fp12, fp26, fp28
    fsubs fp13, fp27, fp29
    fsubs fp12, fp12, fp30
    fsubs fp13, fp13, fp31
    fmuls fp12, fp12, fp18
    fmuls fp13, fp13, fp18

    fmuls fp19, fp28, fp23
    fmuls fp20, fp29, fp23
    fmsubs fp14, fp26, fp22, fp24
    fmsubs fp15, fp27, fp22, fp25
    fmsubs fp19, fp30, fp21, fp19
    fmsubs fp20, fp31, fp21, fp20
    fadds fp14, fp14, fp19
    fadds fp15, fp15, fp20

    fmuls fp19, fp30, fp22
    fmuls fp20, fp31, fp22
    fmsubs fp16, fp26, fp23, fp24
    fmsubs fp17, fp27, fp23, fp25
    fmsubs fp19, fp28, fp21, fp19
    fmsubs fp20, fp29, fp21, fp20
    fadds fp16, fp16, fp19
    fadds fp17, fp17, fp20

    // twiddle into same registers but all mixed up
    // 0=0, 1=17, 2=1, 3=16, 4=2, 5=15, 6=3, 7=14, 8=4, 9=13
    // 10=8, 11=9, 12=7, 13=10, 14=6, 15=11, 16=5, 17=12
  
    #define twiddle( f51, f52, f41, f42, scale1, scale2, lscale1, lscale2, hscale1, hscale2 ) ;\
      lfs   fp26, scale1@sda21(0)          ;\
      lfs   fp27, scale2@sda21(0)          ;\
      lfs   fp28, lscale1@sda21(0)         ;\
      lfs   fp29, lscale2@sda21(0)         ;\
      lfs   fp30, hscale1@sda21(0)         ;\
      lfs   fp31, hscale2@sda21(0)         ;\
      fadds fp18, fp##f51, fp##f41      ;\
      fadds fp19, fp##f52, fp##f42      ;\
      fsubs fp20, fp##f51, fp##f41      ;\
      fsubs fp21, fp##f52, fp##f42      ;\
      fmuls fp19, fp19, fp26            ;\
      fmuls fp21, fp21, fp27            ;\
      fadds fp##f51, fp18, fp19         ;\
      fsubs fp##f52, fp18, fp19         ;\
      fadds fp##f41, fp20, fp21         ;\
      fsubs fp##f42, fp20, fp21         ;\
      fmuls fp##f51, fp##f51, fp28      ;\
      fmuls fp##f52, fp##f52, fp30      ;\
      fmuls fp##f41, fp##f41, fp29      ;\
      fmuls fp##f42, fp##f42, fp31      ;

    twiddle( 0, 1, 10, 11, dct36t0, dct36t21, dct36t1, dct36t22, dct36t2, dct36t23 )
    twiddle( 2, 3, 12, 13, SQRT6MSQRT2D2, SQRT6PSQRT2D2, dct36t4, dct36t19, dct36t5, dct36t20 )
    twiddle( 4, 5, 14, 15, dct36t6, dct36t15, dct36t7, dct36t16, dct36t8, dct36t17 )
    twiddle( 6, 7, 16, 17, dct36t9, dct36t12, dct36t10, dct36t13, dct36t11, dct36t14 )

    lfs fp20, dct36t25@sda21(0)
    lfs fp21, dct36t26@sda21(0)
    fadds fp18, fp8, fp9
    fsubs fp19, fp8, fp9
    fmuls fp8, fp18, fp20
    fmuls fp9, fp19, fp21

    andi. r0, sb, 1
    beq   __even_window

    // 0 to 3
    lfs fp18, (0*4)(windp)
    lfs fp19, (1*4)(windp)
    lfs fp20, (2*4)(windp)
    lfs fp21, (3*4)(windp)
    fneg fp18, f18
    fneg fp20, f20
    lfs fp22, (0*4)(lastp)
    lfs fp23, (1*4)(lastp)
    lfs fp24, (2*4)(lastp)
    lfs fp25, (3*4)(lastp)
    fmadds fp18, fp18, fp11, fp22
    fmsubs fp19, fp19, fp13, fp23
    fmadds fp20, fp20, fp15, fp24
    fmsubs fp21, fp21, fp17, fp25
    stfs fp18, (0*4)(destp)
    stfs fp19, (1*4)(destp)
    stfs fp20, (2*4)(destp)
    stfs fp21, (3*4)(destp)

    // 4 to 8
    lfs fp18, (4*4)(windp)
    lfs fp19, (5*4)(windp)
    lfs fp20, (6*4)(windp)
    lfs fp21, (7*4)(windp)
    lfs fp26, (8*4)(windp)
    fneg fp18, f18
    fneg fp20, f20
    fneg fp26, f26
    lfs fp22, (4*4)(lastp)
    lfs fp23, (5*4)(lastp)
    lfs fp24, (6*4)(lastp)
    lfs fp25, (7*4)(lastp)
    lfs fp27, (8*4)(lastp)
    fmadds fp18, fp18, fp9, fp22
    fmsubs fp19, fp19, fp7, fp23
    fmadds fp20, fp20, fp5, fp24
    fmsubs fp21, fp21, fp3, fp25
    fmadds fp26, fp26, fp1, fp27
    stfs fp18, (4*4)(destp)
    stfs fp19, (5*4)(destp)
    stfs fp20, (6*4)(destp)
    stfs fp21, (7*4)(destp)
    stfs fp26, (8*4)(destp)

    // 9 to 12
    lfs fp18, (9*4)(windp)
    lfs fp19, (10*4)(windp)
    lfs fp20, (11*4)(windp)
    lfs fp21, (12*4)(windp)
    lfs fp22, (9*4)(lastp)
    lfs fp23, (10*4)(lastp)
    lfs fp24, (11*4)(lastp)
    lfs fp25, (12*4)(lastp)
    fmadds fp18, fp18, fp1, fp22
    fmadds fp19, fp19, fp3, fp23
    fmadds fp20, fp20, fp5, fp24
    fmadds fp21, fp21, fp7, fp25
    fneg fp18, f18
    fneg fp20, f20
    stfs fp18, (9*4)(destp)
    stfs fp19, (10*4)(destp)
    stfs fp20, (11*4)(destp)
    stfs fp21, (12*4)(destp)

    // 13 to 17
    lfs fp18, (13*4)(windp)
    lfs fp19, (14*4)(windp)
    lfs fp20, (15*4)(windp)
    lfs fp21, (16*4)(windp)
    lfs fp26, (17*4)(windp)
    lfs fp22, (13*4)(lastp)
    lfs fp23, (14*4)(lastp)
    lfs fp24, (15*4)(lastp)
    lfs fp25, (16*4)(lastp)
    lfs fp27, (17*4)(lastp)
    fmadds fp18, fp18, fp9, fp22
    fmadds fp19, fp19, fp17, fp23
    fmadds fp20, fp20, fp15, fp24
    fmadds fp21, fp21, fp13, fp25
    fmadds fp26, fp26, fp11, fp27
    fneg fp18, f18
    fneg fp20, f20
    fneg fp26, f26
    stfs fp18, (13*4)(destp)
    stfs fp19, (14*4)(destp)
    stfs fp20, (15*4)(destp)
    stfs fp21, (16*4)(destp)
    stfs fp26, (17*4)(destp)

    b __save

__even_window:

    // 0 to 3
    lfs fp18, (0*4)(windp)
    lfs fp19, (1*4)(windp)
    lfs fp20, (2*4)(windp)
    lfs fp21, (3*4)(windp)
    fneg fp18, f18
    fneg fp19, f19
    fneg fp20, f20
    fneg fp21, f21
    lfs fp22, (0*4)(lastp)
    lfs fp23, (1*4)(lastp)
    lfs fp24, (2*4)(lastp)
    lfs fp25, (3*4)(lastp)
    fmadds fp18, fp18, fp11, fp22
    fmadds fp19, fp19, fp13, fp23
    fmadds fp20, fp20, fp15, fp24
    fmadds fp21, fp21, fp17, fp25
    stfs fp18, (0*4)(destp)
    stfs fp19, (1*4)(destp)
    stfs fp20, (2*4)(destp)
    stfs fp21, (3*4)(destp)

    // 4 to 8
    lfs fp18, (4*4)(windp)
    lfs fp19, (5*4)(windp)
    lfs fp20, (6*4)(windp)
    lfs fp21, (7*4)(windp)
    lfs fp26, (8*4)(windp)
    fneg fp18, f18
    fneg fp19, f19
    fneg fp20, f20
    fneg fp21, f21
    fneg fp26, f26
    lfs fp22, (4*4)(lastp)
    lfs fp23, (5*4)(lastp)
    lfs fp24, (6*4)(lastp)
    lfs fp25, (7*4)(lastp)
    lfs fp27, (8*4)(lastp)
    fmadds fp18, fp18, fp9, fp22
    fmadds fp19, fp19, fp7, fp23
    fmadds fp20, fp20, fp5, fp24
    fmadds fp21, fp21, fp3, fp25
    fmadds fp26, fp26, fp1, fp27
    stfs fp18, (4*4)(destp)
    stfs fp19, (5*4)(destp)
    stfs fp20, (6*4)(destp)
    stfs fp21, (7*4)(destp)
    stfs fp26, (8*4)(destp)

    // 9 to 12
    lfs fp18, (9*4)(windp)
    lfs fp19, (10*4)(windp)
    lfs fp20, (11*4)(windp)
    lfs fp21, (12*4)(windp)
    lfs fp22, (9*4)(lastp)
    lfs fp23, (10*4)(lastp)
    lfs fp24, (11*4)(lastp)
    lfs fp25, (12*4)(lastp)
    fmadds fp18, fp18, fp1, fp22
    fmadds fp19, fp19, fp3, fp23
    fmadds fp20, fp20, fp5, fp24
    fmadds fp21, fp21, fp7, fp25
    stfs fp18, (9*4)(destp)
    stfs fp19, (10*4)(destp)
    stfs fp20, (11*4)(destp)
    stfs fp21, (12*4)(destp)

    // 13 to 17
    lfs fp18, (13*4)(windp)
    lfs fp19, (14*4)(windp)
    lfs fp20, (15*4)(windp)
    lfs fp21, (16*4)(windp)
    lfs fp26, (17*4)(windp)
    lfs fp22, (13*4)(lastp)
    lfs fp23, (14*4)(lastp)
    lfs fp24, (15*4)(lastp)
    lfs fp25, (16*4)(lastp)
    lfs fp27, (17*4)(lastp)
    fmadds fp18, fp18, fp9, fp22
    fmadds fp19, fp19, fp17, fp23
    fmadds fp20, fp20, fp15, fp24
    fmadds fp21, fp21, fp13, fp25
    fmadds fp26, fp26, fp11, fp27
    stfs fp18, (13*4)(destp)
    stfs fp19, (14*4)(destp)
    stfs fp20, (15*4)(destp)
    stfs fp21, (16*4)(destp)
    stfs fp26, (17*4)(destp)

__save:
    // save into previous
    lfs fp18, (18*4)(windp)
    lfs fp19, (19*4)(windp)
    lfs fp20, (20*4)(windp)
    lfs fp21, (21*4)(windp)
    lfs fp22, (22*4)(windp)
    lfs fp23, (23*4)(windp)
    lfs fp24, (24*4)(windp)
    lfs fp25, (25*4)(windp)
    lfs fp26, (26*4)(windp)
    fmuls fp18, fp18, fp10
    fmuls fp19, fp19, fp12
    fmuls fp20, fp20, fp14
    fmuls fp21, fp21, fp16
    fmuls fp22, fp22, fp8
    fmuls fp23, fp23, fp6
    fmuls fp24, fp24, fp4
    fmuls fp25, fp25, fp2
    fmuls fp26, fp26, fp0
    stfs fp18, (0*4)(lastp)
    stfs fp19, (1*4)(lastp)
    stfs fp20, (2*4)(lastp)
    stfs fp21, (3*4)(lastp)
    stfs fp22, (4*4)(lastp)
    stfs fp23, (5*4)(lastp)
    stfs fp24, (6*4)(lastp)
    stfs fp25, (7*4)(lastp)
    stfs fp26, (8*4)(lastp)

    lfs fp18, (27*4)(windp)
    lfs fp19, (28*4)(windp)
    lfs fp20, (29*4)(windp)
    lfs fp21, (30*4)(windp)
    lfs fp22, (31*4)(windp)
    lfs fp23, (32*4)(windp)
    lfs fp24, (33*4)(windp)
    lfs fp25, (34*4)(windp)
    lfs fp26, (35*4)(windp)
    fmuls fp18, fp18, fp0
    fmuls fp19, fp19, fp2
    fmuls fp20, fp20, fp4
    fmuls fp21, fp21, fp6
    fmuls fp22, fp22, fp8
    fmuls fp23, fp23, fp16
    fmuls fp24, fp24, fp14
    fmuls fp25, fp25, fp12
    fmuls fp26, fp26, fp10
    stfs fp18, (9*4)(lastp)
    stfs fp19, (10*4)(lastp)
    stfs fp20, (11*4)(lastp)
    stfs fp21, (12*4)(lastp)
    stfs fp22, (13*4)(lastp)
    stfs fp23, (14*4)(lastp)
    stfs fp24, (15*4)(lastp)
    stfs fp25, (16*4)(lastp)
    stfs fp26, (17*4)(lastp)

    END_FUNCTION(PPC_IMDCT_1x36)

#undef srcp
#undef lastp
#undef windp
#undef destp

#undef input
#undef sb   
#undef result
#undef save  
#undef window

//#############################################################################
//##                                                                         ##
//## 3x12 IMDCT for window type 2 (short windows)                            ##
//##                                                                         ##
//#############################################################################

                //
                //t1 = in2 * cos(pi/6)
                //t2 = in1 * sin(pi/6)
                //t3 = in3 + t2;
                //out1 = in3 - in1;
                //out2 = t3 + t1;
                //out3 = t3 - t1;
                //

#define IDCT_3( in1, in2, in3, reg1, reg2, reg3, scale1, scale2 ) ;\
  fmadds reg3, in1, scale1, in3                  ;\
  fmuls in2, in2, scale2                         ;\
  fsubs reg1, in3, in1                           ;\
  fadds reg2, reg3, in2                          ;\
  fsubs reg3, reg3, in2                          ;

#define blend_four_alt( save_off, dest_off ) ;\
    lfs fp0, ((save_off+0)*4)(save)          ;\
    lfs fp1, ((save_off+1)*4)(save)          ;\
    lfs fp2, ((save_off+2)*4)(save)          ;\
    lfs fp3, ((save_off+3)*4)(save)          ;\
    lfs fp4, ((dest_off+0)*4)(destp)         ;\
    lfs fp5, ((dest_off+1)*4)(destp)         ;\
    lfs fp6, ((dest_off+2)*4)(destp)         ;\
    lfs fp7, ((dest_off+3)*4)(destp)         ;\
    fneg fp5, fp5                            ;\
    fneg fp7, fp7                            ;\
    fadds fp4, fp4, fp0                      ;\
    fsubs fp5, fp5, fp1                      ;\
    fadds fp6, fp6, fp2                      ;\
    fsubs fp7, fp7, fp3                      ;\
    stfs fp4, ((save_off+0)*4)(result)       ;\
    stfs fp5, ((save_off+1)*4)(result)       ;\
    stfs fp6, ((save_off+2)*4)(result)       ;\
    stfs fp7, ((save_off+3)*4)(result)       ;

#define blend_four( save_off, dest_off )     ;\
    lfs fp0, ((save_off+0)*4)(save)          ;\
    lfs fp1, ((save_off+1)*4)(save)          ;\
    lfs fp2, ((save_off+2)*4)(save)          ;\
    lfs fp3, ((save_off+3)*4)(save)          ;\
    lfs fp4, ((dest_off+0)*4)(destp)         ;\
    lfs fp5, ((dest_off+1)*4)(destp)         ;\
    lfs fp6, ((dest_off+2)*4)(destp)         ;\
    lfs fp7, ((dest_off+3)*4)(destp)         ;\
    fadds fp4, fp4, fp0                      ;\
    fadds fp5, fp5, fp1                      ;\
    fadds fp6, fp6, fp2                      ;\
    fadds fp7, fp7, fp3                      ;\
    stfs fp4, ((save_off+0)*4)(result)       ;\
    stfs fp5, ((save_off+1)*4)(result)       ;\
    stfs fp6, ((save_off+2)*4)(result)       ;\
    stfs fp7, ((save_off+3)*4)(result)       ;

#define CASCADE_ADD_6( a, f0, f1, f2, f3, f4, f5 )     ;\
  lfs f0, (((a)+0)*4)(srcp)   ;\
  lfs f1, (((a)+1)*4)(srcp)   ;\
  lfs f2, (((a)+2)*4)(srcp)   ;\
  lfs f3, (((a)+3)*4)(srcp)   ;\
  lfs f4, (((a)+4)*4)(srcp)   ;\
  lfs f5, (((a)+5)*4)(srcp)   ;\
  fadds f5,f5,f4              ;\
  fadds f4,f4,f3              ;\
  fadds f3,f3,f2              ;\
  fadds f2,f2,f1              ;\
  fadds f1,f1,f0              ;\
  fadds f5,f5,f3              ;\
  fadds f3,f3,f1              ;

FUNCTION(PPC_IMDCT_3x12,24*4)

#define in      r3
#define sb      r4
#define result  r5
#define save    r6

#define destp   r7
#define srcp    r3
  
#define output  8
#if 0
void AILCALL PPC_IMDCT_3x12    (register F32 FAR *in,        // r3
                                register S32      sb,        // r4
                                register F32 FAR *result,    // r5
                                register F32 FAR *save)      // r6
{
  F32 output[24];
#endif
    
    li      r0,0
 
    la      destp, output(sp)
    stw     r0,0(destp)
    stw     r0,4(destp)

    lfd  fp13, 0(destp)

// This instruction seems somewhat odd to me? srcp is not defined, should it be 'in'?
    addi r12, srcp, 18 * 4

    stfd fp13, 8(destp)
    stfd fp13, 16(destp)

    lfs fp30, sin30@sda21(0)                          
    lfs fp31, cos30@sda21(0)                         

    lfs fp26, SQRT6PSQRT2D2@sda21(0)
    lfs fp27, SQRT2D2@sda21(0)
    lfs fp28, SQRT6MSQRT2D2@sda21(0)
    
    lfs fp15, dct3t4@sda21(0)
    lfs fp22, dct3t5@sda21(0)
    lfs fp23, dct3t6@sda21(0)
    lfs fp24, dct3t7@sda21(0)
    lfs fp25, dct3t8@sda21(0)
    lfs fp29, dct3t9@sda21(0)

    lfs fp16, dct3t17@sda21(0)
    lfs fp17, dct3t18@sda21(0)
    lfs fp18, dct3t19@sda21(0)
    lfs fp19, dct3t20@sda21(0)
    lfs fp20, dct3t21@sda21(0)
    lfs fp21, dct3t22@sda21(0)

__for_DCT:            
    
    CASCADE_ADD_6( 0, fp6, fp7, fp8, fp9, fp10, fp11 )

    IDCT_3( fp10, fp8, fp6, fp1, fp0, fp2, fp30, fp31 )
    IDCT_3( fp11, fp9, fp7, fp4, fp5, fp3, fp30, fp31 )

    // scale
    fmuls fp3, fp3, fp26
    fmuls fp4, fp4, fp27
    fmuls fp5, fp5, fp28

    // butterfly
    fsubs fp6, fp0, fp5
    fadds fp0, fp0, fp5
    fsubs fp7, fp1, fp4
    fadds fp1, fp1, fp4
    fsubs fp8, fp2, fp3
    fadds fp2, fp2, fp3

    // swizzle
    fmuls fp0, fp0, fp15
    fmuls fp1, fp1, fp22
    fmuls fp2, fp2, fp23
    fmuls fp3, fp8, fp24
    fmuls fp4, fp7, fp25
    fmuls fp5, fp6, fp29

    // multiply and accumulate into the first six
    lfs fp6, (0*4)(destp)
    lfs fp7, (1*4)(destp)
    lfs fp8, (2*4)(destp)
    lfs fp9, (3*4)(destp)
    lfs fp10, (4*4)(destp)
    lfs fp11, (5*4)(destp)
    
    fmadds fp7, fp4, fp16, fp7
    fmadds fp8, fp5, fp17, fp8
    fmadds fp9, fp5, fp18, fp9
    fmadds fp10, fp4, fp19, fp10
    fmadds fp11, fp3, fp20, fp11
    fmadds fp6, fp3, fp21, fp6

    stfs fp6, (0*4)(destp)
    stfs fp7, (1*4)(destp)
    stfs fp8, (2*4)(destp)
    stfs fp9, (3*4)(destp)
    stfs fp10, (4*4)(destp)
    stfs fp11, (5*4)(destp)

    // do the direct copy into the second six
    lfs fp10, dct3t14@sda21(0)
    lfs fp8, dct3t12@sda21(0)
    lfs fp6, dct3t10@sda21(0)
    lfs fp7, dct3t11@sda21(0)
    lfs fp9, dct3t13@sda21(0)
    lfs fp11, dct3t15@sda21(0)
    fmuls fp10, fp2, fp10
    fmuls fp8, fp1, fp8
    fmuls fp6, fp0, fp6
    fmuls fp7, fp0, fp7
    fmuls fp9, fp1, fp9
    fmuls fp11, fp2, fp11
    stfs fp10, (6*4)(destp)
    stfs fp8, (7*4)(destp)
    stfs fp6, (8*4)(destp)
    stfs fp7, (9*4)(destp)
    stfs fp9, (10*4)(destp)
    stfs fp11, (11*4)(destp)

    // keep looping...
    addi srcp, srcp, 6 * 4
    addi destp, destp, 6 * 4
    cmpw srcp, r12
    bne  __for_DCT

    la destp, output(sp)

    andi. r0, sb, 1
    beq overlap

    lfs fp0, (0*4)(save)
    lfs fp1, (1*4)(save)
    lfs fp2, (2*4)(save)
    lfs fp3, (3*4)(save)
    lfs fp4, (4*4)(save)
    lfs fp5, (5*4)(save)
    fneg fp1, fp1
    fneg fp3, fp3
    fneg fp5, fp5
    stfs fp0, (0*4)(result)
    stfs fp1, (1*4)(result)
    stfs fp2, (2*4)(result)
    stfs fp3, (3*4)(result)
    stfs fp4, (4*4)(result)
    stfs fp5, (5*4)(result)

    blend_four_alt( 6, 0 )
    blend_four_alt( 10, 4 )
    blend_four_alt( 14, 8 )

    b overlap_done

   overlap:
    lfd fp0, (0*4)(save)
    lfd fp2, (2*4)(save)
    lfd fp4, (4*4)(save)
    stfd fp0, (0*4)(result)
    stfd fp2, (2*4)(result)
    stfd fp4, (4*4)(result)

    blend_four( 6, 0 )
    blend_four( 10, 4 )
    blend_four( 14, 8 )

   overlap_done:
    lfd fp0, ((12*4)+0)(destp)
    lfd fp1, ((12*4)+8)(destp)
    lfd fp2, ((12*4)+16)(destp)
    lfd fp3, ((12*4)+24)(destp)
    lfd fp4, ((12*4)+32)(destp)
    lfd fp5, ((12*4)+40)(destp)
    stfd fp0, 0(save)
    stfd fp1, 8(save)
    stfd fp2, 16(save)
    stfd fp3, 24(save)
    stfd fp4, 32(save)
    stfd fp5, 40(save)
    stfd fp13, 48(save)
    stfd fp13, 56(save)
    stfd fp13, 64(save)

    END_FUNCTION(PPC_IMDCT_3x12)

#undef destp
#undef srcp
#undef output
#undef in
#undef sb
#undef result
#undef save


//#############################################################################
//##                                                                         ##
//## IDCT reordering and butterflies for polyphase filter                    ##
//##                                                                         ##
//#############################################################################

                //
                //B = (A - C) * D; d1 = A + C;
                //F = (E - G) * H; d2 = E + G;
                //

#define REORD_PAIR( in, out, dest1, A, B, C, D, dest2, E, F, G, H ) ;\
  lfs fp16, ((A)*18*4)(in)                                        ;\
  lfs fp17, ((E)*18*4)(in)                                        ;\
  lfs fp18, ((C)*18*4)(in)                                        ;\
  lfs fp19, ((G)*18*4)(in)                                        ;\
  lfs fp20, ((D)*4)(barrayp)                                      ;\
  lfs fp21, ((H)*4)(barrayp)                                      ;\
                                                                  ;\
  fadds fp##dest1, fp16, fp18                                     ;\
  fadds fp##dest2, fp17, fp19                                     ;\
  fsubs fp16, fp16, fp18                                          ;\
  fsubs fp17, fp17, fp19                                          ;\
  fmuls fp16, fp16, fp20                                          ;\
  fmuls fp17, fp17, fp21                                          ;\
                                                                  ;\
  stfs fp16, (((B)-16)*4)(out)                                    ;\
  stfs fp17, (((F)-16)*4)(out)                                    ;


#define REORD_PAIR_TO_R( in, dest1, A, B, C, D, dest2, E, F, G, H ) ;\
  lfs fp##B, (((A)-16)*4)(in)                                       ;\
  lfs fp##F, (((E)-16)*4)(in)                                       ;\
  lfs fp16, (((C)-16)*4)(in)                                        ;\
  lfs fp17, (((G)-16)*4)(in)                                        ;\
  lfs fp18, ((D)*4)(barrayp)                                        ;\
  lfs fp19, ((H)*4)(barrayp)                                        ;\
  fadds fp##dest1, fp##B, fp16                                      ;\
  fadds fp##dest2, fp##F, fp17                                      ;\
  fsubs fp##B, fp##B, fp16                                          ;\
  fsubs fp##F, fp##F, fp17                                          ;\
  fmuls fp##B, fp##B, fp18                                          ;\
  fmuls fp##F, fp##F, fp19                                          ;\

#define REORD_PAIR_R( A, B, scale1, A2, B2, scale2 ) ;\
  fsubs fp24, fp##A, fp##B                           ;\
  fsubs fp25, fp##A2, fp##B2                         ;\
  fadds fp##A, fp##A, fp##B                          ;\
  fadds fp##A2, fp##A2, fp##B2                       ;\
  fmuls fp##B, fp24, scale1                          ;\
  fmuls fp##B2, fp25, scale2                         ;



FUNCTION(PPC_poly_filter,16*4)

#define lpsrc   r3
#define barray  r4
#define phase   r5
#define lpout1  r6
#define lpout2  r7
#define srcp    r3

#define temp    4           // offset within stackframe

#if 0
void AILCALL PPC_poly_filter(register const F32 FAR *lpsrc,   //r3
                             register const F32 FAR *barray,  //r4
                             register S32            phase,   //r5
                             register F32       FAR *lpout1,  //r6
                             register F32       FAR *lpout2)  //r7
{

  F32 temp[16];
#endif

 #define barrayp r4
 #define lpout1p r6
 #define lpout2p r7
 #define temp1   r10

    slwi phase, phase, 2
    la  temp1, temp(sp)
    add r8, srcp, phase

    REORD_PAIR( r8, temp1, 0,0,16,31,1,    1,1,17,30,3 )
    REORD_PAIR( r8, temp1, 3,2,19,29,5,    2,3,18,28,7 )
    REORD_PAIR( r8, temp1, 6,4,22,27,9,    7,5,23,26,11 )
    REORD_PAIR( r8, temp1, 5,6,21,25,13,   4,7,20,24,15 )
    REORD_PAIR( r8, temp1, 12,8,28,23,17,  13,9,29,22,19 )
    REORD_PAIR( r8, temp1, 15,10,31,21,21, 14,11,30,20,23 )
    REORD_PAIR( r8, temp1, 10,12,26,19,25, 11,13,27,18,27 )
    REORD_PAIR( r8, temp1, 9,14,25,17,29,  8,15,24,16,31 )

    lfs fp16, (2*4)(barrayp)
    lfs fp17, (6*4)(barrayp)
    lfs fp18, (14*4)(barrayp)
    lfs fp19, (10*4)(barrayp)
    lfs fp20, (30*4)(barrayp)
    lfs fp21, (26*4)(barrayp)
    lfs fp22, (18*4)(barrayp)
    lfs fp23, (22*4)(barrayp)
    REORD_PAIR_R( 0,8,fp16,    1,9,fp17 )
    REORD_PAIR_R( 2,10,fp18,  3,11,fp19 )
    REORD_PAIR_R( 4,12,fp20,  5,13,fp21 )
    REORD_PAIR_R( 6,14,fp22,  7,15,fp23 )

    lfs fp16, (4*4)(barrayp)
    lfs fp17, (12*4)(barrayp)
    lfs fp18, (28*4)(barrayp)
    lfs fp19, (20*4)(barrayp)
    REORD_PAIR_R( 0,4,fp16,      1,5,fp17 )
    REORD_PAIR_R( 2,6,fp18,      3,7,fp19 )
    REORD_PAIR_R( 8,12,fp16,    9,13,fp17 )
    REORD_PAIR_R( 10,14,fp18,  11,15,fp19 )

    lfs fp16, (8*4)(barrayp)
    lfs fp17, (24*4)(barrayp)
    REORD_PAIR_R( 0,2,fp16,     1,3,fp17 )
    REORD_PAIR_R( 4,6,fp16,     5,7,fp17 )
    REORD_PAIR_R( 8,10,fp16,   9,11,fp17 )
    REORD_PAIR_R( 12,14,fp16, 13,15,fp17 )

    fneg fp16, fp0
    lfs fp17, (16*4)(barrayp)
    fsubs fp16, fp16, fp1
    fsubs fp1, fp0, fp1
    fadds fp0, fp16, fp16
    fmuls fp1, fp1, fp17

    fsubs fp16, fp2, fp3
    fadds fp2, fp2, fp3
    fsubs fp18, fp4, fp5
    fmsubs fp3, fp16, fp17, fp2

    fadds fp4, fp4, fp5
    fsubs fp16, fp6, fp7
    fmadds fp5, fp18, fp17, fp4

    fadds fp6, fp6, fp7
    fmsubs fp7, fp16, fp17, fp5

    fsubs fp16, fp8, fp9
    fadds fp8, fp8, fp9
    fmuls fp9, fp16, fp17

    fsubs fp16, fp10, fp11
    fadds fp18, fp8, fp9
    fadds fp10, fp10, fp11
    fmadds fp11, fp16, fp17, fp18

    fsubs fp16, fp12, fp13
    fadds fp12, fp12, fp13
    fsubs fp19, fp14, fp15
    fsubs fp18, fp12, fp18
    fadds fp14, fp14, fp15
    fadds fp20, fp8, fp10
    fmadds fp13, fp16, fp17, fp18

    fmsubs fp15, fp19, fp17, fp11
    fsubs fp14, fp14, fp20

    fsubs fp19, fp11, fp13
    fadds fp18, fp8, fp9
    fsubs fp17, fp9, fp14
    fsubs fp16, fp5, fp6
    fsubs fp19, fp19, fp18
    fsubs fp18, fp13, fp10
    stfs fp1, (0*16*4)(lpout1p)
    stfs fp17, (2*16*4)(lpout1p)
    stfs fp16, (4*16*4)(lpout1p)
    stfs fp18, (6*16*4)(lpout1p)

    stfs fp3, (8*16*4)(lpout1p)
    stfs fp19, (10*16*4)(lpout1p)
    stfs fp7, (12*16*4)(lpout1p)
    stfs fp15, (14*16*4)(lpout1p)

    fneg fp16, fp1
    fneg fp17, fp14
    fsubs fp19, fp4, fp6
    fsubs fp20, fp8, fp12
    fsubs fp21, fp12, fp8
    stfs fp16, (0*16*4)(lpout2p)
    stfs fp17, (2*16*4)(lpout2p)

    fneg fp16, fp8
    fneg fp17, fp4
    fsubs fp21, fp21, fp10
    fneg fp18, fp2
    stfs fp19, (4*16*4)(lpout2p)
    stfs fp21, (6*16*4)(lpout2p)
    stfs fp18, (8*16*4)(lpout2p)
    stfs fp20, (10*16*4)(lpout2p)
    stfs fp17, (12*16*4)(lpout2p)
    stfs fp16, (14*16*4)(lpout2p)
    stfs fp0, (16*16*4)(lpout2p)
    
    REORD_PAIR_TO_R( temp1, 0,16,8,24,2,   1,17,9,25,6 )
    REORD_PAIR_TO_R( temp1, 2,18,10,26,14, 3,19,11,27,10 )
    REORD_PAIR_TO_R( temp1, 4,20,12,28,30, 5,21,13,29,26 )
    REORD_PAIR_TO_R( temp1, 6,22,14,30,18, 7,23,15,31,22 )

    lfs fp16, (4*4)(barrayp)
    lfs fp17, (12*4)(barrayp)
    lfs fp18, (28*4)(barrayp)
    lfs fp19, (20*4)(barrayp)
    REORD_PAIR_R( 0,4,fp16,      1,5,fp17 )
    REORD_PAIR_R( 2,6,fp18,      3,7,fp19 )
    REORD_PAIR_R( 8,12,fp16,    9,13,fp17 )
    REORD_PAIR_R( 10,14,fp18,  11,15,fp19 )

    lfs fp16, (8*4)(barrayp)
    lfs fp17, (24*4)(barrayp)
    REORD_PAIR_R( 0,2,fp16,     1,3,fp17 )
    REORD_PAIR_R( 4,6,fp16,     5,7,fp17 )
    REORD_PAIR_R( 8,10,fp16,   9,11,fp17 )
    REORD_PAIR_R( 12,14,fp16, 13,15,fp17 )

    lfs fp17, (16*4)(barrayp)
    fsubs fp16, fp0, fp1
    fadds fp0, fp0, fp1
    fmuls fp1, fp16, fp17

    fsubs fp16, fp2, fp3
    fadds fp2, fp2, fp3
    fmuls fp3, fp16, fp17

    fadds fp16, fp4, fp5
    fsubs fp5, fp4, fp5
    fadds fp4, fp16, fp0
    fmadds fp5, fp5, fp17, fp1

    fsubs fp16, fp6, fp7
    fadds fp6, fp6, fp7
    fmadds fp7, fp16, fp17, fp0
    fadds fp6, fp6, fp0
    fadds fp7, fp7, fp1
    fadds fp6, fp6, fp2
    fadds fp7, fp7, fp3

    fsubs fp16, fp8, fp9
    fadds fp8, fp8, fp9
    fmuls fp9, fp16, fp17

    fsubs fp16, fp10, fp11
    fadds fp10, fp10, fp11
    fmadds fp11, fp16, fp17, fp9
    fadds fp10, fp10, fp8
    fadds fp11, fp11, fp8

    fadds fp16, fp12, fp13
    fsubs fp13, fp12, fp13
    fsubs fp12, fp16, fp4
    fmsubs fp13, fp13, fp17, fp5
    fadds fp13, fp13, fp12

    fadds fp16, fp14, fp15
    fsubs fp15, fp14, fp15
    fsubs fp14, fp16, fp6
    fmsubs fp15, fp15, fp17, fp7

    fneg fp16, fp14
    fadds fp18, fp2, fp4
    fsubs fp17, fp10, fp6
    fsubs fp19, fp12, fp2
    fsubs fp18, fp18, fp10
    stfs fp16, (1*16*4)(lpout2p)
    stfs fp17, (3*16*4)(lpout2p)
    stfs fp18, (5*16*4)(lpout2p)
    stfs fp19, (7*16*4)(lpout2p)

    fneg fp16, fp12
    fsubs fp17, fp8, fp4
    fsubs fp18, fp0, fp8
    fneg fp19, fp0
    stfs fp16, (9*16*4)(lpout2p)
    stfs fp17, (11*16*4)(lpout2p)
    stfs fp18, (13*16*4)(lpout2p)
    stfs fp19, (15*16*4)(lpout2p)

    fsubs fp16, fp13, fp2
    fsubs fp17, fp4, fp9
    fsubs fp18, fp5, fp10
    fadds fp17, fp17, fp2
    fsubs fp19, fp9, fp6
    fadds fp17, fp17, fp18
    fsubs fp18, fp10, fp1
    fsubs fp20, fp1, fp14
    fadds fp18, fp18, fp19
    stfs fp20, (1*16*4)(lpout1p)
    stfs fp18, (3*16*4)(lpout1p)
    stfs fp17, (5*16*4)(lpout1p)
    stfs fp16, (7*16*4)(lpout1p)

    fsubs fp18, fp11, fp3
    fsubs fp19, fp3, fp13
    fsubs fp18, fp18, fp4
    fsubs fp17, fp7, fp11
    fsubs fp18, fp18, fp5
    stfs fp19, (9*16*4)(lpout1p)
    stfs fp18, (11*16*4)(lpout1p)
    stfs fp17, (13*16*4)(lpout1p)
    stfs fp15, (15*16*4)(lpout1p)

    END_FUNCTION(PPC_poly_filter)
#undef srcp
#undef lpout2
#undef lpout1
#undef phase
#undef barray
#undef lpsrc

//#############################################################################
//##                                                                         ##
//## Apply inverse window and write sample data                              ##
//##                                                                         ##
//#############################################################################

#define MAC_INIT_PAIR( f1, f2, f3, f4 ) ;\
  lfs fp0, (f1)(srcp)                   ;\
  lfs fp1, (f3)(srcp)                   ;\
  lfs fp2, (f2)(windp)                  ;\
  lfs fp3, (f4)(windp)                  ;\
  fmuls fp0, fp0, fp2                   ;\
  fmuls fp1, fp1, fp3                   ;

#define MAC_INIT_PAIR_LOOP( f1, f2, f3, f4, srcd, wind ) ;\
  lfs fp0, (f1)(srcp)                   ;\
  lfs fp1, (f3)(srcp)                   ;\
  lfs fp10, ((f1)+(srcd))(srcp)         ;\
  lfs fp11, ((f3)+(srcd))(srcp)         ;\
  lfs fp2, (f2)(windp)                  ;\
  lfs fp3, (f4)(windp)                  ;\
  lfs fp4, ((f2)+(wind))(windp)         ;\
  lfs fp5, ((f4)+(wind))(windp)         ;\
  fmuls fp0, fp0, fp2                   ;\
  fmuls fp1, fp1, fp3                   ;\
  fmuls fp10, fp10, fp4                 ;\
  fmuls fp11, fp11, fp5                 ;


#define MAC_PAIR( f1, f2, f3, f4 ) ;\
  lfs fp2, (f1)(srcp)              ;\
  lfs fp3, (f3)(srcp)              ;\
  lfs fp4, (f2)(windp)             ;\
  lfs fp5, (f4)(windp)             ;\
  fmadds fp0, fp2, fp4, fp0        ;\
  fmadds fp1, fp3, fp5, fp1        ;\

#define MAC_PAIR_LOOP( f1, f2, f3, f4, srcd, wind ) ;\
  lfs fp2, (f1)(srcp)              ;\
  lfs fp3, (f3)(srcp)              ;\
  lfs fp6, ((f1)+(srcd))(srcp)     ;\
  lfs fp7, ((f3)+(srcd))(srcp)     ;\
  lfs fp4, (f2)(windp)             ;\
  lfs fp5, (f4)(windp)             ;\
  lfs fp8, ((f2)+(wind))(windp)    ;\
  lfs fp9, ((f4)+(wind))(windp)    ;\
  fmadds fp0, fp2, fp4, fp0        ;\
  fmadds fp1, fp3, fp5, fp1        ;\
  fmadds fp10, fp6, fp8, fp10      ;\
  fmadds fp11, fp7, fp9, fp11      ;


#define clip( v )                \
  xoris  temp1, v, 0x8000       ;\
  subfc  temp2, neg32768, temp1 ;\
  subfe  temp1, temp1, temp1    ;\
  andc   temp2, temp2, temp1    ;\
  addi   v, temp2, -32768       ;\
  xoris  temp1, v, 0x8000       ;\
  subfc  temp2, pos32767, temp1 ;\
  subfe  temp1, temp1, temp1    ;\
  and    temp2, temp2, temp1    ;\
  addi   v, temp2, 32767        ;\

#define clip2( v1, v2 )          \
  xoris  temp1, v1, 0x8000      ;\
  xoris  temp3, v2, 0x8000      ;\
  subfc  temp2, neg32768, temp1 ;\
  subfe  temp1, temp1, temp1    ;\
  subfc  temp4, neg32768, temp3 ;\
  subfe  temp3, temp3, temp3    ;\
  andc   temp2, temp2, temp1    ;\
  andc   temp4, temp4, temp3    ;\
  addi   v1, temp2, -32768      ;\
  addi   v2, temp4, -32768      ;\
  xoris  temp1, v1, 0x8000      ;\
  xoris  temp3, v2, 0x8000      ;\
  subfc  temp2, pos32767, temp1 ;\
  subfe  temp1, temp1, temp1    ;\
  subfc  temp4, pos32767, temp3 ;\
  subfe  temp3, temp3, temp3    ;\
  and    temp2, temp2, temp1    ;\
  and    temp4, temp4, temp3    ;\
  addi   v1, temp2, 32767       ;\
  addi   v2, temp4, 32767       ;\


#define MAC_END_SUM_LOOP        ;\
  fadds fp0, fp0, fp1           ;\
  fadds fp10, fp10, fp11        ;\
  fctiwz fp0, fp0               ;\
  fctiwz fp10, fp10             ;\
  stfd fp0, tempc(sp)           ;\
  stfd fp10, tempc2(sp)         ;\
  lwz temp0, (tempc+(1*4))(sp)  ;\
  lwz temp10, (tempc2+(1*4))(sp);\
  clip2( temp0, temp10 )        ;\
  add temp1, destp, stepr       ;\
  sthx temp0, 0, destp          ;\
  sthx temp10, 0, temp1         ;\
  add destp, temp1, stepr       ;

#define MAC_END_DIF_LOOP        ;\
  fsubs fp0, fp0, fp1           ;\
  fsubs fp10, fp10, fp11        ;\
  fctiwz fp0, fp0               ;\
  fctiwz fp10, fp10             ;\
  stfd fp0, tempc(sp)           ;\
  stfd fp10, tempc2(sp)         ;\
  lwz temp0, (tempc+(1*4))(sp)  ;\
  lwz temp10, (tempc2+(1*4))(sp);\
  clip2( temp0, temp10 )        ;\
  add temp1, destp, stepr       ;\
  sthx temp0, 0, destp          ;\
  sthx temp10, 0, temp1         ;\
  add destp, temp1, stepr       ;

#define MAC_END_SUM             ;\
  fadds fp0, fp0, fp1           ;\
  fctiwz fp0, fp0               ;\
  stfd fp0, tempc(sp)           ;\
  lwz temp0, (tempc+(1*4))(sp)  ;\
  clip( temp0 )                 ;\
  sthx temp0, 0, destp          ;\
  add destp, destp, stepr       ;

#define MAC_END_DIF             ;\
  fsubs fp0, fp0, fp1           ;\
  fctiwz fp0, fp0               ;\
  stfd fp0, tempc(sp)           ;\
  lwz temp0, (tempc+(1*4))(sp)  ;\
  clip( temp0 )                 ;\
  sthx temp0, 0, destp          ;\
  add destp, destp, stepr       ;

FUNCTION(PPC_dewindow_and_write,64)

#define u           r3
#define dewindow    r4
#define start       r5
#define samples     r6
#define output_step r7
#define div         r8

#if 0
void AILCALL PPC_dewindow_and_write(F32 FAR *u,           //r3
                                    F32 FAR *dewindow,    //r4
                                    S32      start,       //r5
                                    S16 FAR *samples,     //r6
                                    S32      output_step, //r7
                                    S32      div)         //r8
{
//  F64 tempc,tempc2;
//  S32 save[4];
#endif


#define tempc   (0*8+8)
#define tempc2  (1*8+8)
#define save    (2*8+8)

    #define srcp   r3
    #undef destp
    #define destp  r6
    #define startr r5
    #undef windp
    #define windp  r4
    #define stepr  r7
    #define divr   r8

    #define temp0    r0
    #define temp10   r14
    #undef temp1
    #define temp1    r9
    #define temp2    r10
    #define temp3    r15
    #define temp4    r16
    #define neg32768 r12
    #define pos32767 r17

    stw r14, ((0*4)+save)(sp)
    stw r15, ((1*4)+save)(sp)
    stw r16, ((2*4)+save)(sp)
    stw r17, ((3*4)+save)(sp)
    
    slwi startr, startr, 2

    lis    neg32768, -32768
    lis    pos32767, -32768
    subi   neg32768, neg32768, 32768
    addi   pos32767, pos32767, 32767

    addi windp, windp, 16*4
    sub  windp, windp, startr

    add startr, startr, startr
    subi startr, startr, 48*4

    //
    //First 16 samples
    //

    addi r11, srcp, 16*16*4

__for_loops:
    MAC_INIT_PAIR_LOOP( 0,0,4,4, 16*4, 32*4 )

    MAC_PAIR_LOOP( 8, 8, (8+4), (8+4), 16*4, 32*4 )
    MAC_PAIR_LOOP( 16,16,(16+4),(16+4), 16*4, 32*4 )
    MAC_PAIR_LOOP( 24,24,(24+4),(24+4), 16*4, 32*4 )
    MAC_PAIR_LOOP( 32,32,(32+4),(32+4), 16*4, 32*4 )
    MAC_PAIR_LOOP( 40,40,(40+4),(40+4), 16*4, 32*4 )
    MAC_PAIR_LOOP( 48,48,(48+4),(48+4), 16*4, 32*4 )
    MAC_PAIR_LOOP( 56,56,(56+4),(56+4), 16*4, 32*4 )

    addi windp, windp, 32*4 *2
    addi srcp, srcp, 16*4   *2

    MAC_END_SUM_LOOP

    cmpw srcp, r11
    bne  __for_loops

    andi. r0, divr, 1
    beq __even_segment

   //
   //Odd segment, 17th sample
   //

   MAC_INIT_PAIR( 0,0,8,8 )

   MAC_PAIR( 16, 16, (16+8), (16+8) )
   MAC_PAIR( 32, 32, (32+8), (32+8) )
   MAC_PAIR( 48, 48, (48+8), (48+8) )

   MAC_END_SUM

   add windp, windp, startr

   //
   //Odd segment, 14 samples
   //

   subi r11, srcp, 14*16*4

__odd_loops:
   subi srcp, srcp,16*4

   MAC_INIT_PAIR_LOOP( 4,((15*4)-4),0,(15*4), -16*4, -32*4 )

   MAC_PAIR_LOOP( (8+4), ((15*4)-8-4), 8, ((15*4)-8), -16*4, -32*4 )
   MAC_PAIR_LOOP( (16+4),((15*4)-16-4),16,((15*4)-16), -16*4, -32*4 )
   MAC_PAIR_LOOP( (24+4),((15*4)-24-4),24,((15*4)-24), -16*4, -32*4 )
   MAC_PAIR_LOOP( (32+4),((15*4)-32-4),32,((15*4)-32), -16*4, -32*4 )
   MAC_PAIR_LOOP( (40+4),((15*4)-40-4),40,((15*4)-40), -16*4, -32*4 )
   MAC_PAIR_LOOP( (48+4),((15*4)-48-4),48,((15*4)-48), -16*4, -32*4 )
   MAC_PAIR_LOOP( (56+4),((15*4)-56-4),56,((15*4)-56), -16*4, -32*4 )

   subi windp, windp, 32*4 *2
   subi srcp, srcp,16*4

   MAC_END_DIF_LOOP

   cmpw srcp,r11
   bne __odd_loops

   //
   //Odd segment, last sample
   //

   subi srcp, srcp,16*4

   MAC_INIT_PAIR( 4,((15*4)-4),0,(15*4) )

   MAC_PAIR( (8+4), ((15*4)-8-4), 8, ((15*4)-8) )
   MAC_PAIR( (16+4),((15*4)-16-4),16,((15*4)-16) )
   MAC_PAIR( (24+4),((15*4)-24-4),24,((15*4)-24) )
   MAC_PAIR( (32+4),((15*4)-32-4),32,((15*4)-32) )
   MAC_PAIR( (40+4),((15*4)-40-4),40,((15*4)-40) )
   MAC_PAIR( (48+4),((15*4)-48-4),48,((15*4)-48) )
   MAC_PAIR( (56+4),((15*4)-56-4),56,((15*4)-56) )

   MAC_END_DIF

   b __exit

   //
   //Even segment, 17th sample
   //

__even_segment:
   MAC_INIT_PAIR( 4,4,12,12 )

   MAC_PAIR( (16+4), (16+4), (16+12), (16+12) )
   MAC_PAIR( (32+4), (32+4), (32+12), (32+12) )
   MAC_PAIR( (48+4), (48+4), (48+12), (48+12) )
   MAC_END_SUM

   add windp, windp, startr

   //
   //Even segment, 14 samples
   //

   subi r11, srcp, 14*16*4

__even_loops:
   subi srcp, srcp, 16*4 

   MAC_INIT_PAIR_LOOP( 0,(15*4),4,((15*4)-4), -16*4, -32*4 )

   MAC_PAIR_LOOP( 8, ((15*4)-8), (8+4), ((15*4)-8-4), -16*4, -32*4 )
   MAC_PAIR_LOOP( 16,((15*4)-16),(16+4),((15*4)-16-4), -16*4, -32*4 )
   MAC_PAIR_LOOP( 24,((15*4)-24),(24+4),((15*4)-24-4), -16*4, -32*4 )
   MAC_PAIR_LOOP( 32,((15*4)-32),(32+4),((15*4)-32-4), -16*4, -32*4 )
   MAC_PAIR_LOOP( 40,((15*4)-40),(40+4),((15*4)-40-4), -16*4, -32*4 )
   MAC_PAIR_LOOP( 48,((15*4)-48),(48+4),((15*4)-48-4), -16*4, -32*4 )
   MAC_PAIR_LOOP( 56,((15*4)-56),(56+4),((15*4)-56-4), -16*4, -32*4 )

   subi windp, windp, 32*4 *2
   subi srcp, srcp, 16*4 

   MAC_END_DIF_LOOP

   cmp cr0,srcp,r11
   bne __even_loops

   //
   //Even segment, last sample
   //

   subi srcp, srcp, 16*4

   MAC_INIT_PAIR( 0,(15*4),4,((15*4)-4) )

   MAC_PAIR( 8, ((15*4)-8), (8+4), ((15*4)-8-4) )
   MAC_PAIR( 16,((15*4)-16),(16+4),((15*4)-16-4) )
   MAC_PAIR( 24,((15*4)-24),(24+4),((15*4)-24-4) )
   MAC_PAIR( 32,((15*4)-32),(32+4),((15*4)-32-4) )
   MAC_PAIR( 40,((15*4)-40),(40+4),((15*4)-40-4) )
   MAC_PAIR( 48,((15*4)-48),(48+4),((15*4)-48-4) )
   MAC_PAIR( 56,((15*4)-56),(56+4),((15*4)-56-4) )

   MAC_END_DIF

__exit:

    lwz r14, ((0*4)+save)(sp)
    lwz r15, ((1*4)+save)(sp)
    lwz r16, ((2*4)+save)(sp)
    lwz r17, ((3*4)+save)(sp)
    END_FUNCTION(PPC_dewindow_and_write)

   
#if 0
static char marker[]={ "\r\n\r\nMiles Sound System\r\n\r\n"
                       "Copyright (C) 1991-2002 RAD Game Tools, Inc.\r\n\r\n"};
#endif

