//==============================================================================
//
//  GeomBone.hpp
//
//==============================================================================

#ifndef __GEOM_BONE_HPP__
#define __GEOM_BONE_HPP__

//==============================================================================
// INCLUDES
//==============================================================================
#include "x_files.hpp"
#include "x_math.hpp"
#include "StickBone.hpp"


//==============================================================================
// CLASSES
//==============================================================================

// Class that maps a geometry bone to a stick bone
struct geom_bone
{
// Data
public:
    matrix4         m_SkinBind ;        // Skin bind matrix of this bone
    matrix4         m_RagdollInvBind ;  // Inverse ragdoll bind matrix of this bone
    s32             m_StickBone ;       // Index of stick bone that it's attached too
    f32             m_BindT ;           // BindT of stick bone that it's attached too
    bbox            m_LocalBBox ;       // Local space bounding box of bones verts
    
#ifdef X_DEBUG
    const char*     m_pName ;           // Name
    xcolor          m_Color ;           // Debug color
#endif
                    
// Functions
public:

    // Constructor
         geom_bone() ;

    // Initialization functions
    void Init( const char*      pName,
               stick_bone       StickBones[],
               s32              StickBone,
               f32              BindT,
               const matrix4&   SkinBind,
               bbox&            LocalBBox,
               xcolor           Color,
               xbool            bAnimController = FALSE );
         
    // Matrix functions
    matrix4& GetRagdollL2W( stick_bone StickBones[] ) ;
    void     GetSkinL2W   ( stick_bone StickBones[], matrix4& SkinL2W ) ;
    void     GetBBoxL2W   ( stick_bone StickBones[], matrix4& L2W ) ;

#ifdef X_DEBUG

    // Render functions
    void Render( stick_bone StickBones[] ) ;

#endif

} PS2_ALIGNMENT(16) ;

//==============================================================================

// Returns local to world matrix for ragdoll stick bones
inline
matrix4& geom_bone::GetRagdollL2W( stick_bone StickBones[] )
{
    ASSERT(m_StickBone != -1) ;
    return StickBones[m_StickBone].GetL2W() ;
}

//==============================================================================

// Returns local to world for skinned geometry
inline
void geom_bone::GetSkinL2W( stick_bone StickBones[], matrix4& SkinL2W )
{
    /*
    We need the mapping from from RagBind -> SkinBind
    i.e. RagBind * Offset = SkinBind

    Now:
    matrix4 Offset = m_RagdollInvBind * m_SkinBind ;

    So we have:
    return RagdollL2W * (m_RagdollInvBind * m_SkinBind) * m_SkinInvBind ;

    // Which nicely optimizes down to the simple:
    RagdollL2W * m_RagdollInvBind
    */

    // Lookup stick bone L2W
    matrix4& RagdollL2W = GetRagdollL2W( StickBones ) ;

    SkinL2W = RagdollL2W * m_RagdollInvBind ;
}

//==============================================================================

// Returns local to world for local bounding box
// (not time critical - only used to init ragdoll def)
inline
void geom_bone::GetBBoxL2W ( stick_bone StickBones[], matrix4& L2W )
{
    matrix4& RagdollL2W = GetRagdollL2W(StickBones) ;
    L2W = RagdollL2W * m_RagdollInvBind * m_SkinBind ;
}

//==============================================================================

#endif  // #ifndef __GEOM_BONE_HPP__
