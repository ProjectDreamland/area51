//==============================================================================
//
//  GeomBone.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================
#include "GeomBone.hpp"
#include "Entropy.hpp"


//==============================================================================
// CLASSES
//==============================================================================

geom_bone::geom_bone()
{
    m_StickBone   = -1 ;
    m_BindT       = 0 ;
    m_SkinBind.Identity() ;
    m_RagdollInvBind.Identity() ;
    m_LocalBBox.Clear() ;
    
#ifdef X_DEBUG    
    m_pName = NULL ;
    m_Color = XCOLOR_WHITE ;
#endif    
}

//==============================================================================

void geom_bone::Init( const char*       pName,
                      stick_bone        StickBones[],
                      s32               StickBone,
                      f32               BindT,
                      const matrix4&    SkinBind,
                      bbox&             LocalBBox,
                      xcolor            Color,
                      xbool             bAnimController )
{
    // Keep info
    m_StickBone   = StickBone ;
    m_BindT       = BindT ;
    m_SkinBind    = SkinBind ;
    m_LocalBBox   = LocalBBox ;

#ifdef X_DEBUG    
    m_pName       = pName ;
    m_Color       = Color ;
#else
    (void)pName;
    (void)Color;
#endif    

    // Compute ragdoll inverse bind matrix
    m_RagdollInvBind = GetRagdollL2W(StickBones) ;  // Get ragdoll bind
    m_RagdollInvBind.Invert() ;                     // Convert to inverse

    // Setup for correct playback in an animation controller
    // (animation performs a multiply by the SkinInvBind at the very end,
    //  so this counter-acts it)
    if(bAnimController)
        m_RagdollInvBind = m_RagdollInvBind * SkinBind ;
}

//==============================================================================

#ifdef X_DEBUG

void geom_bone::Render( stick_bone StickBones[] )
{
    ASSERT(m_StickBone != -1) ;

    matrix4 L2W ;
    
    GetBBoxL2W(StickBones, L2W) ;
    draw_SetL2W(L2W) ;
    draw_BBox(m_LocalBBox, m_Color) ;
    draw_ClearL2W() ;
    
    // Draw bind point
    //StickBones[m_StickBone].GetL2W(L2W, m_BindT) ;
    //L2W = StickBones[m_StickBone].GetL2W() ;
    //draw_Point(L2W.GetTranslation(), XCOLOR_WHITE) ;
}

#endif

//==============================================================================
