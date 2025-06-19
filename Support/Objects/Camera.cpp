//=========================================================================
// INCLUDES
//=========================================================================
#include "Camera.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Path.hpp"
#include "Characters\Character.hpp"
#include "Objects\Player.hpp"
#include "Objects\HudObject.hpp"
#include "TriggerEx\TriggerEx_Object.hpp"
#include "TriggerEx\Actions\action_set_property.hpp"


#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif


//=========================================================================
// GLOBALS
//=========================================================================

extern xbool g_first_person;


//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct camera_desc : public object_desc
{
    camera_desc( void ) : object_desc( 
            object::TYPE_CAMERA, 
            "Camera", 
            "SCRIPT",
#ifdef X_EDITOR
            object::ATTR_RENDERABLE | object::ATTR_NEEDS_LOGIC_TIME |
            object::ATTR_SPACIAL_ENTRY,

#else // X_EDITOR
            object::ATTR_NEEDS_LOGIC_TIME,
#endif // X_EDITOR
            FLAGS_GENERIC_EDITOR_CREATE | FLAGS_TARGETS_OBJS |
            FLAGS_IS_DYNAMIC ) {}

    //---------------------------------------------------------------------
    virtual object* Create          ( void ) { return new camera; }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32     OnEditorRender  ( object& Object ) const 
    { 
        object_desc::OnEditorRender( Object );

        return EDITOR_ICON_CAMERA;
    }

#endif // X_EDITOR

} s_camera_Desc;

//=========================================================================

const object_desc& camera::GetTypeDesc( void ) const
{
    return s_camera_Desc;
}

//=========================================================================

const object_desc& camera::GetObjectType( void )
{
    return s_camera_Desc;
}
//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

camera::camera( void ) : tracker()
{
    m_FieldOfView = R_60 ;
    m_FarClip     = 10000.0f ;
    m_PipEarVolume   = 1.0f ;
    m_PipEarNearClip = 1.0f ;
    m_PipEarFarClip  = 1.0f ;
}

//=============================================================================

void camera::OnEnumProp( prop_enum& List )
{
    // Call base class
    tracker::OnEnumProp( List );

    // Add properties
    List.PropEnumHeader("Camera", "Properties for the camera object", 0 );

    s32 iHeader = List.PushPath( "Camera\\" );    

    //guid specific fields
    List.PropEnumGuid  ("TargetGuid",  "Guid of object to look at (if any)", PROP_TYPE_EXPOSE | PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_SAVE);
    m_ObjectAffecter.OnEnumProp( List, "Target" ); //must come after the "TargetGuid" property

    List.PropEnumAngle ("FieldOfView", "Field of view for camera.", 0 ) ;
    List.PropEnumFloat ("FarClip",     "Far clipping plane distance from camera.", 0 ) ;
    
    List.PropEnumFloat ("PipEarVolume",   "Pip ear volume - if this camera is used by a pip.", 0 ) ;
    List.PropEnumFloat ("PipEarNearClip", "Pip ear near clip scaler.", 0 ) ;
    List.PropEnumFloat ("PipEarFarClip",  "Pip ear far clip scaler.", 0 ) ;

    List.PopPath( iHeader );
}

//=============================================================================

xbool camera::OnProperty( prop_query& I )
{
    // Call base class
    if( tracker::OnProperty( I ) )
        return TRUE ;

    if( I.IsSimilarPath( "Camera" ) )
    {
        s32 iHeader = I.PushPath( "Camera\\" );        

        if( m_ObjectAffecter.OnProperty( I, "Target" ) )
        {
            //new guid specific fields
        }
        else if (I.IsVar("TargetGuid"))
        {
            //old Target guid
            if( I.IsRead() )
            {
                I.SetVarGUID( m_ObjectAffecter.GetGuid() );
            }
            else
            {
                //write the new target
                m_ObjectAffecter.SetStaticGuid(I.GetVarGUID());
            }
        }
        else if (I.VarAngle("FieldOfView", m_FieldOfView, 0, R_180))
        {
            // FieldOfView?
        }
        else if (I.VarFloat("FarClip", m_FarClip,0, 50000))
        {
            // Far clip?
        }
        else if (I.VarFloat("PipEarVolume", m_PipEarVolume, 0.0f, 1.0f))
        {
            // PipEarVolume?
        }
        else if (I.VarFloat("PipEarNearClip", m_PipEarNearClip, 0.0f, 10.0f))
        {
            // PipEarNearClip?
        }
        else if (I.VarFloat("PipEarFarClip", m_PipEarFarClip, 0.0f, 10.0f))
        {
            // PipEarFarClip?
        }
        else
        {
            I.PopPath( iHeader );
            return FALSE;
        }

        I.PopPath( iHeader );
        return TRUE;
    }

    return FALSE ;
}

//=========================================================================

void camera::OnMove( const vector3& NewPos )
{
    // So that the camera rotation is updated (if there is a target)
    // we actually need to call OnTransform!
    matrix4 L2W = GetL2W() ;
    L2W.SetTranslation(NewPos) ;
    OnTransform(L2W) ;
}

//=========================================================================

void camera::OnTransform( const matrix4& L2W )
{
    // Lookup position
    vector3 NewPos = L2W.GetTranslation();

    // Lookup target guid
    guid TargetGuid = m_ObjectAffecter.GetGuid();

    // Get target pos from tracker?
    object_ptr<tracker> pTracker( TargetGuid );
    if( pTracker )
    {
        m_TargetPos = pTracker->GetCurrentKey().m_Position;
    }        
    else
    {
        // Get target pos from character?
        object_ptr<character> pCharacter( TargetGuid );
        if( pCharacter )
        {
            m_TargetPos = pCharacter->GetPositionWithOffset( character::OFFSET_EYES );
        }            
        else
        {
            // Get target pos from object?
            object_ptr<object> pObject( TargetGuid );
            if ( pObject )
            {
                m_TargetPos = pObject->GetPosition();
            }                
            else
            // If no target, then face forwards
            if ( TargetGuid == 0 )
            {
                // Update target
                m_TargetPos = NewPos + L2W.RotateVector( vector3( 0,0,5 ) );
            }                
            else
            {
                // Leave the target untouched
            }
        }
    }

    // Update view properties
    m_View.SetXFOV( m_FieldOfView );
    m_View.SetZLimits( 10.0f, m_FarClip );

    // Compute new L2W using target?
    matrix4 LookL2W;
    if( TargetGuid )
    {
        // Setup rotation
        vector3 DeltaTarget = m_TargetPos - NewPos;
        LookL2W.Identity();
        LookL2W.RotateX( DeltaTarget.GetPitch() );
        LookL2W.RotateY( DeltaTarget.GetYaw() );
        LookL2W.PreRotateZ( m_CurrentKey.m_Roll );

        // Setup translation
        LookL2W.SetTranslation( NewPos );
    }
    else
    {
        // Just use the transform as is so that cinema roll comes through for the player
        LookL2W = L2W;
    }
    
    // Update view
    m_View.SetV2W( LookL2W );

    // Call base class with final LookL2W!
    tracker::OnTransform( LookL2W );
}
    
//=========================================================================

void camera::OnInit( void )
{
    // Call base class
    tracker::OnInit() ;
}

//=========================================================================

#ifndef X_RETAIL
void camera::OnDebugRender  ( void )
{
#ifdef X_EDITOR
    CONTEXT("camera::OnDebugRender" );

    // Make sure view is updated!
    Update(FALSE) ;

    // Draw frustrum where view is
    draw_ClearL2W() ;
    draw_Frustum(m_View, XCOLOR_RED, 100.0f) ;
    draw_Line(m_View.GetPosition(), m_TargetPos, XCOLOR_RED) ;
#endif // X_EDITOR
}
#endif // X_RETAIL

//=========================================================================

void camera::OnRender  ( void )
{
#ifdef X_EDITOR
    CONTEXT("camera::OnRender" );

    // Make sure view is updated!
    Update(FALSE) ;

    // Draw frustrum where view is
    draw_ClearL2W() ;
    draw_Frustum(m_View, XCOLOR_RED, 100.0f) ;
    draw_Line(m_View.GetPosition(), m_TargetPos, XCOLOR_RED) ;
#endif // X_EDITOR
}

//=========================================================================

void camera::Update( xbool bSendKeyEvents )
{
    // Call base class which will update L2W
    tracker::Update(bSendKeyEvents) ;

    // If the camera is not on a path, then we need to call an OnTransform
    // so that the look direction is updated!
    if (m_PathGuid == 0)
        OnTransform(GetL2W()) ;

    // Over-ride field of view?
    path* pPath = GetPath() ;
    if (pPath)
    {
        if (pPath->GetFlags() & path::FLAG_KEY_FIELD_OF_VIEW)
            m_FieldOfView = m_CurrentKey.m_FieldOfView ;
    }
}

//=========================================================================

static
s32 ps2_TexLog( s32 Dimension )
{
    switch (Dimension)
    {
    case 8:     return 3;
    case 16:    return 4;
    case 32:    return 5;
    case 64:    return 6;
    case 128:   return 7;
    case 256:   return 8;
    case 512:   return 9;
    default:
        ASSERT( FALSE );
        return 1;
    }
}

//=========================================================================

void camera::RenderViewBegin( const irect& Viewport, s32 VramID )
{
    CONTEXT( "camera::RenderViewBegin" );


#ifdef TARGET_PS2
    if( eng_Begin("camera::RenderViewBegin") )
    {
        // Set the scissor region
        gsreg_Begin( 4 );
        
        // Set context0
        eng_PushGSContext( 0 );
        gsreg_SetScissor( Viewport.l, Viewport.t, Viewport.r, Viewport.b );
        gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                    FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
        eng_PopGSContext();

        // Set context1
        eng_PushGSContext( 1 );
        gsreg_SetScissor( Viewport.l, Viewport.t, Viewport.r, Viewport.b );
        gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                    FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
        eng_PopGSContext();
        
        gsreg_End();

        (void)VramID ;
/*
        // Flush bank 0 where we will be sticking the pip texture
        vram_FlushBank(0);

        // Reserve vram for pip texture
        if (VramID != -1)
        {
            eng_PushGSContext(0) ;
            vram_Activate(VramID) ;
            eng_PopGSContext() ;
        }
*/

        eng_End();
    }

#else
    
    // Not implemented...
    (void)Viewport ;
    (void)VramID ;

#endif  // #ifdef TARGET_PS2

#ifdef X_EDITOR
    // Since in the editor, the camera view is drawn after the normal render,
    // we need to clear rect and reset the z buffer so the camera will draw
    if( eng_Begin("camera::RenderViewBegin") )
    {
        draw_ClearZBuffer(Viewport) ; 
        draw_Rect(Viewport, XCOLOR_BLACK, FALSE) ;
        eng_End() ;
    }
#endif // X_EDITOR

}

//=========================================================================

#ifdef TARGET_PS2

void ps2_ClearScreenAndZBuffer( const irect& Rect, xcolor Color, u32 Z )
{
    s32 Width  = Rect.r - Rect.l ;
    s32 Height = Rect.b - Rect.t ;

    // Copy the screen in vertical strips (this will reduce texture/frame cache misses, and
    // give us a much nicer fill rate), using page-width (64 pixels) columns is sufficient when bilinear is
    // off, but we need half page-width columns when bilinear is on.
    s32 nColumns = Width / 64 ;

    #define GS_X_OFFSET    (2048-(VRAM_FRAME_BUFFER_WIDTH/2))
    #define GS_Y_OFFSET     (2048-(VRAM_FRAME_BUFFER_HEIGHT/2))

    s32 DstDW = Width / nColumns;
    ASSERT( (DstDW % nColumns) == 0 );
    DstDW = DstDW<<4;
    s32 X0      = (GS_X_OFFSET + Rect.l) << 4 ;
    s32 X1      = X0 + DstDW;
    s32 Y0      = (GS_Y_OFFSET + Rect.t) << 4 ;
    s32 Y1      = Y0 + (Height<<4);

    gsreg_Begin( 4 + nColumns*2 ) ;
    gsreg_SetZBufferUpdate(TRUE) ;
    gsreg_SetZBufferTest(ZBUFFER_TEST_ALWAYS) ;
    gsreg_Set( SCE_GS_RGBAQ,  SCE_GS_SET_RGBAQ( Color.R, Color.G, Color.B, Color.A, 0x3f800000 ) );
    gsreg_Set( SCE_GS_PRIM,   SCE_GS_SET_PRIM( GIF_PRIM_SPRITE, 
                                               1,   // 0=flat
                                               0,   // 0=tex map
                                               0,   // 0=fog off
                                               0,   // 0=alpha blend off
                                               0,   // 0=anti-alias off
                                               1,   // 1=2D UVs
                                               0,   // 0=Context0
                                               0    // 0=normal fragment ctrl
                                               ) );
    for ( s32 i = 0; i < nColumns; i++ )
    {
        // Render quad
        gsreg_Set( SCE_GS_XYZ2,   SCE_GS_SET_XYZ2( X0, Y0, Z ) );
        gsreg_Set( SCE_GS_XYZ2,   SCE_GS_SET_XYZ2( X1, Y1, Z ) );

        // Next quad
        X0 += DstDW;
        X1 += DstDW;
    }
    gsreg_End() ;

}

#endif  //#ifdef TARGET_PS2

//=========================================================================

void camera::RenderViewEnd( const irect& Viewport, s32 VramID, s32 TexWidth, s32 TexHeight )

{
    CONTEXT( "camera::RenderViewEnd" );

#ifdef TARGET_PS2

    // Grab frame buffer?
    if (VramID != -1)
    {
        // Must be valid!
        ASSERT(VramID != -1) ;
        ASSERT(TexWidth  > 0) ;
        ASSERT(TexHeight > 0) ;

        // Flush bank 0 where we will be sticking the pip texture
        vram_FlushBank(0);

        // Begin
        if( eng_Begin("Copy camera pip view to vram") )
        {
            eng_PushGSContext(0) ;

            // Allocate vram for this texture and reserve it!
            vram_Activate(VramID) ;

            // Lookup back buffer and texture vram addresses
            s32 ScrPageAddr  = eng_GetFrameBufferAddr(0) / 2048;
            s32 TexBlockAddr = vram_GetPixelBaseAddr(VramID) ;
            s32 TexPageAddr  = TexBlockAddr / 32 ;
        
            // Texture must be page aligned!
            ASSERT((TexBlockAddr & 0x1F) == 0) ;

            // Copy the screen in vertical strips (this will reduce texture/frame cache misses, and
            // give us a much nicer fill rate), using page-width (64 pixels) columns is sufficient when bilinear is
            // off, but we need half page-width columns when bilinear is on.
            s32 nColumns = TexWidth / 64 ;

            // Wait for drawing to finish...
            gsreg_Begin( 12 + nColumns*4 );
            gsreg_Set( SCE_GS_TEXFLUSH, 0 );

            // Turn off z buffer
            gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 0, ALPHA_TEST_FAIL_KEEP,
                                        FALSE, DEST_ALPHA_TEST_0, 
                                        TRUE, ZBUFFER_TEST_ALWAYS );

            // Turn off mipping and bilinear
            gsreg_Set( SCE_GS_TEX1_1, 
                    SCE_GS_SET_TEX1( 1,      // LOD method (1=fixed K)
                                        0,      // Max pip level
                                        0,      // Filter when tex expanded (0=nearest)
                                        1,      // Filter when tex reduced  (0=nearest)
                                        0,      // BaseAddr of Mipmap texture
                                        0,      // L
                                        0 ) );  // K

            // Setup destination (texture)
            gsreg_Set( SCE_GS_FRAME_1, SCE_GS_SET_FRAME( TexPageAddr, TexWidth/64, SCE_GS_PSMCT32, 0x00000000 ) );
        
            // Setup source (screen)
            gsreg_Set( SCE_GS_TEX0_1,  SCE_GS_SET_TEX0 ( ScrPageAddr*32,                       // TexBase (WordAddr/64)
                                                        VRAM_FRAME_BUFFER_WIDTH/64,           // PixelWidth/64
                                                        SCE_GS_PSMCT32,                       // Format
                                                        ps2_TexLog(VRAM_FRAME_BUFFER_WIDTH),   // TW
                                                        ps2_TexLog(VRAM_FRAME_BUFFER_HEIGHT),  // TH
                                                        0,                      // 0=RGB (alpha from vert color), 1=RGBA from tex
                                                        0,                      // 0=MODULATE
                                                        0,                      // ClutBase (WordAddr/64)
                                                        0,                      // ClutFormat
                                                        0,                      // ClutStorage mode
                                                        0,                      // ClutEntryOffset
                                                        0 ) );                  // ClutBufferLoadCtrl

            gsreg_SetScissor(Viewport.l,Viewport.t,Viewport.r,Viewport.b);  // Set scissor
            gsreg_Set( SCE_GS_ALPHA_1, (u64)(-1)) ; // Off
            gsreg_Set( SCE_GS_CLAMP_1, SCE_GS_SET_CLAMP( 1,                 // ClampH
                                                        1,                 // ClampV
                                                        Viewport.l,        // MinU
                                                        Viewport.r-1,      // MaxU
                                                        Viewport.t,        // MinV
                                                        Viewport.b-1 ) );  // MaxV

        #define GS_X_OFFSET    (2048-(VRAM_FRAME_BUFFER_WIDTH/2))
        #define GS_Y_OFFSET     (2048-(VRAM_FRAME_BUFFER_HEIGHT/2))

            s32 SrcDW = TexWidth / nColumns;
            s32 DstDW = TexWidth / nColumns;
            ASSERT( (SrcDW % nColumns) == 0 );
            ASSERT( (DstDW % nColumns) == 0 );
            SrcDW = SrcDW<<4;
            DstDW = DstDW<<4;
            s32 XOffset = (s32)(0*16.0f);
            s32 YOffset = (s32)(0*16.0f);
            s32 X0      = (GS_X_OFFSET<<4) - (1<<3) + XOffset;
            s32 X1      = X0 + DstDW;
            s32 Y0      = (GS_Y_OFFSET<<4) - (1<<3) + YOffset;
            s32 Y1      = Y0 + (TexHeight<<4);
            s32 U0      = 0;
            s32 U1      = SrcDW;
            s32 V0      = 0;
            s32 V1      = (TexHeight<<4);

            gsreg_Set( SCE_GS_RGBAQ,  SCE_GS_SET_RGBAQ( 128, 128, 128, 0, 0x3f800000 ) );
            gsreg_Set( SCE_GS_PRIM,   SCE_GS_SET_PRIM( GIF_PRIM_SPRITE, 
                                                    1,   // 0=flat
                                                    1,   // 1=tex map
                                                    0,   // 0=fog off
                                                    0,   // 0=alpha blend off
                                                    0,   // 0=anti-alias off
                                                    1,   // 1=2D UVs
                                                    0,   // 0=Context0
                                                    0    // 0=normal fragment ctrl
                                                    ) );
            for ( s32 i = 0; i < nColumns; i++ )
            {
                // Render quad
                gsreg_Set( SCE_GS_UV,     SCE_GS_SET_UV  ( U0, V0    ) );
                gsreg_Set( SCE_GS_XYZ2,   SCE_GS_SET_XYZ2( X0, Y0, 0 ) );
                gsreg_Set( SCE_GS_UV,     SCE_GS_SET_UV  ( U1, V1    ) );
                gsreg_Set( SCE_GS_XYZ2,   SCE_GS_SET_XYZ2( X1, Y1, 0 ) );

                // Next quad
                X0 += DstDW;
                X1 += DstDW;
                U0 += SrcDW;
                U1 += SrcDW;
            }

            // Reset states
            gsreg_SetClamping( FALSE, FALSE );
            gsreg_SetFBMASK(0); // Also sets frame buffer address!

            // End
            gsreg_End();
            eng_PopGSContext() ;
            eng_End() ;
        }
    }

    // Restore frame buffers as if nothing was ever drawn...
    if( eng_Begin("Clear camera pip view") )
    {
        // Clear front buffer alpha
        eng_ClearFrontBuffer();
        eng_WriteToBackBuffer();

        // Clear back buffer and Z buffer fast
        ps2_ClearScreenAndZBuffer(Viewport, xcolor(0,0,0,0), 0) ;

        // Reset the viewport and z buffer test
        s32 XRes, YRes;
        eng_GetRes(XRes, YRes);
        gsreg_Begin( 4 );

        // Restore context0
        eng_PushGSContext(0) ;
        gsreg_SetScissor(0,0,XRes,YRes);
        gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                    FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
        eng_PopGSContext() ;

        // Restore context1
        eng_PushGSContext(1) ;
        gsreg_SetScissor(0,0,XRes,YRes);
        gsreg_SetAlphaAndZBufferTests( FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                    FALSE, DEST_ALPHA_TEST_0, TRUE, ZBUFFER_TEST_GEQUAL );
        eng_PopGSContext() ;

        gsreg_End();
        eng_End();
    }

#else

    // Not implemented...
    (void)Viewport ;
    (void)VramID ;
    (void)TexWidth ;
    (void)TexHeight ;

#endif  //#ifdef TARGET_PS2

#ifdef X_EDITOR
    // Fill z buffer so nothing is drawn over the top in editor
    if( eng_Begin("Fill camera viewport Z buffer") )
    {
        draw_FillZBuffer(Viewport) ;
        eng_End() ;
    }
#endif // X_EDITOR

}

//=========================================================================

void camera::RenderView( const irect& Viewport,
                               s32    VramID,
                               s32    TexWidth,
                               s32    TexHeight )
{
    // Keep copy of current view
    view View = *eng_GetView() ;

    // Make sure view is up to date with position and target
    Update(FALSE) ;

    // Set viewport
    m_View.SetViewport(Viewport.l, Viewport.t, Viewport.r, Viewport.b) ;

#ifdef X_EDITOR
    // TEMP to refresh cached values so I can see them in the debugger
    f32 Temp;
    Temp = View.GetXFOV();
    Temp = View.GetYFOV();
    Temp = View.GetScreenDist();
    View.GetV2C();
    
    Temp = m_View.GetXFOV();
    Temp = m_View.GetYFOV();
    m_View.GetV2C();
    m_View.GetScreenDist();
#endif

    // Activate camera view
    eng_SetView     (m_View) ;
    eng_SetViewport (m_View) ;

    // Platform specific stuff
    RenderViewBegin(Viewport, VramID) ;

    // Render world
    g_ObjMgr.Render3dObjects( TRUE, m_View, m_ZoneTracker.GetMainZone() ) ;

    // Platform specific stuff
    RenderViewEnd( Viewport, VramID, TexWidth, TexHeight ) ;
    
    // Restore view
    eng_SetView     (View) ;
    eng_SetViewport (View) ;
}

//=========================================================================

xbool camera::IsEditorSelected( void )
{
    // Is this object selected?
    if (tracker::IsEditorSelected())
        return TRUE ;

    guid ObjGuid = m_ObjectAffecter.GetGuid();

    // Is target object selected?
    object_ptr<object> pObject(ObjGuid) ;
    if (pObject)
    {
        // Is object selected?
        if (pObject->GetAttrBits() & (object::ATTR_EDITOR_SELECTED | object::ATTR_EDITOR_PLACEMENT_OBJECT))
            return TRUE ;
    }

    // Is targe a tracker?
    object_ptr<tracker> pTracker(ObjGuid) ;
    if (pTracker)
    {
        // Is tracker object selected?
        if (pTracker->IsEditorSelected())
            return TRUE ;
    }

    // Not selected
    return FALSE ;
}

//=========================================================================

#ifdef X_EDITOR

static
xbool IsCinemaCameraAction( actions_ex_base* pAction, guid CameraGuid )
{
    // Must be set property action
    if( pAction->GetType() == actions_ex_base::TYPE_ACTION_AFFECT_PROPERTY )
    {
        action_set_property* pSetProperty = (action_set_property*)pAction;
        
        // Setting a player cinema camera guid to use this camera?
        if(         ( pSetProperty->GetCode() == action_set_property::PMOD_CODE_SET )
                &&  ( ( pSetProperty->GetPropertyType() & PROP_TYPE_BASIC_MASK ) == PROP_TYPE_GUID )
                &&  ( pSetProperty->GetPropertyGuid() == CameraGuid )
                &&  ( x_strcmp( pSetProperty->GetPropertyName(), "Player\\Cinema\\CinemaCameraGuid" ) == 0 ) )
        {                
            return TRUE; 
        }
    }
        
    // Not found
    return NULL;        
}

//=========================================================================

static
xbool IsCinemaCamera( guid CameraGuid )
{
    slot_id     SlotID     = SLOT_NULL;
    s32         i          = 0;

    // Loop through all players and search for this camera being used in a cinema
    SlotID = g_ObjMgr.GetFirst( object::TYPE_PLAYER );
    while( SlotID != SLOT_NULL )
    {
        // Get player
        player* pPlayer = (player*)g_ObjMgr.GetObjectBySlot( SlotID );
        ASSERT( pPlayer );

        // Used in cinema?
        if( pPlayer->GetCinemaCameraGuid() == CameraGuid )
            return TRUE;

        // Check next player
        SlotID = g_ObjMgr.GetNext( SlotID );
    }

    // Loop through all triggers to see this camera is referenced in a cinema
    SlotID = g_ObjMgr.GetFirst( object::TYPE_TRIGGER_EX );
    while( SlotID != SLOT_NULL )
    {
        // Get trigger
        trigger_ex_object* pTrigger = (trigger_ex_object*)g_ObjMgr.GetObjectBySlot( SlotID );
        ASSERT( pTrigger );

        // Loop through actions
        for( i = 0; i < pTrigger->GetActionCount(); i++ )
        {
            // Is this a set property action?
            actions_ex_base* pAction = pTrigger->GetAction( i );
            ASSERT( pAction );
            if( IsCinemaCameraAction( pAction, CameraGuid ) )
                return TRUE;
        }

        // Loop through else actions
        for( i = 0; i < pTrigger->GetElseActionCount(); i++ )
        {
            // Is this a set property action?
            actions_ex_base* pAction = pTrigger->GetElseAction( i );
            ASSERT( pAction );
            if( IsCinemaCameraAction( pAction, CameraGuid ) )
                return TRUE;
        }
        
        // Check next trigger
        SlotID = g_ObjMgr.GetNext( SlotID );
    }
    
    // Not found
    return FALSE;
}

//=========================================================================

void camera::RenderEditorView( void )
{
    s32 W, H;
    f32 PixelScale;

    // Lookup the current view info
    const view* pCurrView = eng_GetView();
    ASSERT( pCurrView );
    rect CurrVP;
    pCurrView->GetViewport( CurrVP );
    
    // Setup size
    xbool bIsCinemaCamera = IsCinemaCamera( GetGuid() );
    if( bIsCinemaCamera )
    {
        // PS2 cinema display
        W          = PS2_VIEWPORT_WIDTH / 2;
        H          = PS2_VIEWPORT_HEIGHT / 2;
        PixelScale = PS2_PIXEL_SCALE;
    }
    else
    {
        // Pip display
        W          = 256;
        H          = 128;
        PixelScale = DEFAULT_PIXEL_SCALE;
    }

    // Setup viewport    
    irect Viewport;
    Viewport.l = ( (s32)CurrVP.GetWidth() - W ) / 2;
    Viewport.r = Viewport.l + W;
    Viewport.t = g_first_person ? (s32)( CurrVP.GetHeight() * 0.21f ) : 8;
    Viewport.b = Viewport.t + H;

    // Skip if bigger than the window
    if( W > (s32)CurrVP.GetWidth() )
        return;
    if( H > (s32)CurrVP.GetHeight() )
        return;
        
    // Render
    m_View.SetPixelScale( PixelScale );
    RenderView( Viewport );
    
    // Render letter box?        
    if( ( bIsCinemaCamera ) && ( eng_Begin() ) )
    {
        rect VP;
        VP.Min.Set( (f32)Viewport.l, (f32)Viewport.t );
        VP.Max.Set( (f32)Viewport.r, (f32)Viewport.b );
        hud_object::RenderLetterBox( VP, 1.0f );
        eng_End();
    }
        
    // Render red border
    if( eng_Begin() )    
    {
        draw_Rect( Viewport, XCOLOR_RED, TRUE );
        eng_End();
    }
}

#endif

//=========================================================================

void camera::OnAdvanceLogic( f32 DeltaTime )
{
    // Call base class for camera position
    tracker::OnAdvanceLogic(DeltaTime) ;
}

//=========================================================================
