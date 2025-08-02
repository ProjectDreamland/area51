//==============================================================================
//
//  File:           ViewerObject.hpp
//
//  Description:    Viewer object class
//
//  Author:         Stephen Broumley
//
//  Date:           Started July29th, 2003 (C) Inevitable Entertainment Inc.
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================
#include "ViewerObject.hpp"
#include "Config.hpp"
#include "Util.hpp"
#include "Controls.hpp"
#include "Main.hpp"
#include "EventMgr\EventMgr.hpp"
#include "Objects\Player.hpp"


//==============================================================================
// Constants
//==============================================================================
//static f32 LOCO_BLEND_SPEED_AIM = 0.01f ;

//==============================================================================
// UTIL FUNCTIONS
//==============================================================================

// Draws marker with z buffering
void Util_DrawMarker( const vector3& Pos,
                            xcolor   Color,
                            s32      Size )
{
    const view* pView = eng_GetView();

    // Get screen coordinates of projected point
    vector3 ScreenPos;
    ScreenPos = pView->PointToScreen( Pos );

    // Infront of screen?
    if( ScreenPos.GetZ() < 0.0f )
        return ;

    // Get screen viewport bounds
    s32 X0,Y0,X1,Y1;
    pView->GetViewport(X0,Y0,X1,Y1);

    // Peg screen coordinates to just outside viewport
    ScreenPos.Max( vector3((f32)X0-16, (f32)Y0-16, 0.0f) );
    ScreenPos.Min( vector3((f32)X1+16, (f32)Y1+16, 0.0f) );

    // Make screen pos relative to viewport
    ScreenPos -= vector3( (f32)X0, (f32)Y0, 0.0f );

    // Draw with z buffering
    draw_Begin( DRAW_QUADS, DRAW_2D | DRAW_2D_KEEP_Z );
    {
        draw_Color( Color );
        draw_Vertex( ScreenPos.GetX(),       ScreenPos.GetY()-Size, 0 );
        draw_Vertex( ScreenPos.GetX()-Size,  ScreenPos.GetY(),      0 );
        draw_Vertex( ScreenPos.GetX(),       ScreenPos.GetY()+Size, 0 );
        draw_Vertex( ScreenPos.GetX()+Size,  ScreenPos.GetY(),      0 );
    }
    draw_End();
}

//==============================================================================
// VIEWER OBJECT FUNCTIONS
//==============================================================================

// Constructor/destructor
viewer_object::viewer_object()
{
    // Type
    m_CompiledGeom[0] = 0;
    m_pConfigObject = NULL;                 // Owner config object
    m_RenderType    = RENDER_NULL ;         // Render type - skin or rigid
    m_Type          = config_options::TYPE_NULL ;   // Type of object

    // Resources
    m_bAudioSet  = FALSE ;          // TRUE if audio is in config file

    // Render
    m_pGeom      = NULL ;           // Geometry
    m_hInst      = HNULL ;
    m_nColors    = 0 ;              // # of colors
    m_pColors    = NULL ;           // List of rigid colors
    m_LODs.Clear();

    // Position/Animation
    m_L2W.Identity() ;              // Local to world
    m_iLastAnim    = 0 ;            // Index of last anim
    m_bAnimPaused  = FALSE;         // Anim paused flag

    // Link info
    m_pParentObject = NULL ;        // Parent object (or NULL)
    m_iParentBone   = -1 ;          // Parent bone attached to (or -1)

    // Runtime
    m_iAnim        = 0 ;            // Index of animation to play
    m_nActiveBones = 0 ;            // # of active bones in view
    m_iLOD         = 0 ;            // LOD in view
    m_iMaterials   = 0 ;            // # of materials
    m_nVerts       = 0 ;            // Verts in view
    m_nTris        = 0 ;            // Tris in view

    // Setup default view
    m_View.SetXFOV( R_60 );
    m_View.SetPosition( vector3(0, 100, -200) );
    m_View.LookAtPoint( vector3(0, 100,   0) );
    m_View.SetZLimits ( 0.1f, 10000.0f );
    
    // Setup array grow counts
    m_LODs.SetGrowAmount( 4 );
    m_pChildren.SetGrowAmount( 2 );
}

//==============================================================================

viewer_object::~viewer_object()
{
    // Cleanup
    Kill() ;
}

//==============================================================================

// Returns number of colors in rigid geometry
s32 GetColorCount( const rigid_geom* pRigidGeom )
{
    // No geom?
    if (!pRigidGeom)
        return 0 ;

    s32 Count = 0 ;
    
#ifdef TARGET_PC
    for (s32 i = 0 ; i < pRigidGeom->m_nDList ; i++)
        Count += pRigidGeom->m_System.pPC[i].nVerts ;
#endif

#ifdef TARGET_PS2
    for (s32 i = 0 ; i < pRigidGeom->m_nDList ; i++)
        Count = x_max(Count, pRigidGeom->m_System.pPS2[i].iColor + 
                             pRigidGeom->m_System.pPS2[i].nVerts) ;
#endif
    return Count ;
}

//==============================================================================

// Returns index of mesh if found, else -1
s32 viewer_object::FindMesh( const char* pName )
{
    // No mesh?
    if( !m_pGeom )
        return -1;

    // Find mesh
    return m_pGeom->GetMeshIndex( pName );
}

//==============================================================================

void viewer_object::GetLODInfo( const char* pGeomMesh, s32& iLOD, char* pMesh )
{
    // Found?
    if (        ( x_stristr( pGeomMesh, "MESH_L" ) == pGeomMesh )
            &&  ( pGeomMesh[6] >= '0' ) && ( pGeomMesh[6] <= '9' ) )
    {
        iLOD = pGeomMesh[6] - '0';
        x_strcpy( pMesh, &pGeomMesh[7] );
    }
    else
    {
        // Display on all LODs
        iLOD = 0;
        x_strcpy( pMesh, pGeomMesh );
    }
}

//==============================================================================

xbool viewer_object::IsMeshInLOD( s32 iLOD, const char* pMesh )
{
    s32 i;

    // Loop through all geometry meshes
    for( i = 0 ; i < m_pGeom->m_nMeshes; i++ )
    {
        // Lookup geom mesh
        const char* pGeomMesh = m_pGeom->GetVMeshName( i );
        
        // Get LOD info
        s32  iCheckLOD;
        char CheckMesh[64];
        GetLODInfo( pGeomMesh, iCheckLOD, CheckMesh );

        // Match?
        if( ( x_stricmp( CheckMesh, pMesh ) == 0 ) && ( iCheckLOD == iLOD ) )
            return TRUE;
    }

    // Not found
    return FALSE;
}

//==============================================================================

// Builds lod masks
void viewer_object::BuildLODs( config_options::object& Object )
{
    s32 i,j;

    // Any geometry?
    if( !m_pGeom )
        return;

    // Autobuild?
    if( Object.m_LODs.GetCount() == 0 )
    {
        // Aaron doesn't want my beautiful auto lod computation
        m_LODs.SetCount( 1 );
        m_LODs[0].m_Mask       = 0xFFFFFFFFFFFFFFFF;
        m_LODs[0].m_ScreenSize = 0;
        
/*    
        // Setup LOD count
        s32 LODCount=0;
        for( i = 0 ; i < m_pGeom->m_nMeshes; i++ )
        {
            // Lookup geom mesh
            const char* pGeomMesh = m_pGeom->m_pMesh[i].Name;
            
            // Get LOD info
            s32  iLOD;
            char Mesh[64];
            GetLODInfo( pGeomMesh, iLOD, Mesh );

            // Update limits
            LODCount = x_max( LODCount, iLOD+1 );
        }

        // Setup LODs starting with highest
        m_LODs.SetCount( LODCount );
        f32 ScreenSize = 200.0f;
        for( i = 0 ; i < LODCount ; i++ )
        {
            // Setup screen size
            m_LODs[i].m_ScreenSize = ScreenSize;
            m_LODs[i].m_Mask       = 0;

            // Half screen size
            ScreenSize *= 0.5;
        }

        // Add meshes to LODs
        for( i = 0 ; i < m_pGeom->m_nMeshes; i++ )
        {
            // Lookup geom mesh
            const char* pGeomMesh = m_pGeom->m_pMesh[i].Name;
            
            // Get LOD info
            s32  iLOD;
            char Mesh[64];
            GetLODInfo( pGeomMesh, iLOD, Mesh );

            // Add to LOD
            m_LODs[iLOD].m_Mask |= (1 << i);

            // Add to any LODs lower than this if the mesh is not already present
            for( j = iLOD+1 ; j < LODCount ; j++ )
            {
                if( IsMeshInLOD( j, Mesh ) == FALSE )
                    m_LODs[j].m_Mask |= (1 << i);
            }
        }
*/        
    }
    else
    {
        // Setup from object
        m_LODs.SetCount( Object.m_LODs.GetCount() );
        for( i = 0 ; i < Object.m_LODs.GetCount() ; i++ )
        {
            m_LODs[i].m_ScreenSize = Object.m_LODs[i].m_ScreenSize;
            m_LODs[i].m_Mask       = 0;

            for( j = 0 ; j < Object.m_LODs[i].m_Meshes.GetCount() ; j++ )
            {
                s32 iMesh = FindMesh( Object.m_LODs[i].m_Meshes[j].m_Name );
                if( iMesh != -1 )
                    m_LODs[i].m_Mask |= 1 << iMesh;
            }
        }
    }
}

//==============================================================================

// Initialize
xbool viewer_object::Init( config_options::object& ConfigObject )
{
    s32 i ;

    xbool bSuccess = TRUE;

    // Keep ptr to config object owner
    m_pConfigObject = &ConfigObject;
    
    // Reset?
    xbool bReset = (m_Type == config_options::TYPE_NULL) ;

    // Reset position?
    if (bReset)
        m_L2W.Identity() ;

    // Setup render type
    if (ConfigObject.IsSoftSkinned())
        m_RenderType = RENDER_SKIN ;
    else
        m_RenderType = RENDER_RIGID ;

    // Setup geom resource
    if (m_RenderType == RENDER_SKIN)
    {
        // Skinned
        if( ConfigObject.m_CompiledGeom[0] )
        {
            m_hSkinGeom.SetName(ConfigObject.m_CompiledGeom) ;
            skin_geom* pSkinGeom = m_hSkinGeom.GetPointer();
            if (pSkinGeom)
                m_hInst = render::RegisterSkinInstance( *pSkinGeom );
            m_pGeom = pSkinGeom ;
        }
    }        
    else
    {
        // Rigid
        if( ConfigObject.m_CompiledGeom[0] )
        {
            m_hRigidGeom.SetName(ConfigObject.m_CompiledGeom) ;
            rigid_geom* pRigidGeom = m_hRigidGeom.GetPointer();
            if (pRigidGeom)
                m_hInst = render::RegisterRigidInstance( *pRigidGeom );
            m_pGeom = pRigidGeom ;
        }
    }
    
    // Setup object type
    m_Type = ConfigObject.GetType();

    // Load audio
    m_bAudioSet = FALSE ;
    if (ConfigObject.m_CompiledAudio[0])
    {
        m_bAudioSet = TRUE ;
        m_hAudioPackage.SetName(ConfigObject.m_CompiledAudio) ;
        m_hAudioPackage.GetPointer() ;
    }

    // Setup anim resource only if bone count matches geometry!
    if (ConfigObject.m_CompiledAnim[0])
    {
        anim_group::handle hAnimGroup ;
        hAnimGroup.SetName(ConfigObject.m_CompiledAnim) ;
        const anim_group* pAnimGroup = hAnimGroup.GetPointer() ;
        if (pAnimGroup)
        {
            // If anim bones do not match geometry bones, then clear animation
            if ( (!m_pGeom) || (pAnimGroup->GetNBones() != m_pGeom->m_nBones) )
                ConfigObject.m_CompiledAnim[0] = 0 ;
        }
    }

    // Set anim group name if present
    if( ConfigObject.m_CompiledAnim[0] )
        m_hAnimGroup.SetName(ConfigObject.m_CompiledAnim) ;
    
    // Lookup anim group
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer() ;
    
    // Setup anim players?
    if (pAnimGroup)
    {
        // Setup anim player
        m_AnimPlayer.SetAnimGroup(m_hAnimGroup) ;
        m_AnimPlayer.SetAnim(0, FALSE) ;

        // Setup loco
        if( m_Type != config_options::TYPE_OBJECT )
        {
            m_Loco.OnInit( m_pGeom, ConfigObject.m_CompiledAnim, ConfigObject) ;
            m_Loco.SetLookAt(m_Loco.GetEyePosition() + vector3(0,0,-100)) ;
            m_Loco.SetYaw(R_180) ;
        
            // If this a lip sync, then there is no bind pose so go to the last frame of the anim
            if( m_Type == config_options::TYPE_LIP_SYNC )
                m_Loco.GetMotionController().SetFrameParametric(1.0f);
        }
        
        // Keep count
        m_iLastAnim = pAnimGroup->GetNAnims()-1 ;
        if (pAnimGroup->GetNAnims() > 1)
            m_iAnim = 1 ;   // 1st anim
        else
            m_iAnim = 0 ;   // Bindpose

        // Fail?
        if( m_Loco.HasWarnings() )            
            bSuccess = FALSE;
    }

    // Setup audio resource
    m_hAudioPackage.SetName(ConfigObject.m_CompiledAudio) ;

    // Setup color table?
    if ((m_pGeom) && (m_RenderType == RENDER_RIGID))
    {
#ifdef TARGET_PS2
        // Setup colors
        m_nColors = GetColorCount((rigid_geom*)m_pGeom) ;
        m_pColors = new u16[m_nColors] ;
#endif

        // Light the object
        Light() ;
    }
    else
    {
        m_nColors = 0 ;
        m_pColors = 0 ;
    }

    // Builds lod ready for rendering
    BuildLODs( ConfigObject ) ;

    // Copy name
    x_strcpy(m_CompiledGeom, ConfigObject.m_CompiledGeom) ;

    // Link to a parent?
    if (ConfigObject.m_AttachObject[0])
    {
        // Search for parent
        for (i = 0 ; i < g_Config.m_Objects.GetCount() ; i++)
        {
            // Found parent?
            if (x_stricmp(g_Config.m_Objects[i].m_Name, ConfigObject.m_AttachObject) == 0)
            {
                // Get parent
                m_pParentObject = &g_pObjects[i] ;
                ASSERT(m_pParentObject) ;

                // Update parent
                m_pParentObject->m_pChildren.Append(this) ;

                // Get bone
                m_iParentBone = m_pParentObject->FindBoneIndex(ConfigObject.m_AttachBone) ;

                // Done
                break ;
            }
        }
    }       

    // Setup view
    if (bReset)
        ResetView() ;
        
    return bSuccess;
}

//==============================================================================

// Destroy
void viewer_object::Kill( void )
{
    // Unregister instance with render system
    if (m_hInst.IsNonNull())
    {
        if (m_RenderType == RENDER_SKIN)
            render::UnregisterSkinInstance(m_hInst) ;
        else
            render::UnregisterRigidInstance(m_hInst) ;

        // Set to null
        m_hInst = HNULL ;
    }

    // Free colors
    if (m_pColors)
    {
        delete [] m_pColors ;
        m_pColors = NULL ;
        m_nColors = 0 ;
    }

    // Free handles
    m_hSkinGeom.Destroy() ;
    m_hRigidGeom.Destroy() ;
    m_hAnimGroup.Destroy() ;
    m_hAudioPackage.Destroy() ;
    m_bAudioSet = FALSE ;

    // Flag not loaded anymore
    m_RenderType = RENDER_NULL ;
    m_Type       = config_options::TYPE_NULL ;
    m_pGeom      = NULL ;
}

//==============================================================================

// Resets view so object is fully on screen
void viewer_object::ResetView( void )
{
    // Lookup object bounds info
    bbox    BBox   = GetBindWorldBBox() ;
    vector3 Center = BBox.GetCenter() ;
    f32     Radius = BBox.GetRadius() ;

#ifdef TARGET_PC
    m_View.SetPosition( vector3(Center.GetX(), Center.GetY(), Center.GetZ() - (Radius*2.0f)) ) ;
#else
    m_View.SetPosition( vector3(Center.GetX(), Center.GetY(), Center.GetZ() - (Radius*2.5f)) ) ;
#endif
    m_View.LookAtPoint( Center ) ;
}

//==============================================================================

// Returns type of anim (masked, lip sync, full body etc)
const char* viewer_object::GetAnimType( const char* pAnimName ) const    
{
    // Bad init?
    if( !m_pConfigObject )
        return "FULLBODY";
        
    // Lookup anim array
    xarray<config_options::object::anim>& Anims = m_pConfigObject->m_Anims;

    // Search through all anims
    for( s32 i = 0; i < Anims.GetCount(); i++ )
    {
        // Found?
        if( x_stricmp( Anims[i].m_Name, pAnimName ) == 0 )
            return Anims[i].m_Type;
    }
    
    // Not found - default to full body
    return "FULLBODY";
}

//==============================================================================

void viewer_object::GetAnimInfo( const char*&           pAnimName,
                                 const char*&           pAnimType,
                                 loco::bone_masks_type& MaskType,
                                 u32&                   MaskFlags )
{
    // Lookup anim group
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer() ;
    if( !pAnimGroup )
    {
        pAnimName = "NULL";
        pAnimType = "NULL";
        MaskType  = loco::BONE_MASKS_TYPE_FULL_BODY;
        MaskFlags = loco::ANIM_FLAG_MASK_TYPE_FULL_BODY;
        return;
    }

    // Lookup name and type
    pAnimName = pAnimGroup->GetAnimInfo(m_iAnim).GetName() ;
    pAnimType = GetAnimType( pAnimName );

    // Lookup bone mask type and flags
    MaskType  = loco::BONE_MASKS_TYPE_FULL_BODY;
    MaskFlags = loco::ANIM_FLAG_MASK_TYPE_FULL_BODY;
    if( x_stristr( pAnimType, "MASKED_FACE" ) )
    {
        MaskType  = loco::BONE_MASKS_TYPE_FACE;
        MaskFlags = loco::ANIM_FLAG_MASK_TYPE_FACE;
    }                    
    else if(    ( x_stristr( pAnimType, "MASKED_UPPER_BODY" ) )
        || ( x_stristr( pAnimType, "MASKED_UPPERBODY"  ) ) )
    {
        MaskType  = loco::BONE_MASKS_TYPE_UPPER_BODY;
        MaskFlags = loco::ANIM_FLAG_MASK_TYPE_UPPER_BODY;
    }                    
    else if(    ( x_stristr( pAnimType, "MASKED_RELOAD_SHOOT" ) )
        || ( x_stristr( pAnimType, "MASKED_RELOADSHOOT"  ) ) )
    {
        MaskType  = loco::BONE_MASKS_TYPE_RELOAD_SHOOT;
        MaskFlags = loco::ANIM_FLAG_MASK_TYPE_FULL_BODY;
    }                    
}

//==============================================================================

void viewer_object::PlayCurrentAnim( xbool bMovieStart )
{
    // Lookup anim info
    const char*           pAnimName;
    const char*           pAnimType;
    loco::bone_masks_type MaskType;
    u32                   MaskFlags;
    GetAnimInfo( pAnimName, pAnimType, MaskType, MaskFlags );

    // Play loco?
    if ( ( m_Type == config_options::TYPE_LOCO ) || ( m_Type == config_options::TYPE_LIP_SYNC ) )
    {
        // No pause in loco mode
        m_bAnimPaused = FALSE;

        loco_anim_controller* pCont = NULL;

        // Additive?
        if( x_stristr( pAnimType, "ADDITIVE" ) )
        {
            pCont = &m_Loco.GetAdditiveController();
            m_Loco.PlayAdditiveAnim( pAnimName, 0.1f, 0.1f, loco::ANIM_FLAG_RESTART_IF_SAME_ANIM );
        }                    
        else
        // Lip sync?
        if( ( bMovieStart == FALSE ) &&
               (    ( x_stristr( pAnimType, "LIPSYNC"  ) )
                 || ( x_stristr( pAnimType, "LIP_SYNC" ) )
                 || ( x_stristr( pAnimName, "LIPSYNC"  ) )
                 || ( x_stristr( pAnimName, "LIP_SYNC" ) ) ) )
        {
            // Play anim with specified mask type
            pCont = &m_Loco.GetLipSyncController();
            m_Loco.PlayLipSyncAnim( m_iAnim, pAnimName, MaskFlags | loco::ANIM_FLAG_ARTIST_VIEWER | loco::ANIM_FLAG_RESTART_IF_SAME_ANIM );
        }
        // Masked?
        else if ( MaskType != loco::BONE_MASKS_TYPE_FULL_BODY )
        {
            pCont = &m_Loco.GetMaskController();
            m_Loco.PlayMaskedAnim(m_iAnim, MaskType, 0.25f, loco::ANIM_FLAG_RESTART_IF_SAME_ANIM) ;
        }                    
        else
        {
            // Full body
            pCont = &m_Loco.GetMotionController();
            m_Loco.PlayAnim( m_iAnim, 0.25f, loco::ANIM_FLAG_RESTART_IF_SAME_ANIM );
        }
            
        // Start at frame 0?
        if( ( pCont ) && ( bMovieStart ) )
        {
            pCont->SetFrame( 0.0f );   
        }
    }
    else
    {
        // Play non-loco

        // If we have just switched to a new anim, then start playing it
        if( ( bMovieStart ) || ( m_AnimPlayer.GetAnimIndex() != m_iAnim ) )
        {
            m_AnimPlayer.SetAnim( m_iAnim, TRUE );
            m_AnimPlayer.SetLooping( !bMovieStart );
            m_AnimPlayer.SetFrame( 0 ) ;
            m_bAnimPaused = FALSE;
        }
        else
        {
            // Toggle pause
            m_bAnimPaused ^= TRUE;

            // Stop/start the anim
            m_AnimPlayer.SetLooping( !m_bAnimPaused );
        }
    }
}

//==============================================================================

// Returns TRUE if current anim has finished playing
xbool viewer_object::HasCurrentAnimFinished( void )
{
    // Lookup anim info
    const char*           pAnimName;
    const char*           pAnimType;
    loco::bone_masks_type MaskType;
    u32                   MaskFlags;
    GetAnimInfo( pAnimName, pAnimType, MaskType, MaskFlags );

    // Play loco?
    if ( ( m_Type == config_options::TYPE_LOCO ) || ( m_Type == config_options::TYPE_LIP_SYNC ) )
    {
        loco_anim_controller* pCont = NULL;
        
        // Additive?
        if( x_stristr( pAnimType, "ADDITIVE" ) )
        {
            pCont = &m_Loco.GetAdditiveController();
        }                    
        else
        // Lip sync?
        if(     ( g_bMovieShotActive == FALSE )
            &&  (       ( x_stristr( pAnimType, "LIPSYNC"  ) )
                    ||  ( x_stristr( pAnimType, "LIP_SYNC" ) )
                    ||  ( x_stristr( pAnimName, "LIPSYNC"  ) )
                    ||  ( x_stristr( pAnimName, "LIP_SYNC" ) ) ) )
        {
            // Play anim with specified mask type
            pCont = &m_Loco.GetLipSyncController();
        }
        // Masked?
        else if ( MaskType != loco::BONE_MASKS_TYPE_FULL_BODY )
        {
            pCont = &m_Loco.GetMaskController();
        }                    
        else
        {
            pCont = &m_Loco.GetMotionController();
        }
        
        // No anim present?
        if( !pCont )
            return TRUE;
            
        // Start?
        if( pCont->GetFrame() == 0.0f )
            return FALSE ;
            
        // Done?
        return( pCont->IsAtEnd() || pCont->GetWeight() == 0.0f );
    }
    else
    {
        return m_AnimPlayer.IsAtEnd();
    }        
}

//==============================================================================

const char* viewer_object::GetCurrentAnimName( void )
{
    // Lookup group
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    if( !pAnimGroup )
        return "NULL";
        
    // Get anim
    const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( m_iAnim );
    static char Name[256];
    x_strcpy( Name, AnimInfo.GetName() );
    
    // Replace spaces and underscores with ~ so pic join tool works
    char* pChar;
    while( ( pChar = (char*)x_stristr( Name, " " ) ) )
    {
        *pChar = '~';
    }
    while( ( pChar = (char*)x_stristr( Name, "_" ) ) )
    {
        *pChar = '~';
    }
    return Name;
}

//==============================================================================

// Returns current frame
s32 viewer_object::GetCurrentAnimFrame( void )
{
    // Lookup anim info
    const char*           pAnimName;
    const char*           pAnimType;
    loco::bone_masks_type MaskType;
    u32                   MaskFlags;
    GetAnimInfo( pAnimName, pAnimType, MaskType, MaskFlags );

    // Play loco?
    if ( ( m_Type == config_options::TYPE_LOCO ) || ( m_Type == config_options::TYPE_LIP_SYNC ) )
    {
        // Additive?
        if( x_stristr( pAnimType, "ADDITIVE" ) )
        {
            return (s32)m_Loco.GetAdditiveController().GetFrame();
        }                    
        else
            // Lip sync?
            if(     ( g_bMovieShotActive == FALSE )
                &&  (      ( x_stristr( pAnimType, "LIPSYNC"  ) )
                        || ( x_stristr( pAnimType, "LIP_SYNC" ) )
                        || ( x_stristr( pAnimName, "LIPSYNC"  ) )
                        || ( x_stristr( pAnimName, "LIP_SYNC" ) ) ) )
            {
                return (s32)m_Loco.GetLipSyncController().GetFrame();
            }
            // Masked?
            else if ( MaskType != loco::BONE_MASKS_TYPE_FULL_BODY )
            {
                return (s32)m_Loco.GetMaskController().GetFrame();
            }                    
            else
            {
                return (s32)m_Loco.GetMotionController().GetFrame();
            }
    }
    else
    {
        return (s32)m_AnimPlayer.GetFrame();
    }        
}

//==============================================================================

// Writes out .tga of current frame
void viewer_object::DoCurrentAnimFrameScreenShot( void )
{
    // Do shot
    eng_ScreenShot( xfs( "%s\\%s~F%04d.tga",
                        g_Config.m_ScreenShotPath.m_Name, 
                        GetCurrentAnimName(),
                        GetCurrentAnimFrame() ),
                    g_ScreenShotSize );
}

//==============================================================================

// Searches for bone index
s32 viewer_object::FindBoneIndex( const char* pName )
{
    // Lookup anim group
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer() ;
    if (!pAnimGroup)
        return -1 ;

    // Find anywhere in name
    return pAnimGroup->GetBoneIndex(pName, TRUE) ;
}

//==============================================================================

// Returns bone local to world matrix
void viewer_object::GetBoneL2W( s32 iBone, matrix4& L2W )
{
    // Lookup anim group
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer() ;

    // No animation or bone connection?
    if ((iBone == -1) || (pAnimGroup == NULL))
    {
        // Just use L2W
        L2W = m_L2W ;
        return ;
    }

    // Loco?
    if (m_Type == config_options::TYPE_LOCO)
        L2W = m_Loco.m_Player.GetBoneL2W(iBone) ;
    else
        L2W = *m_AnimPlayer.GetBoneL2W(iBone) ;

    // Counter act inverse bind that anim player has put on bone L2W
    ASSERT(pAnimGroup) ;
    L2W.PreTranslate( pAnimGroup->GetBone(iBone).BindTranslation ) ;
}

//==============================================================================

// Returns TRUE if object contains animation a camera bone
xbool viewer_object::HasCameraBone( void )
{
    // Extract view from object? (Player hands)
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    if( pAnimGroup )
    {
        s32 iBoneCam = pAnimGroup->GetBoneIndex( "bone_cam" );
        if( iBoneCam != -1 )
        {
            return TRUE;
        }
    }
    
    // Not found
    return FALSE;
}

//==============================================================================

// Returns TRUE and sets up L2W if object animation contains a camera bone
xbool viewer_object::GetCameraBoneL2W( matrix4& L2W )
{
    // Extract view from object? (Player hands)
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    if( pAnimGroup )
    {
        s32 iBoneCam = pAnimGroup->GetBoneIndex( "bone_cam" );
        if( iBoneCam != -1 )
        {
            // Lookup local space camera offset
            if( m_AnimPlayer.GetAnimGroup() == pAnimGroup )
            {
                const matrix4* pCamL2W = m_AnimPlayer.GetBoneL2W( iBoneCam, FALSE );
                if( pCamL2W )
                {
                    L2W = *pCamL2W;  // World space
                    L2W.PreRotateY( R_180 ); // Camera is 180 degrees off
                    return TRUE;
                }
            }         
        }
    }
        
    // Not found
    L2W.Identity();
    return FALSE;                           
}

//==============================================================================

// Advance logic
void viewer_object::Advance( f32 DeltaTime )
{
    matrix4 CameraBoneL2W;
    
    // No geometry?
    if (!m_pGeom)
        return ;

    // Advance loco and animation players
    switch( m_Type )
    {
    case config_options::TYPE_LOCO:
        m_Loco.OnAdvance( DeltaTime ) ;
        m_Loco.SendEvents( g_pPlayer );
        m_L2W = m_Loco.GetL2W();
        break;

    case config_options::TYPE_LIP_SYNC:
        m_Loco.OnAdvance( DeltaTime ) ;
        m_Loco.SendEvents( g_pPlayer );

        // NOTE: Lip sync loco is left at the origin because loco has no pitch -
        // Compute matrices puts it into world space
        break;

    default:            
        // Advance animation player?
        if( m_bAnimPaused == FALSE )
        {
            m_AnimPlayer.Advance(DeltaTime) ;
            if( m_AnimPlayer.GetAnimIndex() != -1 )
                g_EventMgr.HandleSuperEvents( m_AnimPlayer, g_pPlayer );
        }

        // Extract view from object? (Player hands)
        if( GetCameraBoneL2W( CameraBoneL2W ) )
            m_View.SetV2W( CameraBoneL2W );
        break;        
    }

    // Attached to parent?
    if ((&g_pObjects[g_iObject] != this) && (m_pParentObject))
    {
        // Use parent L2W
        matrix4 L2W;
        m_pParentObject->GetBoneL2W( m_iParentBone, L2W ) ;

        // Also setup animation to match parent if it's present
        if( m_hAnimGroup.GetPointer() && m_pParentObject->m_hAnimGroup.GetPointer() )
        {
            simple_anim_player& Player = m_pParentObject->m_AnimPlayer;      
            s32 iAnim = m_AnimPlayer.GetAnimIndex( Player.GetAnimName() );
            if( iAnim != -1 )
            {
                m_AnimPlayer.SetAnim ( Player.GetAnimName(), Player.IsLooping() );
                m_AnimPlayer.SetFrame( Player.GetFrame() );
            }
        }

        // Update
        SetL2W( L2W );
    }

    // Advance children
    for (s32 i = 0 ; i < m_pChildren.GetCount() ; i++)
        m_pChildren[i]->Advance(DeltaTime) ;
}

//==============================================================================

// Sets L2W for loco/anim players
void viewer_object::SetL2W( const matrix4& L2W, xbool bFlip180 )
{
    // Nothing to do?
    if ( x_memcmp(&L2W, &m_L2W, sizeof(L2W)) == 0 )
        return;

    // Keep new L2W
    m_L2W = L2W;
    
    // Update anim player
    m_AnimPlayer.SetL2W( L2W );
    
    // Update loco?
    if( m_Type == config_options::TYPE_LOCO )
    {
        // Fixup
        matrix4 LocoL2W = L2W;
        if( bFlip180 )
            LocoL2W.PreRotateY( R_180 );
        m_Loco.SetL2W( LocoL2W );
    }
                   
    // NOTE: Lip sync loco is left at the origin because loco has no pitch -
    // Compute matrices puts it into world space
                   
    // Update lighting
    Light() ;
}

//==============================================================================

// Computes matrices from animation (if present)
const matrix4* viewer_object::ComputeMatrices( s32 nActiveBones )
{
    s32 i;
    
    // No geometry?
    if (!m_pGeom)
        return NULL ;

    // Allocate matrices
    matrix4* pMatrices = (matrix4*)smem_BufferAlloc(m_pGeom->m_nBones * sizeof(matrix4));
    if (!pMatrices)
        return NULL ;

    // Start with own L2W
    matrix4 L2W = m_L2W ;
    for(i = 0; i < m_pGeom->m_nBones ; i++)
        pMatrices[i] = L2W ;

    // Animation present?
    if (m_hAnimGroup.GetPointer())
    {
        // Loco?
        if ( ( m_Type == config_options::TYPE_LOCO ) || ( m_Type == config_options::TYPE_LIP_SYNC ) )
        {
            // Animations present?
            if (m_Loco.IsAnimLoaded())
            {
                // Grab matrices
                m_Loco.m_Player.SetNActiveBones(nActiveBones) ;   
                x_memcpy( pMatrices, m_Loco.ComputeL2W(), sizeof(matrix4) * nActiveBones );
            }

            // Lip sync loco?            
            if( m_Type == config_options::TYPE_LIP_SYNC )
            {
                // For lip sync, loco is at origin, so put into world space
                for( i = 0 ; i < nActiveBones ; i++ )
                    pMatrices[i] = L2W * pMatrices[i];
            }
        }
        else
        {
            // Normal player
            x_memcpy( pMatrices, m_AnimPlayer.GetBoneL2Ws(), sizeof(matrix4) * nActiveBones );
        }
    }
    
    return pMatrices ;
}

//==============================================================================

// Render
void viewer_object::Render( const view& View, xtimer& LogicCPU, xtimer& RenderCPU )
{
    s32 i ;

    // No goemetry?
    if (!m_pGeom)
        return ;

    LogicCPU.Start() ;

    // Get world bbox
    bbox WorldBBox = GetWorldBBox() ;

    // Show bounds
    //draw_ClearL2W();
    //draw_BBox( WorldBBox, XCOLOR_RED );

    // Compute render flags
    u32 Flags = 0;
    
    // Offscreen?
    s32 InView = View.BBoxInView(WorldBBox) ;

    // Clip?
    if (InView == view::VISIBLE_PARTIAL)
        Flags |= render::CLIPPED ;

    LogicCPU.Stop() ;
    
    // Render geometry?
    if (InView != view::VISIBLE_NONE)
    {
        LogicCPU.Start() ;

        // Compute LOD to use
        f32 ScreenSize = View.CalcScreenSize( m_L2W.GetTranslation(), m_pGeom->m_BBox.GetRadius() ) ;
        m_iLOD = 0;
        for( i = 0 ; i < m_LODs.GetCount() ; i++ )
        {
            if( ScreenSize < m_LODs[i].m_ScreenSize )
                m_iLOD = i;
        }

        // Lookup mask
        ASSERT(m_iLOD >= 0) ;
        ASSERT(m_iLOD < m_LODs.GetCount() );
        u64 Mask = m_LODs[m_iLOD].m_Mask;

        // Count bones used by geometry and tell the animation player
        // (bones in sorted into heirarchical order so we can just keep the max)
        m_nActiveBones = 0 ;
        m_nVerts       = 0 ;
        m_nTris        = 0 ;
        for (i = 0 ; i < m_pGeom->m_nMeshes ; i++)
        {
            // Is this mesh being used?
            if (Mask & (1<<i))
            {
                // Lookup mesh
                const geom::mesh& Mesh = m_pGeom->m_pMesh[i] ;

                // Update max count
                m_nActiveBones = MAX(m_nActiveBones, Mesh.nBones) ;

                // Update vert and tri count
                m_nVerts += Mesh.nVertices ;
                m_nTris  += Mesh.nFaces ;
            }
        }

        // Compute # of materials used
        xarray<s32> iMats ;
        for (i = 0 ; i < m_pGeom->m_nMeshes ; i++)
        {
            // Mesh not displayed?
            if (!(Mask & (1<<i)))
                continue ;

            // Loop through all sub meshes
            geom::mesh& Mesh = m_pGeom->m_pMesh[i] ;
            for (s32 j = 0 ; j < Mesh.nSubMeshes ; j++)
            {
                // Lookup material on submesh
                s32 iMat = m_pGeom->m_pSubMesh[Mesh.iSubMesh + j].iMaterial ;

                // If not there, add it
                if (iMats.Find(iMat) == -1)
                {
                    // Create new material
                    s32& NewMat = iMats.Append();
                    
                    // Setup
                    NewMat = iMat ;
                }
            }
        }
        m_iMaterials = iMats.GetCount() ;

        // Compute matrices
        const matrix4* pMatrices = ComputeMatrices(m_nActiveBones) ;
        ASSERT(pMatrices) ;

        LogicCPU.Stop() ;

        RenderCPU.Start() ;

        // Render skin?
        if (m_RenderType == RENDER_SKIN)
        {
            // Compute ambient from light
            xcolor Ambient = ComputeAmbient( WorldBBox.GetCenter() );

            // Add a skinned instance
            render::AddSkinInstance( m_hInst,
                                     pMatrices,
                                     Mask,
                                     0,
                                     Flags,
                                     Ambient ) ;
        }
        else
        {
            // Add a rigid instance
            render::AddRigidInstance( m_hInst,
                                      m_pColors,
                                      pMatrices,
                                      Mask,
                                      Flags,
                                      255 );
        }

        RenderCPU.Stop() ;
    }

    // Render bbox
    //draw_SetL2W(m_L2W) ;
    //draw_BBox(GetLocalBBox(), XCOLOR_GREEN) ;
    //draw_ClearL2W() ;

    // Render object?
    if( m_Type == config_options::TYPE_OBJECT )
    {
        // Render grid so we can see the floor when showing player hands
        if (    ( HasCameraBone() )
                && ( !g_ShowHelp ) 
                && ( ( g_StatsMode == STATS_MODE_HIGH ) || ( g_StatsMode == STATS_MODE_MEDIUM ) ) )
        {
            // Render grid
            draw_ClearL2W();
            f32 S = 100*5*5*2 ;
            f32 Y = -155.0f; // Offset from arms (got by debugging "player::OnMoveViewPosition")
            xcolor C = g_Config.m_GridColor;
            draw_Grid( vector3(-S,  Y, -S), 
                    vector3( S*2,  Y,  0), 
                    vector3( 0,  Y,  S*2), 
                    C, 
                    5*5*2 );
                    
            // Draw origin axis
            C.R >>= 2 ;                
            C.G >>= 2 ;                
            C.B >>= 2 ;                
            draw_Line( vector3( -S, 0, 0  ), vector3( S, 0, 0 ), C );
            draw_Line( vector3(  0, 0, -S ), vector3( 0, 0, S ), C );
        }
    }
                
    // Render loco?
    if ( m_Type == config_options::TYPE_LOCO )
    {
        // Render move and look info
        if (    (!g_ShowHelp) &&
                ( (g_StatsMode == STATS_MODE_HIGH) || (g_StatsMode == STATS_MODE_MEDIUM) ) )
        {
            // Render grid
            draw_ClearL2W();
            f32 S = 100*5*6 ;
            xcolor C = g_Config.m_GridColor;
            draw_Grid( vector3(-S,  0, -S), 
                       vector3( S*2,  0,  0), 
                       vector3( 0,  0,  S*2), 
                       C, 5*6 );
            
            // Draw origin axis
            C.R >>= 2 ;                
            C.G >>= 2 ;                
            C.B >>= 2 ;                
            draw_Line( vector3( -S, 0, 0  ), vector3( S, 0, 0 ), C );
            draw_Line( vector3(  0, 0, -S ), vector3( 0, 0, S ), C );
            
            // Draw move and look markers?
            if (m_Loco.HasMoveAnims())
            {
                m_Loco.Render();
            }                
        }
    }

    // Render children
    for (i = 0 ; i < m_pChildren.GetCount() ; i++)
        m_pChildren[i]->Render(View, LogicCPU, RenderCPU) ;
}

//==============================================================================

// Returns local bbox
const bbox& viewer_object::GetLocalBBox( void )
{
    // Clear
    static bbox BBox ;
    BBox.Clear();

    // Use bbox from animation?
    anim_group* pAnimGroup = m_hAnimGroup.GetPointer() ;
    if (pAnimGroup)
        BBox += pAnimGroup->GetBBox() ;

    // Use geom?
    if (m_pGeom)
        BBox += m_pGeom->m_BBox ;

    // No geometry?
    if( (!pAnimGroup) && (!m_pGeom) )
        BBox.Set(vector3(0,0,0), 100) ;

    return BBox ;
}

//==============================================================================

// Returns bbox of bind pose
bbox viewer_object::GetBindWorldBBox( void )
{
    bbox BBox;
    
    // Try bind pose animation first
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    if( pAnimGroup )
    {
        // Return bbox of bind pose
        const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( 0 );
        BBox = AnimInfo.GetBBox();
    }
    // Try geometry
    else if( m_pGeom )
    {
        BBox = m_pGeom->m_BBox;
    }                
    else
    {
        // Use default
        BBox.Set( vector3( 0.0f, 0.0f, 0.0f ), 100.0f );        
    }
            
    // Translate into world space   
    BBox.Transform( GetL2W() );
    return BBox;
}

//==============================================================================

// Returns local to world matrix
matrix4 viewer_object::GetL2W( void )
{
    // Get L2W
    matrix4 L2W = m_L2W;
    if ((&g_pObjects[g_iObject] != this) && (m_pParentObject))
        m_pParentObject->GetBoneL2W(m_iParentBone, L2W) ;

    return L2W;
}

//==============================================================================

// Returns world bbox
bbox viewer_object::GetWorldBBox( void )
{
    // Compute world bbox
    bbox BBox = GetLocalBBox() ;
    BBox.Transform( GetL2W() );

    return BBox ;
}

//==============================================================================

// Light vert using config omni light
xcolor viewer_object::LightVert( const vector3& Position, const vector3& Normal )
{
    // Full bright?
    if (g_Light.m_State == config_options::light::STATE_FULL_BRIGHT)
        return XCOLOR_WHITE ;
    
    // Off?
    if (g_Light.m_State == config_options::light::STATE_OFF)
        return XCOLOR_BLACK ;

    // Get direction and distance to light
    vector3 Dir  = g_Light.m_Position - (m_L2W * Position) ;
    f32     Dist = Dir.Length() ;

    // Compute directional intensity
    Dir /= Dist ;
    f32 I = x_max(0.0f, Dir.Dot(m_L2W.RotateVector(Normal)) * g_Light.m_Intensity) ;

    // Compute attenuation
    f32 Atten = 1.0f - x_min(1.0f, Dist / g_Light.m_Radius) ;
    
    // Compute final color
    f32 R = x_min(255.0f, Atten * (g_Light.m_Ambient.R + (g_Light.m_Color.R * I))) ;
    f32 G = x_min(255.0f, Atten * (g_Light.m_Ambient.G + (g_Light.m_Color.G * I))) ;
    f32 B = x_min(255.0f, Atten * (g_Light.m_Ambient.B + (g_Light.m_Color.B * I))) ;
    f32 A = x_min(255.0f, Atten * (g_Light.m_Ambient.A + (g_Light.m_Color.A * I))) ;

    // Extract as color
    xcolor Color ;
    Color.R = (u8)R ;
    Color.G = (u8)G ;
    Color.B = (u8)B ;
    Color.A = (u8)A ;
    return Color ;
}

//==============================================================================

// Lights the object using the config file lighting
void viewer_object:: Light( void )
{
    // Only need to light rigid instances
    if (m_RenderType != RENDER_RIGID)
        return ;

    // Must have geometry!
    if (!m_pGeom)
        return ;

#ifdef TARGET_PC
    // Get geom
    rigid_geom* pGeom = (rigid_geom*)m_pGeom ;
    ASSERT(pGeom) ;

    // Loop through all meshes
    for (s32 i = 0 ; i < pGeom->m_nSubMeshes ; i++)
    {
        // Lookup display list for sub-mesh
        rigid_geom::dlist_pc& DList = pGeom->m_System.pPC[pGeom->m_pSubMesh[i].iDList] ;

        // Lock verts for submesh
        rigid_geom::vertex_pc* pVerts = (rigid_geom::vertex_pc* )render::LockRigidDListVertex(m_hInst, i) ;
        ASSERT(pVerts) ;

        // Loop through verts
        for (s32 j = 0 ; j < DList.nVerts ; j++)
        {
            // Lookup vert
            rigid_geom::vertex_pc& Vert = pVerts[j] ;

            // Light
            xcolor Color = LightVert(Vert.Pos, Vert.Normal) ;

            // Store in hardware vert format
            Vert.Color.R = Color.R>>1 ;
            Vert.Color.G = Color.G>>1 ;
            Vert.Color.B = Color.B>>1 ;
            Vert.Color.A = Color.A ;
        }

        // Unlock verts
        render::UnlockRigidDListVertex(m_hInst, i) ;
    }
#endif

#ifdef TARGET_PS2
    // Get geom
    rigid_geom* pGeom = (rigid_geom*)m_pGeom ;
    ASSERT(pGeom) ;

    // Loop through all display lists
    for (s32 i = 0 ; i < pGeom->m_nDList ; i++)
    {
        // Lookup display list
        rigid_geom::dlist_ps2& DList = pGeom->m_System.pPS2[i] ;

        // Lookup position and normal streams
        vector4* pPositions = DList.pPosition ;
        s8*      pNormals   = DList.pNormal ;

        // Loop through verts
        for (s32 j = 0 ; j < DList.nVerts ; j++)
        {
            // Compute position
            vector3 Pos(pPositions[j].GetX(), pPositions[j].GetY(), pPositions[j].GetZ()) ;

            // Compute normal
            vector3 Normal(pNormals[(j*3)+0], pNormals[(j*3)+1], pNormals[(j*3)+2]) ;
            Normal *= 1.0f / 127.0f ;

            // Light
            xcolor Color = LightVert(Pos, Normal) ;

            // Grab top 5 bits of each color
            u8 R = (Color.R>>3) & 0x1F ;
            u8 G = (Color.G>>3) & 0x1F ;
            u8 B = (Color.B>>3) & 0x1F ;
            u8 A = (Color.A>>7) & 0x01 ;

            // 128 is 1.0 on PS2
            R >>= 1;
            G >>= 1;
            B >>= 1;

            // Store in color table
            m_pColors[DList.iColor+j] = (A<<15) | (B<<10) | (G<<5) | R ;
        }
    }
#endif
}

//==============================================================================

// Process input
void viewer_object::HandleInput( f32 DeltaTime )
{
    // Setup update flags
    xbool bHasCameraBone = HasCameraBone();
    xbool bMoveLoco     = FALSE ;
    xbool bMoveCamera   = FALSE ;
    xbool bRotateCamera = FALSE ;
    xbool bMoveObject   = FALSE ;
    xbool bRotateObject = FALSE ;
    xbool bLipSyncTest  = FALSE ;
    xbool bRagdollShiftPressed = FALSE;

    // Which type?
    if (m_Type == config_options::TYPE_LOCO)
    {
        // Lookup ragdoll shift mode
        bRagdollShiftPressed = input_IsPressed( JOY_OBJECT_RAGDOLL_SHIFT );
    
        // Ragdoll mode?
        if( bRagdollShiftPressed )
        {
            // Blast a ragdoll?
            if( input_WasPressed( JOY_OBJECT_BLAST_RAGDOLL ) )
            {
                AddPhysicsInsts( m_hSkinGeom.GetName(),     // pGeomName
                                 m_LODs[ m_iLOD ].m_Mask,   // Mask
                                 m_Loco.m_Player,           // AnimPlayer
                                 m_Loco.GetDeltaPos(),      // Vel
                                 FALSE,                     // bClearAll
                                 TRUE );                    // bBlast
            }

            // Drop a ragdoll?
            if( input_WasPressed( JOY_OBJECT_DROP_RAGDOLL ) )
            {
                AddPhysicsInsts( m_hSkinGeom.GetName(),     // pGeomName
                                 m_LODs[ m_iLOD ].m_Mask,   // Mask
                                 m_Loco.m_Player,           // AnimPlayer
                                 m_Loco.GetDeltaPos(),      // Vel
                                 TRUE,                      // bClearAll
                                 FALSE );                   // bBlast
            }
        }    
    
        // If shift is pressed, move the camera
        if ( input_IsPressed(JOY_OBJECT_SHIFT) )
        {
            bMoveLoco     = FALSE ;
            bMoveCamera   = TRUE ;
            bRotateCamera = TRUE ;
            bMoveObject   = FALSE ;
            bRotateObject = FALSE ;
        }
        else
        {
            // Move the loco "move at" and "look at"
            bMoveLoco     = TRUE ;
            bMoveCamera   = FALSE ;
            bRotateCamera = FALSE ;
            bMoveObject   = FALSE ;
            bRotateObject = FALSE ;
        }
    }
// Commented out as per Aarons request so lip sync uses normal object camera    
/*    
    else if (m_Type == config_options::TYPE_LIP_SYNC)
    {
        bLipSyncTest  = TRUE ;

        // If shift is pressed, move the camera
        if ( input_IsPressed(JOY_OBJECT_SHIFT) )
        {
            bMoveLoco     = FALSE ;
            bMoveCamera   = TRUE ;
            bRotateCamera = TRUE ;
            bMoveObject   = FALSE ;
            bRotateObject = FALSE ;
        }
        else
        {
            // Move the loco "move at" and "look at"
            bMoveLoco     = TRUE ;
            bMoveCamera   = FALSE ;
            bRotateCamera = FALSE ;
            bMoveObject   = FALSE ;
            bRotateObject = FALSE ;
        }
    }
    */        
    else
    {
        // If shift is pressed, move the object
        if ( (input_IsPressed(JOY_OBJECT_SHIFT)) || ( bHasCameraBone ) )
        {
            bMoveLoco     = FALSE ;
            bMoveCamera   = FALSE ;
            bRotateCamera = FALSE ;
            bMoveObject   = TRUE ;
            bRotateObject = TRUE ;
        }
        else
        {
            // Move the camera
            bMoveLoco     = FALSE ;
            bMoveCamera   = TRUE ;
            bRotateCamera = TRUE ;
            bMoveObject   = FALSE ;
            bRotateObject = FALSE ;
        }
    }

    // Record current L2W so we can check if object moved for re-lighting
    matrix4 L2W = m_L2W ;

    // Scale movements by size of object
    f32 Size = GetBindWorldBBox().GetRadius() / 400.0f ;

    // Update loco?
    if (bMoveLoco)
    {

#ifdef TARGET_PC
        // Set the move at?
        if (input_IsPressed(KEY_OBJECT_SET_MOVE_AT))
        {
            vector3 RayDir = m_View.RayFromScreen( d3deng_GetABSMouseX(), d3deng_GetABSMouseY() ) * 100000.0f;
            vector3 RayPos = m_View.GetPosition();
            plane   Plane( vector3(0,1,0), 0 );
            f32     t;
            if( Plane.Intersect( t, RayPos, RayDir ) )
            {
                vector3 ColPos = RayPos + RayDir*t;
                ColPos.GetY() = 0 ;
                m_Loco.SetMoveAt( ColPos );
                m_Loco.SetExactMove( TRUE );
            }
        }

        // Set look at?
        if (input_IsPressed(KEY_OBJECT_SET_LOOK_AT))
        {
            vector3 RayDir = m_View.RayFromScreen( d3deng_GetABSMouseX(), d3deng_GetABSMouseY() ) * 100000.0f;
            vector3 RayPos = m_View.GetPosition();
            plane   Plane( vector3(0,1,0), 0 );
            f32     t;
            if( Plane.Intersect( t, RayPos, RayDir ) )
            {
                //vector3 ColPos = RayPos + RayDir*t;
                //ColPos.GetY() = m_Loco.GetLookAt().GetY() ;
                //m_Loco.SetLookAt(ColPos) ;
            }
        }
#endif

        // Get move and look positions
        vector3 MoveAt = m_Loco.GetMoveAt() ;
        vector3 LookAt = m_Loco.GetHeadLookAt() ;

        // Compute local->world matrix to counter-act view rotation
        matrix4 L2W ;
        L2W.Identity() ;
        L2W.RotateY(m_View.GetV2W().GetRotation().Yaw) ;

        // Joy controller input
        f32 S  = Size * 1000.0f * DeltaTime ;
        if( input_IsPressed( JOY_OBJECT_SPEEDUP   ) )  S *= 2.0f;
        
        // Update move at
#ifdef TARGET_PC
        f32 L  = S * input_GetValue( JOY_OBJECT_MOVE_Z );
        f32 V = S * input_GetValue( JOY_OBJECT_MOVE_X );
#else
        f32 L  = S * input_GetValue( JOY_OBJECT_MOVE_Z );
        f32 V = S * input_GetValue( JOY_OBJECT_MOVE_X );
#endif
        MoveAt += L2W.RotateVector( vector3(0,0,L) ) ;
        MoveAt += L2W.RotateVector( vector3(-V,0,0) ) ;

        // Update look at
        if( input_IsPressed( JOY_OBJECT_LOOK_UP   ) )  LookAt += vector3( 0, S*0.5f, 0) ;
        if( input_IsPressed( JOY_OBJECT_LOOK_DOWN ) )  LookAt += vector3( 0,-S*0.5f, 0) ;
#ifdef TARGET_PC
        L  = -S * input_GetValue( JOY_OBJECT_LOOK_X );
        V = -S * input_GetValue( JOY_OBJECT_LOOK_Z );
#else
        L  = S * input_GetValue( JOY_OBJECT_LOOK_Z );
        V = S * input_GetValue( JOY_OBJECT_LOOK_X );
#endif
        LookAt += L2W.RotateVector( vector3(0,0,L) ) ;
        LookAt += L2W.RotateVector( vector3(-V,0,0) ) ;

        // Set info
        m_Loco.SetMoveAt(MoveAt) ;
        m_Loco.SetLookAt(LookAt) ;
        m_Loco.SetExactMove( TRUE );

        // Loco changed?
        if( bRagdollShiftPressed  == FALSE )
        {
            // Toggle move style?
            if ( (input_WasPressed(JOY_OBJECT_MOVE_STYLE)) ||
                (input_WasPressed(KEY_OBJECT_MOVE_STYLE)) )
            {
                // Goto next valid move style
                s32 StartMoveStyle = m_Loco.GetMoveStyle() ;
                s32 MoveStyle      = m_Loco.GetMoveStyle() ;
                do
                {
                    if (++MoveStyle >= loco::MOVE_STYLE_COUNT)
                        MoveStyle = 0 ;
                }
                while(      (MoveStyle != StartMoveStyle) &&
                            (m_Loco.IsValidMoveStyle((loco::move_style)MoveStyle) == FALSE) ) ;

                // Allow/disable aim styles so that we can toggle through them!
                if (        (MoveStyle == loco::MOVE_STYLE_RUNAIM)
                        ||  (MoveStyle == loco::MOVE_STYLE_CROUCHAIM) )
                {
                    m_Loco.SetUseAimMoveStyles(TRUE) ;
                }
                else
                {
                    m_Loco.SetUseAimMoveStyles(FALSE) ;
                }

                // Set it
                m_Loco.SetMoveStyle((loco::move_style)MoveStyle) ;
            }
        }
    }
    
    // Rotate object?
    if (bRotateObject)
    {
        // Rotation
        f32 S  = DeltaTime ;

        // Speedup?
        if (input_IsPressed(JOY_OBJECT_SPEEDUP))
            S *= 2.0f ;

    #ifdef TARGET_PC
        f32 Pitch = -input_GetValue( JOY_OBJECT_YAW )   * S * R_180 ;
        f32 Yaw   = -input_GetValue( JOY_OBJECT_PITCH ) * S * R_180 ;
    #else
        f32 Pitch = input_GetValue( JOY_OBJECT_PITCH ) * S * R_180 ;
        f32 Yaw   = input_GetValue( JOY_OBJECT_YAW )   * S * R_180 ;
    #endif

        // First person player?
        if( bHasCameraBone )
        {
            // Operate on pitch and yaw separately like player control
            vector3 Pos = L2W.GetTranslation();
            radian3 Rot = L2W.GetRotation();
            Rot.Pitch -= Pitch * 0.5f;
            if( Rot.Pitch < -R_89 )
                Rot.Pitch = -R_89;
            else if( Rot.Pitch > R_89 )
                Rot.Pitch = R_89;
            Rot.Yaw   -= Yaw * 0.6f;
            L2W.SetRotation( Rot );
            L2W.SetTranslation( Pos );
        }
        // Rotate?
        else if ((Pitch != 0) || (Yaw != 0))
        {
            // Rotate around center of bbox
            vector3 Pos = GetBindWorldBBox().GetCenter() ;
            L2W.Translate(-Pos) ;
            L2W.RotateX(Pitch) ;
            L2W.RotateY(Yaw) ;
            L2W.Translate(Pos) ;
            L2W.Orthogonalize() ;
        }
    }

    // Rotate camera?
    if (bRotateCamera)
    {
        radian Pitch, Yaw ;
        f32    S,R,T ;

        // Joy controller input
        S  = DeltaTime ;

        if (!m_Loco.HasMoveAnims())                    S *= 0.33f ;
        if ( input_IsPressed( JOY_OBJECT_SPEEDUP  ) )  S *= 2.0f;


        m_View.GetPitchYaw( Pitch, Yaw );
        R = S * R_90 ;
    #ifdef TARGET_PC
        Pitch -= input_GetValue( JOY_OBJECT_YAW ) * R ;
        Yaw   += input_GetValue( JOY_OBJECT_PITCH ) * R ;
    #else
        Pitch += input_GetValue( JOY_OBJECT_PITCH ) * R ;
        Yaw   -= input_GetValue( JOY_OBJECT_YAW ) * R ;
    #endif
        m_View.SetRotation( radian3(Pitch,Yaw,0) );

        // Mouse input
        if ( input_IsPressed( MSE_OBJECT_SPEEDUP  ) )  S *= 2.0f;

        T = S * 200.0f ;
        if ( input_IsPressed( MSE_OBJECT_MOVE ) )
        {
        }
        else
        {
            R = S * R_45 ;
            m_View.GetPitchYaw( Pitch, Yaw );       
            Pitch += input_GetValue( MSE_OBJECT_PITCH ) * R ;
            Yaw   -= input_GetValue( MSE_OBJECT_YAW  ) * R ;
            m_View.SetRotation( radian3(Pitch,Yaw,0) );
        }
    }

    // Move camera?
    if (bMoveCamera)
    {
        f32    T ;

        // Joy controller input
        f32 S = Size * DeltaTime ;

        if (!m_Loco.HasMoveAnims())                    S *= 0.33f ;
        if( input_IsPressed( JOY_OBJECT_SPEEDUP   ) )  S *= 2.0f;

        T = S*200 ;
        if( input_IsPressed( JOY_OBJECT_MOVE_UP   ) )  m_View.Translate( vector3( 0, T,0 ), view::VIEW );
        if( input_IsPressed( JOY_OBJECT_MOVE_DOWN ) )  m_View.Translate( vector3( 0,-T,0 ), view::VIEW );

        T = S*500 ;
    #ifdef TARGET_PC    
        m_View.Translate( vector3(0, 0, T * -input_GetValue( JOY_OBJECT_MOVE_Z ) ), view::VIEW );
        m_View.Translate( vector3(T * input_GetValue( JOY_OBJECT_MOVE_X ),0,0), view::VIEW );
    #else
        m_View.Translate( vector3(0, 0, T * input_GetValue( JOY_OBJECT_MOVE_Z ) ), view::VIEW );
        m_View.Translate( vector3(T * -input_GetValue( JOY_OBJECT_MOVE_X ), 0, 0 ), view::VIEW );
    #endif

        // Keyboard input
        T = S * 350 ;
        if( input_IsPressed( KEY_OBJECT_FORWARD ) )  m_View.Translate( vector3( 0, 0, T), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_BACK    ) )  m_View.Translate( vector3( 0, 0,-T), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_LEFT    ) )  m_View.Translate( vector3( T, 0, 0), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_RIGHT   ) )  m_View.Translate( vector3(-T, 0, 0), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_UP      ) )  m_View.Translate( vector3( 0, T, 0), view::VIEW );
        if( input_IsPressed( KEY_OBJECT_DOWN    ) )  m_View.Translate( vector3( 0,-T, 0), view::VIEW );

        // Mouse input
        if ( input_IsPressed( MSE_OBJECT_SPEEDUP  ) )  S *= 2.0f;

        T = S * 200.0f ;
        if ( input_IsPressed( MSE_OBJECT_MOVE ) )
        {
            m_View.Translate( vector3( input_GetValue(MSE_OBJECT_MOVE_HORIZ)   * T, 0, 0), view::VIEW );
            m_View.Translate( vector3( 0, input_GetValue(MSE_OBJECT_MOVE_VERT) * T, 0), view::VIEW );
        }
        m_View.Translate( vector3( 0, 0, input_GetValue(MSE_OBJECT_ZOOM) * S * 10000.0f), view::VIEW );
    }

    // Move object?
    if (bMoveObject)
    {
        f32    T ;

        // Make movement align to view
        matrix4 ViewAlign ;
        ViewAlign.Identity() ;
        ViewAlign = m_View.GetV2W() ;
        ViewAlign.SetTranslation(vector3(0,0,0)) ;

        // Joy controller input
        f32 S = Size * DeltaTime ;
        if( input_IsPressed( JOY_OBJECT_SPEEDUP   ) )  S *= 2.0f;

        // Player arms?
        if( bHasCameraBone )
        {
            // Only consider yaw when moving with view so arms stay on the ground
            radian3 Rot = ViewAlign.GetRotation();
            Rot.Pitch = R_0;
            Rot.Roll = R_0;
            ViewAlign.SetRotation( Rot );
            
            // Speed up movement as if player
            S *= 12.0f;
        }
        
        T = S*400 ;
        
        // Only move vertically if not player arms
        if( !bHasCameraBone )
        {
            if( input_IsPressed( JOY_OBJECT_MOVE_UP   ) )  L2W.Translate( ViewAlign * vector3( 0, T,0 )) ;
            if( input_IsPressed( JOY_OBJECT_MOVE_DOWN ) )  L2W.Translate( ViewAlign * vector3( 0,-T,0 )) ;
        }
        
        T = S*500 ;
    #ifdef TARGET_PC    
        L2W.Translate(ViewAlign * vector3(0, 0, T * -input_GetValue( JOY_OBJECT_MOVE_Z ) )) ;
        L2W.Translate(ViewAlign * vector3(T * input_GetValue( JOY_OBJECT_MOVE_X ),0,0) ) ;
    #else
        L2W.Translate(ViewAlign * vector3(0, 0, T * input_GetValue( JOY_OBJECT_MOVE_Z ) )) ;
        L2W.Translate(ViewAlign * vector3(T * -input_GetValue( JOY_OBJECT_MOVE_X ), 0, 0 )) ;
    #endif

        // Keyboard input
        T = S * 350 ;
        if( input_IsPressed( KEY_OBJECT_FORWARD ) )  L2W.Translate(ViewAlign * vector3( 0, 0, T));
        if( input_IsPressed( KEY_OBJECT_BACK    ) )  L2W.Translate(ViewAlign * vector3( 0, 0,-T));
        if( input_IsPressed( KEY_OBJECT_LEFT    ) )  L2W.Translate(ViewAlign * vector3( T, 0, 0));
        if( input_IsPressed( KEY_OBJECT_RIGHT   ) )  L2W.Translate(ViewAlign * vector3(-T, 0, 0));
        if( input_IsPressed( KEY_OBJECT_UP      ) )  L2W.Translate(ViewAlign * vector3( 0, T, 0));
        if( input_IsPressed( KEY_OBJECT_DOWN    ) )  L2W.Translate(ViewAlign * vector3( 0,-T, 0));

        // Mouse input
        if ( input_IsPressed( MSE_OBJECT_SPEEDUP  ) )  S *= 2.0f;

        T = S * 200.0f ;
        if ( input_IsPressed( MSE_OBJECT_MOVE ) )
        {
            L2W.Translate(ViewAlign * vector3( input_GetValue(MSE_OBJECT_MOVE_HORIZ)   * T, 0, 0 )) ;
            L2W.Translate(ViewAlign * vector3( 0, input_GetValue(MSE_OBJECT_MOVE_VERT) * T, 0 )) ;
        }
        L2W.Translate(ViewAlign * vector3( 0, 0, input_GetValue(MSE_OBJECT_ZOOM) * S * 10000.0f )) ;
    }

    // Force look at and camera?
    if (bLipSyncTest)
    {
// Disable camera tracking the eye as per Aarons request!
/*

        // Get position behind the camera
        vector3 LookAt = m_View.ConvertV2W(vector3(0,0,-100*5)) ;
        LookAt.Y = m_View.GetPosition().Y ;
        m_Loco.SetLookAt(LookAt) ;
        m_Loco.SetEyesLookAt(LookAt) ;

        // Point the camera at the eyes
        m_View.LookAtPoint(m_Loco.GetEyePosition()) ;
        m_Loco.SetBlendSpeedAim( LOCO_BLEND_SPEED_AIM ) ;
*/
    }

    // Reset?
    if (input_WasPressed(JOY_OBJECT_RESET))
    {
        // Reset position and rotation
        L2W.Identity();
        SetL2W( L2W, TRUE );
        
        // Reset loco
        m_Loco.GetAimController().SetBlendFactor( 0.0f, 0.0f, 0.0f );
        m_Loco.SetLookAt(m_Loco.GetEyePosition() + vector3(0,0,-100)) ;
        m_Loco.SetMoveAt(m_Loco.GetPosition()) ;
        m_Loco.SetExactMove( TRUE );
        m_Loco.SetState( loco::STATE_IDLE );
        
        // Advance so the loco settles
        m_Loco.OnAdvance( 1.0f );
        m_Loco.OnAdvance( 1.0f );
        m_Loco.OnAdvance( 1.0f );
        m_Loco.OnAdvance( 1.0f );
       
        // Reposition view
        ResetView() ;
    }

    // Update L2W
    SetL2W( L2W );

    // Handle anim input?    
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer() ;
    if( ( pAnimGroup ) && ( bRagdollShiftPressed == FALSE ) )
    {
        // Previous anim?
        if (input_WasPressed(JOY_OBJECT_PREV_ANIM))
        {
            // Goto previous anim
            if (--m_iAnim < 0)
                m_iAnim = m_iLastAnim ;
        }

        // Next anim?
        if (input_WasPressed(JOY_OBJECT_NEXT_ANIM))
        {
            // Goto next anim
            if (++m_iAnim > m_iLastAnim)
                m_iAnim = 0 ;
        }

        // Play anim?
        if (input_WasPressed(JOY_OBJECT_PLAY_ANIM))
            PlayCurrentAnim( FALSE );

        // Keep looping the anim in loco mode?
        if ( m_Type == config_options::TYPE_LOCO )
        {
            xbool bPlayAnimPressed = input_IsPressed(JOY_OBJECT_PLAY_ANIM) ;

            // Lookup anim info
            const char*           pAnimName;
            const char*           pAnimType;
            loco::bone_masks_type MaskType;
            u32                   MaskFlags;
            GetAnimInfo( pAnimName, pAnimType, MaskType, MaskFlags );

            // Additive?
            if (x_stristr(pAnimType, "ADDITIVE"))
                m_Loco.GetAdditiveController().SetLooping( bPlayAnimPressed ) ;
            else
            // Masked expression?
            if (x_stristr(pAnimType, "MASKED"))
                m_Loco.GetMaskController().SetLooping( bPlayAnimPressed ) ;
            else
            if (x_stristr(pAnimName, "LIP"))
            {
            }
            else
            {
                // Playing an animation?
                if( m_Loco.GetState() == loco::STATE_PLAY_ANIM )
                {
                    // Get anim
                    loco_motion_controller& Cont = m_Loco.GetMotionController();
                   
                    // Exit from play anim?
                    if( ( bPlayAnimPressed == FALSE ) && ( Cont.IsAtEnd() == TRUE ) )
                        m_Loco.SetState( m_Loco.GetPrevState() );
                }
            }
        }
    }
}

//==============================================================================

// Returns animation info as a string
const char* GetAnimInfoString( const anim_info* pInfo )
{
    static char String[256] ;
    static char Name[256] ;
    static char Flags[256] ;

    // Clear strings
    String[0] = 0 ;
    Name  [0] = 0 ;
    Flags [0] = 0 ;

    // No anim?
    if (pInfo == NULL)
        return String ;

    //--- Name
    //

    // Copy name of anim
    x_strcpy(Name, pInfo->GetName()) ;

    //--- Flags
    //

    // Open bracket
    x_strcat(Flags, "[") ;

    // Accum yaw motion flag?
    if (pInfo->AccumYawMotion())
        x_strcat(Flags, "Y") ;
    
    // Accum horiz motion flag?
    if (pInfo->AccumHorizMotion())
        x_strcat(Flags, "H") ;
    
    // Accum vert motion flag?
    if (pInfo->AccumVertMotion())
        x_strcat(Flags, "V") ;
   
    // End bracket
    x_strcat(Flags, "]") ;

    // If no flags were added, clear the string
    if (x_strlen(Flags) == 2)
        Flags[0] = 0 ;

    //-- Join
    //

    x_strcpy(String, Name) ;
    x_strcat(String, " ") ;
    x_strcat(String, Flags) ;

    return String ;
}


// Shows current geometry and animation at top of screen
void viewer_object::ShowInfo( s32& x, s32& y )
{
    // Collect info
    char GeomRsc [256] = {0} ;
    char AnimRsc [256] = {0} ;
    char CurrAnim[256] = {0} ;
    const anim_info* pCurrAnimInfo = NULL ;
    char AudioRsc[256] = {0} ;
    xbool bGeomRscMissing  = FALSE ;
    xbool bAnimRscMissing  = FALSE ;
    xbool bAudioRscMissing = FALSE ;

    // Get geom name from rigid geom?
    if (m_RenderType == RENDER_RIGID)
    {
        x_strcpy(GeomRsc, m_hRigidGeom.GetName()) ;
        bGeomRscMissing = (m_hRigidGeom.GetPointer() == NULL) ;
    }

    // Get geom name from skin geom?
    if (m_RenderType == RENDER_SKIN)
    {
        x_strcpy(GeomRsc, m_hSkinGeom.GetName()) ;
        bGeomRscMissing = (m_hSkinGeom.GetPointer() == NULL) ;
    }

    // Get anim
    x_strcpy(AnimRsc, m_hAnimGroup.GetName()) ;
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer() ;
    if (pAnimGroup)
    {
        // Get anim name from player
        if (m_Type == config_options::TYPE_LOCO)
            pCurrAnimInfo = &(m_Loco.m_Player.GetCurrAnim().GetAnimInfo()) ;
        else
            pCurrAnimInfo = &(m_AnimPlayer.GetAnimInfo()) ;

        // Get name
        if (pCurrAnimInfo)
            x_strcpy(CurrAnim, pCurrAnimInfo->GetName()) ;
    }
    else
    {
        if (AnimRsc[0])
            bAnimRscMissing = TRUE ;
    }

    // Get audio name
    if (m_bAudioSet)
    {
        x_strcpy(AudioRsc, m_hAudioPackage.GetName()) ;
        if (m_hAudioPackage.GetPointer() == NULL)
            bAudioRscMissing = TRUE ;
    }

    // Show full info?
    if (g_StatsMode == STATS_MODE_HIGH)
    {
        // Show geom info
        if (bGeomRscMissing)
        {
            y++ ;
            x_printfxy(x,y++,"ERROR! MISSING GEOMETRY FILE:") ;
            x_printfxy(x,y++, "%s", GeomRsc) ;
            y++ ;
        }
        else
            x_printfxy(x,y++, "%s", GeomRsc) ;
    
        // Show audio info
        if (m_bAudioSet)
        {
            if (bAudioRscMissing)
            {
                y++ ;
                x_printfxy(x,y++,"ERROR! MISSING AUDIO FILE:") ;
                x_printfxy(x,y++, "%s", AudioRsc) ;
                y++ ;
            }
            else
                x_printfxy(x,y++, "%s", AudioRsc) ;
        }

        // Show loco info
        if (m_Type == config_options::TYPE_LOCO)
            x_printfxy(x,y++, "MoveStyle:%s", m_Loco.GetMoveStyleName(m_Loco.GetMoveStyle()) ) ;
    }

    // Show anim info
    if (bAnimRscMissing)
    {
        y++ ;
        x_printfxy(x,y++,"ERROR! MISSING ANIMATION FILE:") ;
        x_printfxy(x,y++, "%s", AnimRsc) ;
        y++ ;
    }
    else
    {
        if (g_StatsMode == STATS_MODE_HIGH)
        {
            x_printfxy(x,y++, "%s", AnimRsc) ;
        }

        if (    (g_StatsMode == STATS_MODE_HIGH) ||
                (g_StatsMode == STATS_MODE_MEDIUM) ||
                (g_StatsMode == STATS_MODE_LOW) )
        {
            x_printfxy(x,y++, "%s", GetAnimInfoString(pCurrAnimInfo)) ;
        }
    }

    // Show selected anim
    if (    (g_StatsMode == STATS_MODE_HIGH) ||
            (g_StatsMode == STATS_MODE_MEDIUM) ||
            (g_StatsMode == STATS_MODE_LOW) )
    {
        if ((pAnimGroup) && (m_iAnim != -1))
        {
            // Show info for selected animation
            const anim_info& AnimInfo = pAnimGroup->GetAnimInfo(m_iAnim) ;
            x_printfxy(x,y++, "%s", GetAnimInfoString(&AnimInfo)) ;
        }
    }


/*
    // Lookup anim group
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer() ;
    if (!pAnimGroup)
        return ;

    // Show heirarchy
    x_printfxy(x,y++,"BoneCount:%d", pAnimGroup->GetNBones()) ;
    for (s32 i = 0 ; i < pAnimGroup->GetNBones() ; i++)
    {
        // Compute indent
        s32 Indent  = 0 ;
        s32 iParent = pAnimGroup->GetBone(i).iParent ;
        while(iParent != -1)
        {
            Indent++ ;
            iParent = pAnimGroup->GetBone(iParent).iParent ;
        }

        // Show bone and parent
        iParent = pAnimGroup->GetBone(i).iParent ;
        if (iParent != -1)
            x_printfxy(x + Indent,y++, "%s       (parent=%s)", 
                       pAnimGroup->GetBone(i).Name,
                       pAnimGroup->GetBone(iParent).Name) ;
        else
            x_printfxy(x + Indent,y++, "%s (root)", 
                       pAnimGroup->GetBone(i).Name) ;
    }
*/
}

//==============================================================================

// Shows current bones and geometry info at the bottom of the screen
void viewer_object::ShowStats( s32 x, s32 y )
{
    // Show geom stats
    if (m_pGeom)
    {
        x_printfxy(x,y--, "Verts:%d Tris:%d VertsPerTri:%.2f", 
                m_nVerts, 
                m_nTris,
                (f32)m_nVerts / (f32)m_nTris) ;

        if ( m_LODs.GetCount() )
            x_printfxy(x,y--, "LOD:%d Bones:%d Mats:%d", m_iLOD, m_nActiveBones, m_iMaterials) ;
        else
            x_printfxy(x,y--, "Bones:%d Mats:%d", m_nActiveBones, m_iMaterials) ;
    }
}

//==============================================================================

// Returns current state
void viewer_object::GetState( viewer_object::state& State )
{
    // Name
    if (m_RenderType == RENDER_SKIN)
        x_strcpy(State.m_CompiledGeom, m_hSkinGeom.GetName()) ;
    else
        x_strcpy(State.m_CompiledGeom, m_hRigidGeom.GetName()) ;

    // L2W
    State.m_L2W = m_L2W ;

    // Get anim state?
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    if( pAnimGroup )
    {
        // Keep selected anim?
        if( ( m_iAnim >= 0 ) && ( m_iAnim < pAnimGroup->GetNAnims() ) )
        {
            const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( m_iAnim );
            x_strcpy( State.m_SelectedAnimName, AnimInfo.GetName() );
        }
        
        // Get anim info from loco?
        if (m_Type == config_options::TYPE_LOCO)
        {
            // Get current anim info
            x_strcpy(State.m_AnimName, m_Loco.m_Player.GetCurrAnim().GetAnimName()) ;
            State.m_Position  = m_Loco.GetPosition() ;
            State.m_AnimFrame = m_Loco.m_Player.GetCurrAnim().GetFrame() ;
            State.m_AnimYaw   = m_Loco.m_Player.GetCurrAnim().GetYaw() ;
            State.m_MoveAt    = m_Loco.GetMoveAt() ;
            State.m_LookAt    = m_Loco.GetHeadLookAt() ;
            State.m_MoveStyle = m_Loco.GetMoveStyle() ;
        }
        else
        {
            // Get current anim info
            if (m_AnimPlayer.GetAnimIndex() != -1)
            {
                x_strcpy(State.m_AnimName, m_AnimPlayer.GetAnimInfo().GetName()) ;
                State.m_AnimFrame = m_AnimPlayer.GetFrame() ;
            }
            else
            {
                // No anim
                State.m_AnimName[0] = 0 ;
                State.m_AnimFrame   = 0 ;
            }
        }
    }

    // View
    State.m_View = m_View ;
}

//==============================================================================

// Sets current state
void viewer_object::SetState( const viewer_object::state& State )
{
    // Get anim state?
    const anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    if( pAnimGroup )
    {
        // Get selected anim
        m_iAnim = pAnimGroup->GetAnimIndex( State.m_SelectedAnimName );
        if( m_iAnim == -1 )
            m_iAnim = 0;
    
        // Get anim info from loco?
        if (m_Type == config_options::TYPE_LOCO)
        {
            // Set current anim info
            m_Loco.SetPosition(State.m_Position) ;
            m_Loco.m_Player.SetAnim(m_hAnimGroup, State.m_AnimName, 0) ;
            m_Loco.m_Player.SetCurrAnimFrame(State.m_AnimFrame) ;
            m_Loco.m_Player.SetCurrAnimYaw(State.m_AnimYaw) ;
            m_Loco.SetMoveAt(State.m_MoveAt) ;
            m_Loco.SetLookAt(State.m_LookAt) ;
            m_Loco.SetMoveStyle(State.m_MoveStyle) ;
            
            // Allow/disable aim styles so that we can toggle through them!
            if (    ( State.m_MoveStyle == loco::MOVE_STYLE_RUNAIM )
                 || ( State.m_MoveStyle == loco::MOVE_STYLE_CROUCHAIM ) )
            {
                m_Loco.SetUseAimMoveStyles(TRUE) ;
            }
            else
            {
                m_Loco.SetUseAimMoveStyles(FALSE) ;
            }
        }
        else
        {
            // Set current anim info
            m_AnimPlayer.SetAnim(State.m_AnimName, FALSE) ;
            m_AnimPlayer.SetFrame((s32)State.m_AnimFrame) ;
        }
    }

    // Setup view
    m_View = State.m_View ;

    // L2W
    SetL2W( State.m_L2W );
}

//==============================================================================
