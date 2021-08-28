/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/* 
 *                      Emotion Engine Library
 *                          Version 0.10
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-1999 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                        libvu0 - libvu0.h
 *                       Header File for VU0 Macro Code Library
 *
 *       Version        Date            Design      Log
 *  --------------------------------------------------------------------
 *       0.10           Mar,25,1999     shino 
 */

#ifndef _LIB_VU0_H_
#define _LIB_VU0_H_

#include "eetypes.h"

#ifdef __cplusplus
extern "C" {
#endif

/* basic type */
typedef int		qword[4] 	__attribute__ ((aligned(16)));
typedef int		sceVu0IVECTOR[4] __attribute__((aligned (16)));
typedef float		sceVu0FVECTOR[4] __attribute__((aligned (16)));
typedef float		sceVu0FMATRIX[4][4] __attribute__((aligned (16)));

/* prototypes */
void  sceVu0CopyVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1);
void  sceVu0CopyVectorXYZ(sceVu0FVECTOR v0, sceVu0FVECTOR v1);
void  sceVu0FTOI0Vector(sceVu0IVECTOR v0, sceVu0FVECTOR v1);
void  sceVu0FTOI4Vector(sceVu0IVECTOR v0, sceVu0FVECTOR v1);
void  sceVu0ITOF0Vector(sceVu0FVECTOR v0, sceVu0IVECTOR v1);
void  sceVu0ITOF4Vector(sceVu0FVECTOR v0, sceVu0IVECTOR v1);
void  sceVu0ScaleVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float s);
void  sceVu0ScaleVectorXYZ(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float s);
void  sceVu0AddVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2);
void  sceVu0SubVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2);
void  sceVu0MulVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2);
void  sceVu0InterVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2, float r);
void  sceVu0InterVectorXYZ(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2, float r);
void  sceVu0DivVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float q);
void  sceVu0DivVectorXYZ(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float q);
float sceVu0InnerProduct(sceVu0FVECTOR v0, sceVu0FVECTOR v1);
void  sceVu0OuterProduct(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2);
void  sceVu0Normalize(sceVu0FVECTOR v0, sceVu0FVECTOR v1);
void  sceVu0ApplyMatrix(sceVu0FVECTOR v0, sceVu0FMATRIX m, sceVu0FVECTOR v1);
void  sceVu0UnitMatrix(sceVu0FMATRIX m);
void  sceVu0CopyMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1);
void  sceVu0TransposeMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1);
void  sceVu0MulMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FMATRIX m2);
void  sceVu0InversMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1);
void  sceVu0RotMatrixX(sceVu0FMATRIX m0, sceVu0FMATRIX m1, float rx);
void  sceVu0RotMatrixY(sceVu0FMATRIX m0, sceVu0FMATRIX m1, float ry);
void  sceVu0RotMatrixZ(sceVu0FMATRIX m0, sceVu0FMATRIX m1, float rz);
void  sceVu0RotMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FVECTOR rot);
void  sceVu0TransMatrix(sceVu0FMATRIX m0, sceVu0FMATRIX m1, sceVu0FVECTOR tv);
void  sceVu0CameraMatrix(sceVu0FMATRIX m, sceVu0FVECTOR p, sceVu0FVECTOR zd, sceVu0FVECTOR yd);
void  sceVu0NormalLightMatrix(sceVu0FMATRIX m, sceVu0FVECTOR l0, sceVu0FVECTOR l1, sceVu0FVECTOR l2);
void  sceVu0LightColorMatrix(sceVu0FMATRIX m, sceVu0FVECTOR c0, 
						sceVu0FVECTOR c1, sceVu0FVECTOR c2, sceVu0FVECTOR a);
void  sceVu0ViewScreenMatrix(sceVu0FMATRIX m, float scrz, float ax, float ay, 
	float cx, float cy, float zmin, float zmax, float nearz, float farz);
void  sceVu0DropShadowMatrix(sceVu0FMATRIX m, 
				sceVu0FVECTOR lp, float a, float b, float c, int mode);
int   sceVu0ClipAll(sceVu0FVECTOR minv, sceVu0FVECTOR maxv, sceVu0FMATRIX ms, sceVu0FVECTOR *vm, int n);
void sceVu0ClampVector(sceVu0FVECTOR v0, sceVu0FVECTOR v1, float min, float max);
int sceVu0ClipScreen(sceVu0FVECTOR v0);
int sceVu0ClipScreen3(sceVu0FVECTOR v0, sceVu0FVECTOR v1, sceVu0FVECTOR v2);
void sceVu0RotTransPersN(sceVu0IVECTOR *v0, sceVu0FMATRIX m0, sceVu0FVECTOR *v1, int n, int mode);
void sceVu0RotTransPers(sceVu0IVECTOR v0, sceVu0FMATRIX m0, sceVu0FVECTOR v1, int mode);
void sceVpu0Reset(void);

extern void *sceVu0GetErxEntries(void);

#ifdef __cplusplus
}
#endif

#endif /*  _LIB_VU0_H_ */
