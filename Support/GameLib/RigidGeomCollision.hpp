//==============================================================================
//
//  RigidGeomCollision.hpp
//
//==============================================================================

#ifndef RIGID_GEOM_COLLISION_HPP
#define RIGID_GEOM_COLLISION_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "MiscUtils\Guid.hpp"
#include "Render\RigidGeom.hpp"
#include "Objects\Object.hpp"
#include "Render\Render.hpp"

//==============================================================================
//  FUNCTIONS
//==============================================================================

void RigidGeom_ApplyCollision( guid              Guid,
                               const bbox&       BBox,
                               const u64         MeshMask,      
                               const matrix4*    pL2W, 
                               const rigid_geom* pRigidGeom );

xbool RigidGeom_GetColDetails( const rigid_geom*           pRigidGeom,
                               const matrix4*        pL2W,
                               const void*           pColor,
                               s32                   Key,
                               object::detail_tri&   Tri );

xbool RigidGeom_GetTriangle( const rigid_geom*          pRigidGeom,
                             s32                   Key,
                             vector3&              P0,
                             vector3&              P1,
                             vector3&              P2);

s32 RigidGeom_GetBoneIndexFromPrimKey( const rigid_geom&    RigidGeom, 
                                       s32                  PrimKey );

void RigidGeom_GatherLoTris( const u64         MeshMask,
                             const matrix4*    pL2W, 
                             const rigid_geom& RigidGeom,
                             const bbox&       GatherBBox,
                             vector3*&         pTriVert,
                             s32&              nTris);

#ifndef X_RETAIL
void RigidGeom_RenderCollision( const matrix4*  pBone, 
                               const rigid_geom*      pRigidGeom, 
                               xbool            bRenderHigh, 
                               u64              LODMask );
#endif

void RigidGeom_GatherToPolyCache(       guid       Guid,
                               const bbox&       BBox,
                               const u64         MeshMask,      
                               const matrix4*    pL2W, 
                               const rigid_geom* pRigidGeom );

//==============================================================================
#endif
//==============================================================================
