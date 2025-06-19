//=========================================================================
// INCLUDES
//=========================================================================
#include "Entropy.hpp"
#include "Pip.hpp"
#include "Camera.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "EventMgr\EventMgr.hpp"
#include "Render\LightMgr.hpp"
#include "PlaySurface.hpp"
#include "GameLib\RenderContext.hpp"

#ifdef TARGET_XBOX
extern s32 xbox_GetPipTexture(void);
#endif

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif


//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct pip_desc : public object_desc
{
    pip_desc( void ) : object_desc( 
            object::TYPE_PIP, 
            "Pip", 
            "SCRIPT",
            object::ATTR_NEEDS_LOGIC_TIME,
            FLAGS_GENERIC_EDITOR_CREATE | FLAGS_TARGETS_OBJS |
            FLAGS_IS_DYNAMIC ) {}

    //---------------------------------------------------------------------
    virtual object* Create          ( void ) { return new pip; }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32     OnEditorRender  ( object& Object ) const 
    { 
        object_desc::OnEditorRender( Object );

        // Render pip geometry?
        object_ptr<pip> pPip(Object.GetGuid());
        if ((pPip) && (pPip->GetType() == pip::TYPE_HUD))
        {
            //pPip->OnRender();
        }

        return EDITOR_ICON_PIP;
    }

#endif // X_EDITOR

} s_pip_Desc;

//=========================================================================

const object_desc& pip::GetTypeDesc( void ) const
{
    return s_pip_Desc;
}

//=========================================================================

const object_desc& pip::GetObjectType( void )
{
    return s_pip_Desc;
}
//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

pip::pip( void )
{
    m_PipVramID     = 0;
    m_Width         = 256;
    m_Height        = 128;
    m_nTextureRefs  = 0;
    
    m_CameraGuid    = 0;
    m_Type          = TYPE_HUD;
    m_State         = STATE_INACTIVE;

    m_EarGuid       = 0 ;
    m_EarID         = 0 ;
}

//=============================================================================

void pip::OnEnumProp( prop_enum& List )
{
    // Call base class
    object::OnEnumProp( List );

    // Add properties
    m_RenderInst.OnEnumProp(List);

    List.PropEnumExternal( "RenderInst\\AnimFile", "Resource\0anim", "Resource Animation File", PROP_TYPE_MUST_ENUM );
    List.PropEnumHeader("Pip",                  "Properties for the picture-in-picture object.", 0 );
    List.PropEnumEnum  ("Pip\\Type",            "HUD\0WORLD\0", "Type of pip. HUD = Hud overlay type. WORLD = In world object.", PROP_TYPE_MUST_ENUM);
    List.PropEnumGuid  ("Pip\\CameraGuid",      "Guid of the camera to view from.", PROP_TYPE_EXPOSE);
    if (m_Type == TYPE_WORLD)
    {
        for (s32 i = 0; i < MAX_WORLD_OBJECTS; i++)
        {
            List.PropEnumHeader(xfs("Pip\\WorldObject[%d]", i), "World object info.", 0 );
            List.PropEnumGuid  (xfs("Pip\\WorldObject[%d]\\Guid", i), "Guid of world object to control.", 0 );
        }
    }
}

//=============================================================================

xbool pip::OnProperty( prop_query& I )
{
    // Call base class
    if (object::OnProperty(I))
        return TRUE;

    // Render inst?
    if (m_RenderInst.OnProperty(I))
        return TRUE;

    // AnimFile?
    if( I.IsVar( "RenderInst\\AnimFile" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hAnimGroup.GetName(), RESOURCE_NAME_SIZE );
        }
        else            
        {
            m_hAnimGroup.SetName( I.GetVarExternal() );

            // If we can load this animgoup then we need to extract some info
            if (m_hAnimGroup.GetPointer() )
                OnInit();
        }
        return TRUE;
    } 

    // Type
    if( I.IsVar("Pip\\Type" ) )
    {
        if( I.IsRead() )
        {
            switch(m_Type)
            {
                case TYPE_HUD:   I.SetVarEnum("HUD");   break;
                case TYPE_WORLD: I.SetVarEnum("WORLD"); break;
            }
        }
        else
        {
            if (!x_stricmp(I.GetVarEnum(), "HUD"))
                m_Type = TYPE_HUD;
            else if (!x_stricmp(I.GetVarEnum(), "WORLD"))
                m_Type = TYPE_WORLD;
        }
        return TRUE;
    }

    // Camera guid
    if (I.VarGUID("Pip\\CameraGuid", m_CameraGuid))
        return TRUE;
    
    // World object?
    if (I.IsSimilarPath("Pip\\WorldObject["))
    {
        s32 Index = I.GetIndex(0);

        // Guid?
        if (I.VarGUID("Pip\\WorldObject[]\\Guid", m_WorldObjectGuids[Index]))
            return TRUE;
    }
    
    return FALSE;
}

//=========================================================================

void pip::OnInit( void )
{
    // Initialize animation
    m_AnimPlayer.SetAnimGroup(m_hAnimGroup);

    // Setup vram texture
    m_Width  = 256;
    m_Height = 128;

#ifdef TARGET_PS2
    // Register a locked area of vram
    m_PipVramID    = vram_RegisterLocked(m_Width, m_Height, 32);
#endif

#ifdef TARGET_XBOX
    // Figure out what the pip render target id is
    m_PipVramID   = xbox_GetPipTexture();
#endif

    // Reset state
    SetupState(STATE_INACTIVE);
}

//=========================================================================

void pip::OnKill( void )
{
#ifdef TARGET_PS2
    // Release vram
    vram_Unregister(m_PipVramID);
#endif

#ifdef TARGET_XBOX
    // note: doesn't need to destroy anything
#endif

    // Turn off pip texture
    ActivatePipTexture(FALSE);

    // Destroy ear?
    if (m_EarID)
    {
        g_AudioMgr.DestroyEar(m_EarID) ;
        m_EarID   = 0 ;
        m_EarGuid = 0 ;
    }
}

//=========================================================================

static vector3 PIP_OBJECT_POS(100,80,400);
static f32     PIP_OBJECT_YAW = 0;

//static vector3 PIP_LIGHT_POS (0,0,0);
//static f32     PIP_LIGHT_RADIUS = 200.0f;
//static f32     PIP_LIGHT_INTENSITY = 0.5f;

void pip::OnRender( void )
{
    s32 i;

    CONTEXT("pip::OnRender" );

#ifdef TARGET_PS2
    // Since post effects has cleared vram, we need to reserve the pip vram id again!
    if (m_PipVramID != -1)
    {
        vram_Activate(m_PipVramID);
    }
#endif

    // Should only get here for the hud render!
    ASSERT(m_Type == pip::TYPE_HUD);

    // Get active view
    const view* pView = eng_GetView();

    // Get viewport
    irect Viewport;
    pView->GetViewport(Viewport.l, Viewport.t, Viewport.r, Viewport.b);

    // Clear all of z buffer
    draw_ClearZBuffer(Viewport);

    // Compute L2W to be infront of view
    matrix4 L2W;
    L2W.Identity();
    L2W.RotateY(PIP_OBJECT_YAW);
    L2W.SetTranslation(PIP_OBJECT_POS);
    L2W = pView->GetV2W() * L2W;

    // Put light at camera
    //g_LightMgr.AddDynamicLight(pView->GetV2W() * PIP_LIGHT_POS,     // Pos
                               //XCOLOR_WHITE,                        // Color
                               //PIP_LIGHT_RADIUS,                    // Radius
                               //PIP_LIGHT_INTENSITY,                 // Intensity
                               //TRUE);                              // bCharOnly

    // Render geometry?
    geom* pGeom = m_RenderInst.GetGeom();
    if (pGeom)
    {
        // Compute flags
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
#ifdef X_EDITOR
        if ( GetAttrBits() & ATTR_EDITOR_SELECTED )
            Flags |= render::WIREFRAME;
#endif

        // Compute LOD mask
        u64 LODMask = m_RenderInst.GetLODMask(GetL2W());
        if (LODMask == 0)
            return;

        // Setup ambient color
        xcolor Ambient(255,255,255,255);

        // Use animation?
        if (m_hAnimGroup.IsLoaded())
        {
            // Compute matrices
            m_AnimPlayer.SetL2W(L2W);
            const matrix4* pMatrices = m_AnimPlayer.GetBoneL2Ws();

            // Render
            m_RenderInst.Render( pMatrices, pMatrices, pGeom->m_nBones, Flags, LODMask, Ambient );
        }
        else
        {
            // Allocate matrices
            matrix4* pMatrices = (matrix4*)smem_BufferAlloc(pGeom->m_nBones * sizeof(matrix4));
            if (!pMatrices)
                return;

            // Use identity
            for (i = 0; i < pGeom->m_nBones; i++)
                pMatrices[i] = L2W;

            // Render
            m_RenderInst.Render( pMatrices, pMatrices, pGeom->m_nBones, Flags, LODMask, Ambient );
        }
    }
    else
    {
#if !defined( CONFIG_RETAIL )
        draw_BBox( GetBBox() );
#endif // !defined( CONFIG_RETAIL )
    }
}

//=========================================================================

void pip::OnAdvanceLogic( f32 DeltaTime )
{
    // Advance animation
    if (m_hAnimGroup.IsLoaded())
    {
        m_AnimPlayer.Advance(DeltaTime);
        g_EventMgr.HandleSuperEvents(m_AnimPlayer, this);
    }

    // Advance state
    AdvanceState(DeltaTime);

    // Update ear
    UpdateEar() ;
}

//=========================================================================

void pip::OnActivate( xbool bFlag )
{
    // Activate?
    if (bFlag)
        SetupState(STATE_ENTER);
    else
        SetupState(STATE_EXIT);
}

//=========================================================================

void pip::RenderView( void )
{
    // Get camera
    object_ptr<camera> pCamera(m_CameraGuid);
    if (pCamera)
    {
        // Setup viewport
        irect Viewport;

#ifdef TARGET_PC
        Viewport.l = 256-8;
        Viewport.t = 48;
        Viewport.r = Viewport.l + m_Width;
        Viewport.b = Viewport.t + m_Height;
#else
        Viewport.l = 0;
        Viewport.t = 0;
        Viewport.r = m_Width;
        Viewport.b = m_Height;
#endif

        g_RenderContext.SetPipRender( TRUE );
        pCamera->RenderView(Viewport, m_PipVramID, m_Width, m_Height);
        g_RenderContext.SetPipRender( FALSE );
    }
}

//=========================================================================

void pip::AddTextureRefs( object* pObject )
{ 
    // No object?
    if (!pObject)
        return;

    // Lookup inst
    render_inst* pRenderInst = pObject->GetRenderInstPtr();
    if (!pRenderInst)
        return;

    // Lookup geometry
    const geom* pGeom = pObject->GetGeomPtr();
    if (!pGeom)
        return;

    // Search for a mesh names "PIP"
    s32 iSubMesh = 0;
    for (s32 i = 0; i < pGeom->m_nMeshes; i++)
    {
        // Is this a pip mesh?
        if (x_stricmp(pGeom->GetMeshName( i ), "PIP") == 0)
        {
            // Use first sub-mesh
            iSubMesh = pGeom->m_pMesh[i].iSubMesh;
            break;
        }
    }

    // Force render library to use top mip (we only have 1!) always since the render
    // library thinks the bitmap has mips since we just cheated and set the bitmap vram id 
    // to use the pips!
    pGeom->m_pSubMesh[iSubMesh].WorldPixelSize = 1000.0f;

    // Lookup material on this submesh
    material& Material = render::GetMaterial(pRenderInst->GetInst(), iSubMesh);
#ifndef TARGET_PC
    // Lookup texture handle
    texture::handle hTexture = Material.m_DiffuseMap;

    // Lookup texture
    texture* pTexture = hTexture.GetPointer();
    if (!pTexture)
        return;

    // Setup next texture ref
    ASSERT(m_nTextureRefs <= MAX_TEXTURE_REFS);
    texture_ref& Ref = m_TextureRefs[m_nTextureRefs++];
    Ref.m_hTexture = hTexture;
    Ref.m_VramID   = pTexture->m_Bitmap.GetVRAMID();
#endif
}

//=========================================================================

void pip::InitTextureRefs( void )
{
#ifndef TARGET_PC

    // Clear texture ref list
    ASSERT(m_nTextureRefs == 0);

    // Add hud object?
    if (m_Type == TYPE_HUD)
        AddTextureRefs(this);

    // Using world type?
    if (m_Type == TYPE_WORLD)
    {
        // Check all objects
        for (s32 i = 0; i < MAX_WORLD_OBJECTS; i++)
        {
            // Link to object textures
            object* pObject = g_ObjMgr.GetObjectByGuid(m_WorldObjectGuids[i]);
            if( pObject)
                AddTextureRefs( pObject );
        }
    }
#endif
}

//=========================================================================

void pip::ActivatePipTexture( xbool bEnable )
{
#ifndef TARGET_PC
    // Lookup textures?
    if (m_nTextureRefs == 0)
        InitTextureRefs();

    // Update all texture refs
    for (s32 i = 0; i < m_nTextureRefs; i++)
    {
        // Lookup texture ref
        texture_ref& Ref = m_TextureRefs[i]; 

        // Lookup texture
        texture* pTexture = Ref.m_hTexture.GetPointer();
        ASSERT(pTexture);

        // Turn on?
        if (bEnable)
            pTexture->m_Bitmap.SetVRAMID(m_PipVramID); // Set pip
        else
            pTexture->m_Bitmap.SetVRAMID(Ref.m_VramID);    // Restore
    }
#endif
}

//=========================================================================

void pip::SetupState( state State )
{
    // Record and setup new state
    m_State = State;
    switch(State)
    {
        case STATE_INACTIVE:
            // No logic needed anymore...
            SetAttrBits(GetAttrBits() & ~ATTR_NEEDS_LOGIC_TIME);
            ActivatePipTexture(FALSE);
            break;

        case STATE_ENTER:
        {
            // Only one pip at a time allowed so turn off all other pips
            select_slot_iterator SlotIter;
            g_ObjMgr.SelectByAttribute( object::ATTR_ALL, object::TYPE_PIP );
            for( SlotIter.Begin(); !SlotIter.AtEnd(); SlotIter.Next() )
            {
                // Get pip
                pip* pPip = (pip*)SlotIter.Get();
                ASSERT( pPip );

                // Is this pip about to enter or active?
                if(( pPip != this ) && (( pPip->GetState() == STATE_ENTER) || ( pPip->GetState() == STATE_ACTIVE )))
                {
                    // Turn off now!
                    pPip->SetupState(STATE_EXIT);
                }
            }
            SlotIter.End();

            // Need logic time
            SetAttrBits(GetAttrBits() | ATTR_NEEDS_LOGIC_TIME);

            // Start animation
            if (m_hAnimGroup.IsLoaded())
                m_AnimPlayer.SetAnim("ENTER");

            // Turn on texture
            ActivatePipTexture(TRUE);
        }
        break;

        case STATE_ACTIVE:
            // Nothing to do...
            break;

        case STATE_EXIT:
            g_AudioMgr.DestroyEar(m_EarID) ;
            m_EarID = 0 ;
            // Start animation
            if (m_hAnimGroup.IsLoaded())
                m_AnimPlayer.SetAnim("EXIT");
            break;
    }
}

//=========================================================================

void pip::AdvanceState( f32 DeltaTime )
{
    (void)DeltaTime;

    // Advance state
    switch(m_State)
    {
        case STATE_INACTIVE:
            // Nothing to do...
            break;

        case STATE_ENTER:
            // If done, goto active state
            if ( (m_hAnimGroup.IsLoaded() == FALSE) || (m_AnimPlayer.IsAtEnd()) )
                SetupState(STATE_ACTIVE);
            break;

        case STATE_ACTIVE:
            // Nothing to do...
            break;

        case STATE_EXIT:
            // If done, goto sleep
            if ( (m_hAnimGroup.IsLoaded() == FALSE) || (m_AnimPlayer.IsAtEnd()) )
                SetupState(STATE_INACTIVE);
            break;
    }
}

//=========================================================================

void pip::UpdateEar( void )
{
    // Nothing to do if inactive
    if( m_State == STATE_INACTIVE )
    {
        // Destroy ear?
        if (m_EarID)
        {
            g_AudioMgr.DestroyEar(m_EarID) ;
            m_EarID   = 0 ;
        }
        return ;
    }

    // Must have a camera before ear is used
    object_ptr<camera> pCamera( m_CameraGuid );
    if( pCamera )
    {
        // Does ear need to be created?
        if( !m_EarID )
        {
            // Create if pip is entering or active
            if( ( m_State == STATE_ENTER ) || ( m_State == STATE_ACTIVE ) )
                m_EarID = g_AudioMgr.CreateEar();
        }

        // Update ear?
        if( m_EarID )
        {
            // Compute camera world -> view
            matrix4 W2V = pCamera->GetL2W();
            W2V.InvertRT();

            // Update ear
            g_AudioMgr.SetEar(m_EarID, 
                                W2V, 
                                pCamera->GetPosition(),
                                pCamera->GetZone1(),
                                pCamera->GetPipEarVolume());
        }
    }
}

//=========================================================================

render_inst* pip::GetRenderInstPtr( void )
{
    // Geometry is only used for HUD object pips
    if( m_Type == TYPE_HUD )
        return &m_RenderInst;
    else
        return NULL;
}

//=========================================================================

anim_group::handle* pip::GetAnimGroupHandlePtr ( void )
{
    // Animation is only used for HUD object pips
    if( m_Type == TYPE_HUD )
        return &m_hAnimGroup;
    else
        return NULL;
}

//=========================================================================
