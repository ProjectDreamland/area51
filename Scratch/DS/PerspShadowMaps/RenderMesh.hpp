#ifndef __RENDERMESH_HPP_INCLUDED__
#define __RENDERMESH_HPP_INCLUDED__

#include "Entropy.hpp"
#include "Auxiliary\Bitmap\aux_Bitmap.hpp"
#include "RawMesh.hpp"

xhandle     InitializeMeshData      ( rawmesh&           Mesh );
void        RenderMeshToShadowMap   ( xhandle            Handle,
                                      xbitmap&           BMP,
                                      matrix4&           L2W,
                                      u32                Color   );
void        RenderMeshWithShadow    ( xhandle            Handle,
                                      xbitmap&           BMP,
                                      IDirect3DTexture8* pShadTexture,
                                      matrix4&           L2W,
                                      matrix4&           ShadowProj,
                                      xbool              UseTexW,
                                      xbool              Project );
void        RenderMesh              ( xhandle            Handle,
                                      xbitmap&           BMP,
                                      matrix4&           L2W,
                                      u32                Color );
void        KillMeshData            ( xhandle            Handle );

#endif // __RENDERMESH_HPP_INCLUDED__
