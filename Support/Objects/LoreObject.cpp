//==============================================================================
// Lore OBJECT
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "LoreObject.hpp"
#include "e_Draw.hpp"
#include "e_View.hpp"
#include "Entropy.hpp"
#include "x_math.hpp"
#include "..\Support\TriggerEx\triggerex_object.hpp"

#include "InputMgr\GamePad.hpp"
#include "Objects\HudObject.hpp"

#include "GameTextMgr\GameTextMgr.hpp"
#include "StringMgr\StringMgr.hpp"

// collision/polycache stuff
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "GameLib\RigidGeomCollision.hpp"

#ifndef X_EDITOR
#include "StateMgr\StateMgr.hpp"
#include "GameLib\RenderContext.hpp"
#include "NetworkMgr\MsgMgr.hpp"
#include "StateMgr\LoreList.hpp"
#endif

#ifdef TARGET_XBOX
// see nasty HACK comment below
extern void xbox_GetRes( s32& XRes,s32& YRes );
#endif

//=========================================================================
// GLOBALS
//=========================================================================

#ifndef CONFIG_RETAIL 
xbool g_bDrawLODebug = FALSE;
xbool g_ShowLoreObjectCollision = FALSE;
#endif

rhandle<xbitmap>            lore_object::m_Bracket;

f32 s_LoreItemMsgFadeTime   = 2.0f;
f32 g_LO_OffsetY            = 5.0f;

#ifndef X_EDITOR
    extern lore_list g_LoreList;
#endif

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

//=========================================================================
static struct lore_object_desc : public object_desc
{
    lore_object_desc( void ) : object_desc( 
            object::TYPE_LORE_OBJECT, 
            "Lore Object",
            "HUD",
            object::ATTR_COLLIDABLE             | 
            object::ATTR_SPACIAL_ENTRY          |
            object::ATTR_BLOCKS_ALL_PROJECTILES | 
            object::ATTR_BLOCKS_ALL_ACTORS      | 
            object::ATTR_BLOCKS_RAGDOLL         | 
            object::ATTR_BLOCKS_CHARACTER_LOS   | 
            object::ATTR_BLOCKS_PLAYER_LOS      | 
            object::ATTR_BLOCKS_PAIN_LOS        | 
            object::ATTR_BLOCKS_SMALL_DEBRIS    | 
            object::ATTR_RENDERABLE             |
            object::ATTR_RECEIVE_SHADOWS        |
            object::ATTR_TRANSPARENT            |
            object::ATTR_NEEDS_LOGIC_TIME,

            FLAGS_GENERIC_EDITOR_CREATE |
            FLAGS_BURN_VERTEX_LIGHTING  |
            FLAGS_NO_ICON               |
            FLAGS_IS_DYNAMIC            |
            FLAGS_TARGETS_OBJS ) {}         

    //---------------------------------------------------------------------

    virtual object* Create( void )
    {
        return new lore_object;
    }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32 OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return EDITOR_ICON_ANCHOR; 
        //return -1;
    }

#endif // X_EDITOR

} s_loreObject_Desc;

//=========================================================================

const object_desc&  lore_object::GetTypeDesc( void ) const
{
    return s_loreObject_Desc;
}

//=========================================================================

const object_desc&  lore_object::GetObjectType( void )
{
    return s_loreObject_Desc;
}


//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

lore_object::lore_object( void ) 
{
    // Set the initial state.
    m_bActive   = TRUE;

    m_bDestroyAfterAcquired = TRUE;

    m_bActivateTriggerOnRestore = FALSE;

    m_TriggerGuid   = 0;
    m_SpatialGuid   = 0;

    m_ViewDist      = 800;
    m_TextDist      = 500;
    m_ScanDist      = 500;
    m_bScanLOS      = TRUE;

    m_DisplayStrTable  = -1;
    m_DisplayStrTitle  = -1;

    m_pScanAudio = "FocusObject_Positive";

    vector3 Corner( 50.0f, 50.0f, 50.0f );
    m_FocusBox.Set( Corner, -Corner );

    m_ViewType = SELECT_BRACKETS;
   
    // Internal state stuff.
    m_AnimState         = 1.0f;
    m_TextAlphaState    = 0.0f;
    m_bLookingAt        = FALSE;
    m_bHasLOS           = FALSE;
    m_bInViewRange      = FALSE;
    m_bInTextRange      = FALSE;
    m_bInScanRange      = FALSE;
    m_bStandingIn       = FALSE;

    m_ColorPhase        = 0.0f;
    m_bUseGeometrySize  = TRUE;
    m_MaxAlpha          = 0;
    m_LoreID            = -1;

    m_VMeshMask = 0;
    m_VMeshMask = ~m_VMeshMask;

    m_bAutoOff          = TRUE;

    m_Bracket.SetName( PRELOAD_FILE("Bracket.xbmp") );
}

//=========================================================================

lore_object::~lore_object( void ) 
{
}

//=========================================================================
#ifdef X_EDITOR
xbool lore_object::IsIconSelectable( void ) 
{ 
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();

    if( pRigidGeom )
    {
        return FALSE;
    }
    
    return TRUE; 
}
#endif

//=========================================================================
#ifndef X_RETAIL
void lore_object::OnDebugRender( void )
{
    // if we are using the geometry's collision size
    if( m_bUseGeometrySize )
    {
        rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();

        if( pRigidGeom )
        {            
            draw_BBox( GetBBox() );
            return;
        }
    }

    // no geometry or we overrode setting above
    {             
        bbox BBox = m_FocusBox;
        BBox.Transform( GetL2W() );
        draw_BBox( BBox, XCOLOR_GREEN );
    }

    draw_Marker( GetPosition() );
}
#endif // X_RETAIL

//=========================================================================
f32 g_LO_SphereSize = 3.0f;
f32 g_LO_RenderDist = 600.0f;
f32 g_Dist = 0.0f;
void lore_object::OnRender( void )
{
    CONTEXT( "lore_object::OnRender" );

    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();

    if( pRigidGeom )
    {
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
        if( GetAttrBits() & object::ATTR_DISABLE_PROJ_SHADOWS )
            Flags |= render::DISABLE_PROJ_SHADOWS;

        if ( pRigidGeom->m_nBones > 1 )
        {
#ifdef X_EDITOR
            // only display the error message once
            static xbool Once = TRUE;
            if ( Once )
            {
                Once = FALSE;
                x_try;
                x_throw( xfs( "Lore object can't use multi-bone geometry (%s)", m_RigidInst.GetRigidGeomName() ) );
                x_catch_display;
            }
#else
            ASSERTS( 0, xfs( "Lore object can't use multi-bone geometry (%s)", m_RigidInst.GetRigidGeomName() ) );
#endif
        }
        else
        {
            m_RigidInst.Render( &GetL2W(), Flags | GetRenderMode() );
        }
    }
    else
    {
#ifdef X_EDITOR
        draw_BBox( GetBBox() );
#endif // X_EDITOR
    }
}
/*
{
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();

    if( pRigidGeom )
    {
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;

        // Setup Render Matrix
        m_RenderL2W = GetL2W();
        //m_RenderL2W.SetRotation(m_TotalSpin);

        m_RigidInst.SetVMeshMask( m_VMeshMask );
        m_RigidInst.Render( &m_RenderL2W, Flags, m_RigidInst.GetLODMask(m_RenderL2W) );
    }

#ifdef X_EDITOR
    //    draw_Line( GetPosition() , GetPosition() + m_NormalCollision );
    //    draw_Line( GetPosition() , GetPosition() + m_Velocity , XCOLOR_BLUE );
#endif // X_EDITOR
}
*/

//=========================================================================
void lore_object::DoCollisionCheck( player *pPlayer, vector3 &StartPos, vector3 &EndPos )
{
    // offset it a bit for objects whose positions end up in the floor            
    EndPos = GetBBox().GetCenter() + vector3(0.0f, g_LO_OffsetY, 0.0f);

    // get player's eye position
    StartPos = pPlayer->GetEyesPosition();

    // set up LOS check for collision manager
    g_CollisionMgr.LineOfSightSetup( pPlayer->GetGuid(), StartPos, EndPos );

    // add this object to ignore list
    g_CollisionMgr.AddToIgnoreList(GetGuid());

    // Only need one collision to block LOS.
    g_CollisionMgr.SetMaxCollisions(1);

    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
        object::ATTR_BLOCKS_PLAYER_LOS,
        (object::object_attr)(object::ATTR_COLLISION_PERMEABLE|object::ATTR_LIVING) );
}

//=========================================================================
void lore_object::OnAdvanceLogic( f32 DeltaTime )
{
    if( !m_bActive )
    {
        return;
    }
    //
    // This deals with the fact that the LO will not run OnRenderTransparent 
    // when off screen, which meant that if you walked through it, 
    // m_bLookingAt would stay true until the LO again entered the screen.
    // This code assumes that if you haven't rendered (where m_bRendered is 
    // set to true) since the last logic pass, then you must not be looking
    // at the LO.
    //
    if( !m_bRendered )
    {
        m_bLookingAt = FALSE;
    }
    m_bRendered = FALSE;
    
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if( !pPlayer )
        return;

    //
    // Perform LOS collision.
    //
    {
        // Distance checks.
        f32 Dist = (GetPosition() - pPlayer->GetEyesPosition()).Length();
        m_bInViewRange     = (Dist < m_ViewDist);
        m_bInTextRange     = (Dist < m_TextDist);
        m_bInScanRange     = (Dist < m_ScanDist);

        m_bStandingIn = FALSE;

        // Only check collision with trigger and LOS if we're within viewable distance.
        if( m_bInViewRange )
        {
            //
            // Check the spatial trigger, if any.
            //
            {
                if( m_SpatialGuid != 0 )
                {
                    object* pObject = g_ObjMgr.GetObjectByGuid( m_SpatialGuid );
                    if( pObject )
                    {
                        if( pObject->IsKindOf( trigger_ex_object::GetRTTI() ) )
                        {
                            trigger_ex_object* pTrigger = (trigger_ex_object*)pObject;

                            ASSERTS( pTrigger->GetTriggerType() == trigger_ex_object::TRIGGER_TYPE_SPATIAL, 
                                "Lore Object attached to non spatial trigger!" );
                            if( pTrigger->GetTriggerType() != trigger_ex_object::TRIGGER_TYPE_SPATIAL )
                            {
                                m_bStandingIn = TRUE;
                            }
                            else
                            {
                                bbox TriggerBBox = pTrigger->GetColBBox();
                                bbox PlayerBBox  = pPlayer->GetColBBox();
                                m_bStandingIn = PlayerBBox.Intersect( TriggerBBox );
                            }
                        }
                    }
                }
                else
                {
                    // If there was no spatial trigger guid assigned,
                    // any standing location is valid.
                    m_bStandingIn = TRUE;
                }
            }

            vector3 StartPos, EndPos;

            // LOS check.
            DoCollisionCheck( pPlayer, StartPos, EndPos );            

            m_bHasLOS = (g_CollisionMgr.m_nCollisions == 0);
        } 
        else
        {
            m_bHasLOS = FALSE;
        }
    }

    //
    // Pulse the color.
    //
    {
        m_ColorPhase += 2.0f * DeltaTime;
        if( m_ColorPhase > 2.0f )
            m_ColorPhase = 0.0f;
    }

    //
    // Update the animation state.
    //
    {
        // Bracket displacement.
        if( m_bHasLOS && m_bInViewRange )
        {
            m_AnimState -= 3.0f * DeltaTime;
        }
        else
        {
            m_AnimState += 3.0f * DeltaTime;
        }

        // Text opacity.
        if( (m_AnimState < 0.1f) && m_bInTextRange )
        {
            m_TextAlphaState += DeltaTime * 3.0f;
        }
        else
        {
            m_TextAlphaState -= DeltaTime * 3.0f;
        }

        // Bound them both.
        m_AnimState      = MINMAX( 0.0f, m_AnimState,      1.0f );
        m_TextAlphaState = MINMAX( 0.0f, m_TextAlphaState, 1.0f );

        // Have to map this piecewise function which makes the 
        // last 30% of transition fully opaque onto the alpha.
        m_MaxAlpha = (s32)(255 * ((m_AnimState <= 0.3f) ? 
            (1.0f) : 
            (1.0f - ((m_AnimState - 0.3f) / 0.7f))));
    }
}

//=========================================================================

#ifdef X_EDITOR
s32 lore_object::OnValidateProperties( xstring& ErrorMsg )
{
    s32 nErrors = 0;
    
    return nErrors;
}
#endif

//=========================================================================

xbool lore_object::TestPress( void )
{
    if( m_bActive           &&
        m_bInScanRange      &&
        m_bStandingIn       &&
        (m_bLookingAt || !m_bScanLOS) &&
        m_bHasLOS )
    {

        // Fire the trigger.
        if( m_TriggerGuid != 0 )
        {
            object* pObject = g_ObjMgr.GetObjectByGuid( m_TriggerGuid );
            if( pObject )
            {
                if( pObject->IsKindOf( trigger_ex_object::GetRTTI() ) )
                {
                    trigger_ex_object* pTrigger = (trigger_ex_object*)pObject;
                    pTrigger->OnActivate( TRUE );
                }
            }
        }

        // Play the sound.
        if( m_pScanAudio[ 0 ] != 0 )
        {
            g_AudioMgr.Play( m_pScanAudio );
        }

        // If the auto-off is set then turn the LO off.
        if( m_bAutoOff )
        {
            OnActivate( FALSE );
        }

        return TRUE;
    }

    return FALSE;
}

//=========================================================================

inline xcolor Interpolate( xcolor Color1, xcolor Color2, f32 Percentage )
{
    xcolor TmpColor( 
        (u8)(Color2.R * Percentage + Color1.R * (1.0f - Percentage)),
        (u8)(Color2.G * Percentage + Color1.G * (1.0f - Percentage)),
        (u8)(Color2.B * Percentage + Color1.B * (1.0f - Percentage)),
        (u8)(Color2.A * Percentage + Color1.A * (1.0f - Percentage))
        );
    return TmpColor;
}

//=========================================================================

void lore_object::OnRenderTransparent( void )
{
    CONTEXT( "lore_object::OnRenderTransparent" );

    if( !m_bActive || (m_AnimState == 1.0f) )
    {
        return;
    }

    m_bRendered = TRUE;

    bbox BBox = GetFocusBBox();
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if( !pPlayer )
        return;

    // Get the longest dimension of the bbox.
    f32 LongestDim = 1;
    LongestDim = x_max( LongestDim, x_abs( BBox.Max.GetX() - BBox.Min.GetX() ) );
    LongestDim = x_max( LongestDim, x_abs( BBox.Max.GetY() - BBox.Min.GetY() ) );
    LongestDim = x_max( LongestDim, x_abs( BBox.Max.GetZ() - BBox.Min.GetZ() ) );    
                    
    rect ViewDimensions;

    // If we have LOs in SS then this will have to be looked at! 
    view& rView = pPlayer->GetView();
    rView.GetViewport( ViewDimensions );
    
    // Draw the LO

    //
    // Brackets.
    //
    switch( m_ViewType )
    {
    case SELECT_BRACKETS:
        {
            vector3 P[8];

            P[0].GetX() = BBox.Min.GetX();    P[0].GetY() = BBox.Min.GetY();    P[0].GetZ() = BBox.Min.GetZ();
            P[1].GetX() = BBox.Min.GetX();    P[1].GetY() = BBox.Min.GetY();    P[1].GetZ() = BBox.Max.GetZ();
            P[2].GetX() = BBox.Min.GetX();    P[2].GetY() = BBox.Max.GetY();    P[2].GetZ() = BBox.Min.GetZ();
            P[3].GetX() = BBox.Min.GetX();    P[3].GetY() = BBox.Max.GetY();    P[3].GetZ() = BBox.Max.GetZ();
            P[4].GetX() = BBox.Max.GetX();    P[4].GetY() = BBox.Min.GetY();    P[4].GetZ() = BBox.Min.GetZ();
            P[5].GetX() = BBox.Max.GetX();    P[5].GetY() = BBox.Min.GetY();    P[5].GetZ() = BBox.Max.GetZ();
            P[6].GetX() = BBox.Max.GetX();    P[6].GetY() = BBox.Max.GetY();    P[6].GetZ() = BBox.Min.GetZ();
            P[7].GetX() = BBox.Max.GetX();    P[7].GetY() = BBox.Max.GetY();    P[7].GetZ() = BBox.Max.GetZ();

            f32 MaxX = -10000.0f;
            f32 MinX =  10000.0f;

            f32 MaxY = -10000.0f;
            f32 MinY =  10000.0f;

            s32 i;
            for( i = 0; i < 8; i++ )
            {
                vector3 P2D = rView.PointToScreen( P[i] );
                MaxX = x_max( MaxX, P2D.GetX() );
                MinX = x_min( MinX, P2D.GetX() );

                MaxY = x_max( MaxY, P2D.GetY() );
                MinY = x_min( MinY, P2D.GetY() );
            }

#ifdef TARGET_XBOX
            // dstewart HACK - Because of the wacky screen scaling that goes on the xbox, rView
            // does not report accurate screen coordinates. This is a problem all over the game,
            // but it is too late in the project to try to do a proper fix, so here's another
            // hack. This won't work in split-screen, but we shouldn't have lore objects in
            // split screen anyway.
            s32 OptW, OptH;
            xbox_GetRes( OptW, OptH );
            MinX *= ( (f32)OptW / (f32)ViewDimensions.GetWidth() );
            MaxX *= ( (f32)OptW / (f32)ViewDimensions.GetWidth() );
            MinY *= ( (f32)OptH / (f32)ViewDimensions.GetHeight() );
            MaxY *= ( (f32)OptH / (f32)ViewDimensions.GetHeight() );
#endif

            // Figure out if the player is looking at the LO
            // real quick while we have the appropriate data.
            {
                if( IN_RANGE( MinX, ViewDimensions.GetCenter().X, MaxX ) && 
                    IN_RANGE( MinY, ViewDimensions.GetCenter().Y, MaxY ) )
                {
                    m_bLookingAt = TRUE;
                }
                else
                {
                    m_bLookingAt = FALSE;
                }
            }
            vector3 P3D[4];
            P3D[ 0 ].Set( MinX, MinY, 0.0f );
            P3D[ 1 ].Set( MaxX, MinY, 0.0f );
            P3D[ 2 ].Set( MaxX, MaxY, 0.0f );
            P3D[ 3 ].Set( MinX, MaxY, 0.0f );

#ifndef CONFIG_RETAIL 
            if( g_bDrawLODebug )
            {
                draw_BBox( BBox, XCOLOR_AQUA );
                draw_Begin( DRAW_LINES, DRAW_2D );

                xcolor RectColor = XCOLOR_GREEN;
                draw_Color( RectColor );
                for( i = 0; i < 4; i++ )
                {
                    draw_Vertex( P3D[ i % 4 ] );
                    draw_Vertex( P3D[ (i + 1) % 4 ] );
                }

                draw_End();

                draw_Marker( GetPosition(), XCOLOR_BLUE );
            }
#endif

            vector3 UpOne   ( 0.0f, -1.0f, 0.0f );
            vector3 RightOne( 1.0f,  0.0f, 0.0f );

            /*
            Lore Objects:
            -If the scanner is not available (whether it is missing from inventory as in "Training" or the player is locked into mutant mode as in Amy's level (Black 0-0)) then the Lore should appear RED
            -If the lore is out of range, but currently usable, then the color should be YELLOW
            -If the lore is in range and is usable, then the color should be GREEN
            -These colors should not be editable in properties, and should override anything already set through the property system.
            */

            xbool bHasScanner = pPlayer->GetInventory2().HasItem( INVEN_WEAPON_SCANNER );

            // not in scan range or not looking directly at it etc.
            xcolor Color1 = xcolor( 240, 240, 0, 200 );  // Light Yellow
            xcolor Color2 = xcolor( 128, 128, 0, 200 );  // Dark Yellow

            // if we have the scanner, then do color logic
            if( bHasScanner )
            {
                // if this is an active lore object, change the colors from
                if( m_bInScanRange  &&
                    (m_bLookingAt || !m_bScanLOS) &&
                    m_bHasLOS && m_bStandingIn)
                {
                    Color1 = xcolor( 0,   240, 0, 200 );  // Light Green
                    Color2 = xcolor( 0,   128, 0, 200 );  // Dark Green
                }
            }
            else
            {
                // no scanner, make it red.
                Color1 = xcolor( 255, 0, 0, 200 ); // Light Red
                Color2 = xcolor( 175, 0, 0, 200 ); // Dark Red
            }

            xcolor RenderColor = Interpolate( Color1, Color2, 
                (m_ColorPhase > 1.0f) ? (2.0f - m_ColorPhase) : m_ColorPhase );

            RenderColor.A = x_min( RenderColor.A, m_MaxAlpha );

            vector2 TL( 0.0f, 0.0f );
            vector2 BR( 1.0f, 1.0f );

            f32 BracketSize = x_sqrt( x_min( MaxX - MinX, MaxY - MinY ) ) / 30.0f;
            f32 CornerOffset = 100.0f;

            xbitmap* pBitmap = m_Bracket.GetPointer();
            vector2 WH( (f32)pBitmap->GetWidth(), (f32)pBitmap->GetHeight() );

            WH.Scale( BracketSize );

            draw_Begin( DRAW_SPRITES, DRAW_2D|DRAW_USE_ALPHA|DRAW_TEXTURED | DRAW_NO_ZWRITE );
            draw_SetTexture( *pBitmap );

            vector3 Displacement = m_AnimState * CornerOffset * (-RightOne + UpOne);
            draw_SpriteUV( P3D[ 0 ] + Displacement, WH, TL, BR, RenderColor, 0.0f );

            Displacement = m_AnimState * CornerOffset * (UpOne + RightOne);
            draw_SpriteUV( P3D[ 1 ] + Displacement, WH, TL, BR, RenderColor, -PI / 2.0f );


            Displacement = m_AnimState * CornerOffset * (RightOne - UpOne);
            draw_SpriteUV( P3D[ 2 ] + Displacement, WH, TL, BR, RenderColor, -PI );

            Displacement = m_AnimState * CornerOffset * (-UpOne - RightOne);
            draw_SpriteUV( P3D[ 3 ] + Displacement, WH, TL, BR, RenderColor, 3.0f * -PI / 2.0f );

            draw_End();

            // Draw the Label.
#ifndef X_EDITOR
            /*
            if( (m_DisplayStrTitle >= 0) && (m_DisplayStrTable >= 0) )
            {
                irect LabelPos( (s32)P3D[ 0 ].GetX(), (s32)P3D[ 0 ].GetY(), (s32)P3D[ 1 ].GetX(), (s32)P3D[ 0 ].GetY() );
                xcolor White            = XCOLOR_WHITE;
                                
                const xwchar* pDisplayStr     = (xwchar*)g_StringTableMgr( g_StringMgr.GetString( m_DisplayStrTable ), 
                    g_StringMgr.GetString( m_DisplayStrTitle ) );
                                
                if( x_wstrcmp( pDisplayStr, (const xwchar*)xwstring( "<null>" ) ) == 0 )
                {
                    White                  = XCOLOR_RED;
                    pDisplayStr = (const xwchar*)xwstring( "Lore Object" );
                }
                
                RenderLine( pDisplayStr, LabelPos, (u8)(255 * m_TextAlphaState), White, 1, ui_font::h_center|ui_font::v_bottom, TRUE );
            }
            */

            /* Design no longer wants anything to be displayed
            if( IsTrueLoreObject() )
            {
                irect LabelPos( (s32)P3D[ 0 ].GetX(), (s32)P3D[ 0 ].GetY(), (s32)P3D[ 1 ].GetX(), (s32)P3D[ 0 ].GetY() );
                const xwchar* pDisplayStr = (const xwchar*)xwstring( "Lore Object" );
                xcolor White = XCOLOR_WHITE;

                RenderLine( pDisplayStr, LabelPos, (u8)(255 * m_TextAlphaState), White, 1, ui_font::h_center|ui_font::v_bottom, TRUE );
            }
            */

#endif
        }
        break;
    case SELECT_NONE:
        break;
    default:
        break;
    }
}

//=========================================================================

bbox lore_object::GetLocalBBox( void ) const
{
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();
    
    // if there is geometry, use it's bbox
    if( pRigidGeom )
    {
        return( pRigidGeom->m_BBox );
    }

    return m_FocusBox; 
}

//=========================================================================

bbox lore_object::GetFocusBBox( void ) const
{
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();

    bbox FocusBBox;

    // if we have geometry and want to use it's BBox instead of Size's BBox
    if( pRigidGeom && m_bUseGeometrySize )
    {
        FocusBBox = pRigidGeom->m_BBox;
    }
    else
    {
        FocusBBox = m_FocusBox;
    }

    // transform to world space
    FocusBBox.Transform( GetL2W() );

    return FocusBBox;
}

//=========================================================================

void lore_object::OnEnumProp( prop_enum&  List )
{
    // Get string which contains all available string tables.
    xstring TableString;
    TableString.Clear();

    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

    if( SlotID == SLOT_NULL )
        x_throw( "No Hud Object specified in this project." );

    hud_object& Hud = hud_object::GetSafeType( *g_ObjMgr.GetObjectBySlot( SlotID ) );
    Hud.GetBinaryResourceName(TableString);
    
    object::OnEnumProp      ( List );
         
    List.PropEnumHeader  ( "LoreObject",               "The lore object.", 0 );
    List.PropEnumBool    ( "LoreObject\\Active",       "Does this LO start active?",                       0 );
    List.PropEnumBool    ( "LoreObject\\UseGeomSize",  "Use geometry collision size or Size var?",         0 );
    List.PropEnumInt     ( "LoreObject\\LoreID",       "What is the lore index for this level? (0-n)",     0 );

    List.PropEnumFloat   ( xfs( "%s\\ViewDist",             "LoreObject" ), "Distance the player needs to be within before the effect is drawn.",                 0 );
    List.PropEnumBBox    ( xfs( "%s\\Size",                 "LoreObject" ), "Where this object will appear in the world.",                                        0 );
    List.PropEnumEnum    ( xfs( "%s\\DisplayTable",         "LoreObject" ), (const char*)TableString, "Table for String displayed over LO.",                      0 );
    List.PropEnumString  ( xfs( "%s\\DisplayTitle",         "LoreObject" ), "Title of String displayed over LO.",                                                 0 );
    List.PropEnumFloat   ( xfs( "%s\\TextDist",             "LoreObject" ), "Distance the player needs to be within before the text is drawn.",                   0 );
    List.PropEnumFloat   ( xfs( "%s\\ScanDist",             "LoreObject" ), "Distance the player needs to be within before he can scan this.",                    0 );
    List.PropEnumBool    ( xfs( "%s\\ScanLOS",              "LoreObject" ), "If true, player must look at LO to scan.",                                           0 );
    
    List.PropEnumGuid    ( xfs( "%s\\TriggerGuid",          "LoreObject" ), "The GUID of the trigger that this LO activates.",                                    0 );
    List.PropEnumGuid    ( xfs( "%s\\SpatialGuid",          "LoreObject" ), "The Spatial Trigger to check the volume of",                                         0 );

    List.PropEnumBool    ( xfs( "%s\\TurnOff",              "LoreObject" ), "Deactivate automatically when used?",                                                0 );
    List.PropEnumBool    ( xfs( "%s\\Destroy",              "LoreObject" ), "Destroy automatically when acquired?",                                               0 );
    List.PropEnumBool    ( xfs( "%s\\TriggerOnRestore",     "LoreObject" ), "Do we kick off the trigger (if any) again on restore?",                              0 );    
    
    List.PropEnumEnum    ( xfs( "%s\\ViewType",             "LoreObject" ), "Brackets\0Nothing\0", "What the display should look like for this LO.",              0 );

    // Rigid inst
    // NOTE: This MUST come after geometry so xarray capacities are setup,
    //       but before all other properties so particle, connections, and triangles are initialized
    m_RigidInst.OnEnumProp ( List );
}

//=========================================================================

xbool lore_object::OnProperty( prop_query& rPropQuery )
{                       
	if( object::OnProperty( rPropQuery ) )
	{
        return TRUE;
	}

    // Rigid inst?
    if( rPropQuery.IsSimilarPath( "RenderInst" ) )
    {
        if( m_RigidInst.OnProperty( rPropQuery ) )
        {
            if( rPropQuery.IsVar( "RenderInst\\File" ) )
            {
                if( !rPropQuery.IsRead() )
                {
                    OnTransform(GetL2W());
                }
            }

            return TRUE ;
        }
    }

    s32 iPath = rPropQuery.PushPath( "LoreObject\\" );

    if( rPropQuery.VarBool      ( "Active",             m_bActive             ) ) return TRUE;
    if( rPropQuery.VarBool      ( "UseGeomSize",        m_bUseGeometrySize    ) ) return TRUE;
    if( rPropQuery.VarInt       ( "LoreID",             m_LoreID              ) ) return TRUE;
    
    if( rPropQuery.VarFloat     ( "ViewDist",           m_ViewDist            ) ) return TRUE;
    if( rPropQuery.VarBBox      ( "Size",               m_FocusBox            ) ) return TRUE;

    if( rPropQuery.VarFloat     ( "TextDist",           m_TextDist            ) ) return TRUE;      
    if( rPropQuery.VarFloat     ( "ScanDist",           m_ScanDist            ) ) return TRUE;  
    if( rPropQuery.VarBool      ( "ScanLOS",            m_bScanLOS            ) ) return TRUE;

    if( rPropQuery.VarGUID      ( "TriggerGuid",        m_TriggerGuid         ) ) return TRUE;
    if( rPropQuery.VarGUID      ( "SpatialGuid",        m_SpatialGuid         ) ) return TRUE;

    if( rPropQuery.VarBool      ( "TurnOff",            m_bAutoOff                  ) ) return TRUE;
    if( rPropQuery.VarBool      ( "Destroy",            m_bDestroyAfterAcquired     ) ) return TRUE;
    if( rPropQuery.VarBool      ( "TriggerOnRestore",   m_bActivateTriggerOnRestore ) ) return TRUE;

    if( rPropQuery.IsVar( "ViewType" ) )
    {
        if( rPropQuery.IsRead() )
        {
            switch( m_ViewType )
            {
            default:  // Fall through...
            case SELECT_NONE:   
                rPropQuery.SetVarEnum( "None" );  
                break;
            case SELECT_BRACKETS:
                rPropQuery.SetVarEnum( "Brackets" );  
                break;
            }
            return( TRUE );
        }
        else
        {
            m_ViewType = SELECT_NONE;
            const char* pString = rPropQuery.GetVarEnum();
            if( x_stricmp( pString, "None"                   ) == 0 )    { m_ViewType = SELECT_NONE;     }
            if( x_stricmp( pString, "Brackets"               ) == 0 )    { m_ViewType = SELECT_BRACKETS; }

            return( TRUE );
        }
    }

    if( rPropQuery.IsVar( "DisplayTable" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if( m_DisplayStrTable >= 0 )
                rPropQuery.SetVarEnum( g_StringMgr.GetString( m_DisplayStrTable ) );
            else
                rPropQuery.SetVarEnum( "" );
        }
        else
        {
            m_DisplayStrTable = g_StringMgr.Add( rPropQuery.GetVarEnum() );
        }

        return TRUE;
    }

    if( rPropQuery.IsVar( "DisplayTitle" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if( m_DisplayStrTitle >= 0 )
            {
                const char* pString = g_StringMgr.GetString( m_DisplayStrTitle );
                rPropQuery.SetVarString( pString, x_strlen( pString )+1 );
            }
            else
            {
                rPropQuery.SetVarString( "", 1 );
            }
        }
        else
        {
            m_DisplayStrTitle = g_StringMgr.Add( rPropQuery.GetVarString() );
        }

        return TRUE;
    }
    
    rPropQuery.PopPath( iPath );

    return FALSE;   
}

//=============================================================================

xbool lore_object::GetColDetails( s32 Key, detail_tri& Tri )
{
    if( Key == -1 )
        return( FALSE );

    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();
    if( !pRigidGeom )
        return( FALSE );

    if( !pRigidGeom->m_Collision.nHighClusters )
        return( FALSE );

    return RigidGeom_GetColDetails( pRigidGeom,
        & GetL2W(),
        m_RigidInst.GetColorTable(),
        Key,
        Tri );
}

//=============================================================================

const matrix4* lore_object::GetBoneL2Ws( void )
{
    // Just 1 bone in a play surface
    return &GetL2W() ;
}

//=============================================================================

void lore_object::OnPolyCacheGather( void )
{
    RigidGeom_GatherToPolyCache( GetGuid(), 
        GetBBox(), 
        m_RigidInst.GetLODMask( U16_MAX ), 
        &GetL2W(), 
        m_RigidInst.GetRigidGeom() );
}


//=============================================================================
#ifndef X_RETAIL
void lore_object::OnColRender( xbool bRenderHigh )
{
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();

    if( pRigidGeom )
    {
        RigidGeom_RenderCollision( &GetL2W(),
            m_RigidInst.GetRigidGeom(),
            bRenderHigh,
            m_RigidInst.GetLODMask( U16_MAX ) );
    }
}
#endif // X_RETAIL

//=============================================================================

void lore_object::OnColCheck( void )
{
    CONTEXT("lore_object::DoColCheck");    
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();

    /*
    RigidGeom_ApplyCollision( GetGuid(), 
        GetBBox(),
        m_RigidInst.GetLODMask( U16_MAX ),
        &GetL2W(),
        pRigidGeom );
        */

    bbox BBox = m_FocusBox;

    if( pRigidGeom )
    {
        BBox = pRigidGeom->m_BBox ;        
    }

    // put BBox in worldspace
    BBox.Transform( GetL2W() );

    // Apply
    g_CollisionMgr.StartApply( GetGuid() ) ;
    g_CollisionMgr.ApplyAABBox( BBox ) ;
    g_CollisionMgr.EndApply() ;
}

//==============================================================================

void lore_object::OnTransform( const matrix4& L2W )
{
#ifdef X_EDITOR
    matrix4 CleanM = L2W;
    CleanM.CleanRotation();
    object::OnTransform(CleanM);
#else
    object::OnTransform(L2W);
#endif
}

//=============================================================================

xbool s_bUseTestMap = FALSE;
s32 s_ScannerTestMap = 1000;

void lore_object::OnActivate( xbool Flag )
{
    // if this is telling us to turn off and we're not supposed to, return
    if( (!Flag) && (!m_bAutoOff) )
    {
        return;
    }

    m_bActive = Flag;
}
//=============================================================================

void lore_object::OnAcquire( xbool bIsRestoring )
{
#ifndef X_EDITOR

    // deactivate, item acquired
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if( !pPlayer )
        return;

    const xwchar* pString;

    s32 LevelID = g_ActiveConfig.GetLevelID();

    // for testing
    if( s_bUseTestMap )
    {
        LevelID = s_ScannerTestMap;
    }

    if( !bIsRestoring )
    {
        player_profile& Profile = g_StateMgr.GetActiveProfile( g_StateMgr.GetProfileListIndex(0) );
        s32 VaultIndex = -1;
        g_LoreList.GetVaultByMapID( LevelID, VaultIndex );

        // this is a true lore object
        if( IsTrueLoreObject() )
        {
            lore_vault* pVault = g_LoreList.GetVaultByMapID( LevelID, VaultIndex );

            s32 LoreID = g_LoreList.GetLoreIDByVault( pVault, GetLoreID() );

            // if we have a real ID for this lore object, get it's name from the stringbin
            if( LoreID == -1 )
            {
                pString = (const xwchar*)xwstring( "IDS_LORE_FULL_NAME_nn not set in xxx_lore_strings table!!" );
            }
            else
            {
                pString = g_LoreList.GetLoreName( LoreID );
            }

            // set up message and sound and send to player
            MsgMgr.Message( MSG_LORE_ITEM_ACQUIRED, pPlayer->net_GetSlot(), (s32)pString );

            // if this has been acquired, don't acquire again.
            if( !Profile.GetLoreAcquired( VaultIndex, GetLoreID() ) )
            {
                g_StateMgr.SetLoreAcquired( LevelID, GetLoreID() );            

                if( ( g_StateMgr.GetTotalLoreAcquired() % 5 ) == 0 )
                {
                    // set up message and sound and send to player
                    g_AudioMgr.Play("HUD_Text_Alert",TRUE);
                    MsgMgr.Message( MSG_SECRET_UNLOCKED, pPlayer->net_GetSlot() );
                }
            }
        }
        else
        {
            // non-lore object, do any special object acquisition stuff here.
            pString = g_StringTableMgr( "lore_ingame", xfs("IDS_NONLORE_%d", GetLoreID()) );

            // not found
            if( x_wstrcmp( pString, (const xwchar*)xwstring( "<null>" ) ) == 0 )
            {
                pString = (const xwchar*)xwstring( xfs("IDS_NONLORE_%d not set in xxx_lore_strings table!!",GetLoreID()) );
            }

            // set up message and sound and send to player
            MsgMgr.Message( MSG_STRING, pPlayer->net_GetSlot(), (s32)(pString) );            
        }
    }

#endif

    // are we active?
    if( m_bActive )
    {
        xbool bShouldFireTrigger = TRUE;

        // if this is a restore, make sure we are supposed to fire the trigger.
        if( bIsRestoring )
        {
            bShouldFireTrigger = m_bActivateTriggerOnRestore;
        }

        // Fire the trigger.
        if( m_TriggerGuid != 0 && bShouldFireTrigger )
        {
            object* pObject = g_ObjMgr.GetObjectByGuid( m_TriggerGuid );
            if( pObject )
            {
                if( pObject->IsKindOf( trigger_ex_object::GetRTTI() ) )
                {
                    trigger_ex_object* pTrigger = (trigger_ex_object*)pObject;
                    pTrigger->OnActivate( TRUE );
                }
            }
        }

        if( m_bDestroyAfterAcquired )
        {            
            g_ObjMgr.DestroyObject( GetGuid() );
        }        
    }

    // turn this puppy off if it's supposed to be turned off
    OnActivate(FALSE);
}

//=============================================================================

xbool lore_object::HasFocus( void )
{
    return m_bLookingAt;
}                 

//=============================================================================
