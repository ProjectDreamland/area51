//=============================================================================

#ifndef _VU1_HPP_
#define _VU1_HPP_

#include "Entropy.hpp"

//=============================================================================

#define VU1_USE_ALPHA       (1 << 0)
#define VU1_NO_TEXTURE      (1 << 1)
#define VU1_NO_GOURAUD      (1 << 2)

//=============================================================================

void    vu1_Init    ( const matrix4& C2S );
void    vu1_SetL2S  ( const matrix4& L2S );
void    vu1_SetL2C  ( const matrix4& L2C );
void    vu1_SetC2S  ( const matrix4& C2S );

void    vu1_Begin   ( u32 Flags = 0 );
void    vu1_End     ( void );

void    vu1_Break   ( void );

void    vu1_Render  ( const vector4* pVert,
                      const s16*     pUV,
                      const xcolor*  pCol,
                      s32            NumVerts,
                      xbool          DoClipping = FALSE );

void    vu1_Render  ( const vector4* pVert,
                      const s16*     pUV,
                      const u16*     pCol,
                      s32            NumVerts,
                      xbool          DoClipping = FALSE );

void    vu1_Render  ( const vector4* pVert,
                      const s16*     pUV,
                      const vector3* pNorm,
                      s32            NumVerts,
                      xbool          DoClipping = FALSE );

//=============================================================================

#endif
