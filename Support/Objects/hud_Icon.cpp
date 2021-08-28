//==============================================================================
//
//  hud_Icon.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "hud_Icon.hpp"
#include "HudObject.hpp"
#include "Ui\ui_manager.hpp"
#include "Ui\ui_font.hpp"
#include "GameLib\RenderContext.hpp"

#ifndef X_EDITOR
#include "StateMgr\StateMgr.hpp"
#endif

//==============================================================================
// FUNCTIONS
//==============================================================================

hud_icon::hud_icon( void ) 
{
    Init();
}

//==============================================================================

void hud_icon::Init( void )
{
    m_NumActiveIcons    = 0;
}

//==============================================================================

void hud_icon::AddIcon(         icon_type       IconType, 
                        const   vector3&        FocusPosition,
                        const   vector3&        RenderPosition,
                                xbool           bOccludes,
                                xbool           bAlignToBottom,
                                gutter_type     GutterType, 
                                xcolor          Color,
                        const   xwchar*         pCharName,
                                xbool           Pulsing, 
                                xbool           Distance, 
                                f32             Opacity,
                                f32             IconFadeDist,
                                f32             TextFadeDist
                       )
{
    if( !IN_RANGE( 0, m_NumActiveIcons, NUM_ICONS - 1 ) )
    {
        return;
    }

#ifndef X_EDITOR
    else if( g_StateMgr.GetState() != SM_PLAYING_GAME )
    {
        return;
    }
#endif

    icon_inf TempIcon;

    TempIcon.IconType       = IconType;
    TempIcon.FocusPosition  = FocusPosition;
    TempIcon.RenderPosition = RenderPosition;
    TempIcon.bOccludes      = bOccludes;
    TempIcon.bAlignToBottom = bAlignToBottom;
    TempIcon.GutterType     = GutterType;
    TempIcon.Color          = Color;
    TempIcon.Pulsing        = Pulsing;
    TempIcon.Distance       = Distance;
    TempIcon.IconFadeDist   = IconFadeDist;
    TempIcon.TextFadeDist   = TextFadeDist;
    TempIcon.Opacity        = Opacity;

    // Copy the label over.
    if( pCharName )     x_wstrcpy( TempIcon.Label, pCharName );
    else                TempIcon.Label[0] = '\0';

    m_Icons[ m_NumActiveIcons ] = TempIcon;
    m_NumActiveIcons++;

    ASSERTS( IN_RANGE( 0, m_NumActiveIcons, NUM_ICONS - 1 ), "Too many icons!" );
}

//==============================================================================

void hud_icon::RenderIcon( player* pPlayer, icon_inf& Icon )
{
    f32 Margin     = 0.85f;
    f32 Hud_Width  = m_ViewDimensions.GetWidth()  / 2.0f;
    f32 Hud_Height = m_ViewDimensions.GetHeight() / 2.0f;

    view& rView = pPlayer->GetView();
    rView.GetViewport( m_ViewDimensions );

    vector3 RenderWorldPos = Icon.RenderPosition;
    vector3 EyesWorldPos   = pPlayer->GetEyesPosition();
    vector3 TargetWorldPos = Icon.FocusPosition;

    vector3 WorldToTarget  = TargetWorldPos - EyesWorldPos;
    f32     WorldDist      = WorldToTarget.Length();

    //
    // If icon can be occluded, check for visibility.
    //
    if( Icon.bOccludes )
    {
        g_CollisionMgr.LineOfSightSetup( pPlayer->GetGuid(), EyesWorldPos, TargetWorldPos );
        // Only need one collision to block LOS.
        g_CollisionMgr.SetMaxCollisions(1);

        // Perform collision
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
            object::ATTR_COLLIDABLE, 
            (object::object_attr)(object::ATTR_COLLISION_PERMEABLE|object::ATTR_LIVING) );

        if( g_CollisionMgr.m_nCollisions > 0 )
        {
            return;
        }
    }

    vector3 FocusScreenPos  ( rView.PointToScreen( TargetWorldPos ) );
    vector3 RenderScreenPos ( rView.PointToScreen( RenderWorldPos ) );

    vector3 ScreenCenterPos ( m_XPos, m_YPos, 0.0f );



    // Find the dir to the icon from the screen center.
    vector3 DirToIcon = RenderScreenPos - ScreenCenterPos;
    DirToIcon.GetZ() = 0.0f;
    DirToIcon.Normalize();

    xbool   InRegion  = FALSE;

    //
    // Figure out what the icon's position on the screen should be
    // based on the bounding type chosen.
    // 
    switch( Icon.GutterType )
    {
    case GUTTER_ELLIPSE:
        // Check to see if the point is inside of the ellipse, 
        // we will scale the point and do point in sphere check. 
        {
            vector3 EllipsePos( ((DirToIcon.GetX() * Hud_Width  * Margin) + m_XPos),
                ((DirToIcon.GetY() * Hud_Height * Margin) + m_YPos),
                0.0f );

            // Which is the smallest axis?
            if( Hud_Width > Hud_Height )
            {
                // Y Axis.
                f32 Sx = Hud_Height/Hud_Width;
                vector3 InterPoint( RenderScreenPos );

                // Bring it in to normal coordinate system.
                InterPoint.GetX() -= Hud_Width;
                InterPoint.GetY() -= Hud_Height;
                InterPoint.GetZ()  = 0.0f;
                InterPoint.GetX() *= Sx;

                f32 LenghtSqrd = InterPoint.LengthSquared();
                if( LenghtSqrd <= (Hud_Height*Hud_Height*Margin*Margin) && 
                    (RenderScreenPos.GetZ() > 0.0f) )
                {
                    InRegion = TRUE;
                }
                else
                {
                    RenderScreenPos = EllipsePos;
                }
            }
            else
            {
                // X Axis.
                f32 Sx = Hud_Width/Hud_Height;
                vector3 InterPoint( RenderScreenPos );
                InterPoint.GetZ() = 0.0f;

                // Bring it in to normal coordinate system.
                InterPoint.GetX() -= Hud_Width;
                InterPoint.GetY() -= Hud_Width;
                InterPoint.GetZ()  = 0.0f;
                InterPoint.GetY() *= Sx;

                f32 LengthSqrd = InterPoint.LengthSquared();
                if( LengthSqrd <= (Hud_Width*Hud_Width*Margin*Margin) && 
                    (RenderScreenPos.GetZ() > 0.0f) )
                {
                    InRegion = TRUE;
                }
                else
                {
                    RenderScreenPos = EllipsePos;
                }
            }
        }
        break;

    case GUTTER_RECTANGLE:
        {
            // Is it outside of a box?

            if( (x_abs( RenderScreenPos.GetX() - m_XPos) > (Hud_Width  * Margin))  ||
                (x_abs( RenderScreenPos.GetY() - m_YPos) > (Hud_Height * Margin))  ||
                (RenderScreenPos.GetZ() < 0.0f) )
            {
                vector3 ScreenPoints[4] = { vector3( (m_XPos - Hud_Width * Margin), (m_YPos - Hud_Height * Margin), 0.0f ),
                    vector3( (m_XPos + Hud_Width * Margin), (m_YPos - Hud_Height * Margin), 0.0f ),
                    vector3( (m_XPos + Hud_Width * Margin), (m_YPos + Hud_Height * Margin), 0.0f ),
                    vector3( (m_XPos - Hud_Width * Margin), (m_YPos + Hud_Height * Margin), 0.0f ) };


                vector3 IconPos = RenderScreenPos;

                // Make sure that it's outside the box if it's behind us.
                IconPos.GetZ() = 0.0f;

                vector3 DirVec = IconPos - ScreenCenterPos;
                IconPos = (((Hud_Width * 3.0f) / DirVec.Length()) * DirVec) + ScreenCenterPos;

                /*
                // Debug code.
                draw_Begin( DRAW_LINES, DRAW_2D );
                xcolor DrawColor = XCOLOR_GREEN;
                draw_Color( DrawColor );

                draw_Vertex( ScreenPoints[ 0 ] );
                draw_Vertex( ScreenPoints[ 1 ] );

                DrawColor = XCOLOR_YELLOW;
                draw_Color( DrawColor );

                draw_Vertex( ScreenPoints[ 1 ] );
                draw_Vertex( ScreenPoints[ 2 ] );

                DrawColor = XCOLOR_BLUE;
                draw_Color( DrawColor );

                draw_Vertex( ScreenPoints[ 2 ] );
                draw_Vertex( ScreenPoints[ 3 ] );

                DrawColor = XCOLOR_PURPLE;
                draw_Color( DrawColor );

                draw_Vertex( ScreenPoints[ 3 ] );
                draw_Vertex( ScreenPoints[ 0 ] );

                DrawColor = XCOLOR_RED;
                draw_Color( DrawColor );

                draw_Vertex( ScreenCenterPos );
                draw_Vertex( IconPos );

                draw_End();
                */

                for( s32 i = 0; i < 4; i++ )
                {
                    vector3 ClosestPoint1, ClosestPoint2;

                    // Find the intersection point.
                    x_ClosestPtsOnLineSegs( ScreenPoints[ i ], ScreenPoints[ (i + 1) % 4 ], 
                        ScreenCenterPos, IconPos, 
                        ClosestPoint1, ClosestPoint2 );

                    // It should intersect at least one of the lines.
                    if( (ClosestPoint1 - ClosestPoint2).LengthSquared() < 0.1f )
                    {
                        RenderScreenPos = ClosestPoint1;
                        break;
                    }
                }
            }
            else
            {
                InRegion = TRUE;
            }
        }
        break;

    case GUTTER_NONE:
        InRegion = TRUE;

        // So that it doesn't render behind the player.
        if( RenderScreenPos.GetZ() < 0.0f )
        {
            return;
        }
        break;

    default:
        break;
    }

    RenderScreenPos.GetZ() = 0.0f;
    FocusScreenPos.GetZ() = 0.0f;
    f32 ScreenDist = (FocusScreenPos - ScreenCenterPos).Length();

    //
    // Draw the icon.
    //
    {
        //
        // Get rotation.
        //
        radian  BitmapRotation = R_0;

        if( !InRegion )
        {
            BitmapRotation = v3_AngleBetween( vector3( 0.0f,             1.0f,             0.0f ), 
                vector3( DirToIcon.GetX(), DirToIcon.GetY(), 0.0f ) );

            // Since angle between returns the smallest angle only, 
            // we need to flip it on the left side of the screen.
            if( DirToIcon.GetX() < 0.0f ) 
                BitmapRotation *= -1.0f;
        }

        m_ScreenEdgeBmp.SetName( PRELOAD_FILE("Hud_icon.xbmp") );

        xbitmap* pBitmap = m_ScreenEdgeBmp.GetPointer();
        if( pBitmap == NULL )
        {
            return;
        }

        f32 IconMargin = 0.01f;

        // This indexes into the icon bitmap to get the appropriate coordinates for the icon.
        f32 x1 = ((f32)(((Icon.IconType) % 4) + 0.0f) / 4.0f) + IconMargin;
        f32 y1 = (((Icon.IconType / 4) + 0) / 4.0f)           + IconMargin;

        f32 x2 = x1 + 0.25f - (2.0f * IconMargin);
        f32 y2 = y1 + 0.25f - (2.0f * IconMargin);
                           
        vector2 WH( (f32)pBitmap->GetWidth() / 4.0f, (f32)pBitmap->GetHeight() / 4.0f );


        if( (Icon.IconType == ICON_FLAG_INNER) || 
            (Icon.IconType == ICON_FLAG_OUTER) )
        {
            x1 = (f32)((Icon.IconType - ICON_FLAG_INNER) * 0.50f) + IconMargin;
            x2 = x1 + 0.5f - (2.0f * IconMargin);

            y1 = 0.5f + IconMargin;
            y2 = 1.0f - IconMargin;

            WH *= 2.0f;
        }

        vector2 UV0(  x1, y1 );
        vector2 UV1(  x2, y2 );

        xcolor  Color            = Icon.Color;
        Color.A                  = (s32)(GetOpacity( ScreenDist, Icon.Opacity, Icon.IconFadeDist ) * 255); 

        if( Icon.bAlignToBottom )
        {
            RenderScreenPos.GetY() -= 8.0f;
        }

        // Draw the icon.
        draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA|DRAW_TEXTURED|DRAW_2D|DRAW_NO_ZWRITE|DRAW_UV_CLAMP );                
        draw_SetTexture( *pBitmap );
        draw_SpriteUV( RenderScreenPos, WH, UV0, UV1, Color, BitmapRotation ); 
        draw_End();
    }

#ifndef X_EDITOR
    //
    // Render any optional text.
    //
    if( InRegion )
    {
        // Render the label, if any.
        if( ((xwchar*)Icon.Label)[0] != 0 )
        {
            // Draw label to the left.
            xcolor LabelColor( XCOLOR_WHITE );

            f32 FinalOpacity = GetOpacity( ScreenDist, Icon.Opacity, Icon.TextFadeDist ); 

            LabelColor.A = (u8)(FinalOpacity * 255); 

            irect LabelPos(
                (s32)(RenderScreenPos.GetX() - 200),
                (s32)(RenderScreenPos.GetY() -   8),
                (s32)(RenderScreenPos.GetX() -  12),
                (s32)(RenderScreenPos.GetY() + 200)
                );

            xcolor TextColor = XCOLOR_WHITE;
            RenderLine( Icon.Label, LabelPos, LabelColor.A, TextColor, 1, ui_font::h_right|ui_font::v_top, TRUE );
        }

        // Render the distance if enabled.
        if( Icon.Distance )
        {
            // Draw the distance to the right.
            xcolor LabelColor( XCOLOR_WHITE );

            f32 FinalOpacity = GetOpacity( ScreenDist, Icon.Opacity, Icon.TextFadeDist ); 

            LabelColor.A = (u8)(FinalOpacity * 255); 

            irect LabelPos(
                (s32)(RenderScreenPos.GetX() + 12),
                (s32)(RenderScreenPos.GetY() - 8),
                (s32)(RenderScreenPos.GetX() + 200),
                (s32)(RenderScreenPos.GetY() + 200)
                );

            xwstring DistanceStr( xfs( "%.2fm", WorldDist / 100.0f ) );

            xcolor TextColor = XCOLOR_WHITE;
            RenderLine( DistanceStr, LabelPos, LabelColor.A, TextColor, 1, ui_font::h_left|ui_font::v_top, TRUE );
        }
    }
#endif


}

//==============================================================================

void hud_icon::OnRender( player* pPlayer )
{
    xbool   IconRendered[ NUM_ICONS ];
    x_memset( IconRendered, FALSE, NUM_ICONS );

    f32 DistancesSquared[ NUM_ICONS ];

    vector3 PlayerPos = pPlayer->GetEyesPosition();

    // Precompute the squared distances.
    for( s32 i = 0; i < m_NumActiveIcons; i++ )
    {
        DistancesSquared[ i ] = (PlayerPos - m_Icons[ i ].RenderPosition).LengthSquared();
    }

    // Now render them from back to front.
    for( s32 i = 0; i < m_NumActiveIcons; i++ )
    {
        f32 Farthest = -1.0f;
        s32 iIcon = -1;

        // Look for the farthest away icon that hasn't been rendered.
        for( s32 j = 0; j < m_NumActiveIcons; j++ )
        {
            if( IconRendered[ j ] ) continue;

            if( DistancesSquared[ j ] > Farthest )
            {
                iIcon    = j;
                Farthest = DistancesSquared[ j ];
            }
        }

        ASSERT( iIcon != -1 );
        ASSERT( !IconRendered[ iIcon ] );

        RenderIcon( pPlayer, m_Icons[ iIcon ] );
        IconRendered[ iIcon ] = TRUE;
    }
}

//==============================================================================

void hud_icon::OnAdvanceLogic( player* pPlayer, f32 DeltaTime )
{
    (void)pPlayer;
    (void)DeltaTime;
}

//==============================================================================

f32 hud_icon::GetOpacity( f32 DistFromCenter, f32 Opacity, f32 FadeDist )
{
    f32 PercentageDist = (FadeDist - DistFromCenter) / FadeDist;

    if( FadeDist < 0.0f )
    {
        PercentageDist = 1.0f;

    }

    else
    {
        if( PercentageDist < 0.0f )
        {
            PercentageDist = 0.0f;
        }
        PercentageDist *= 1.5f;
        if( PercentageDist > 1.0f ) 
        {
            PercentageDist = 1.0f;
        }
    }

    return Opacity * PercentageDist;
}

//==============================================================================

xbool hud_icon::OnProperty( prop_query& rPropQuery )
{
    if( rPropQuery.IsVar( "Nav Point\\ScreenEdge Bmp" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_ScreenEdgeBmp.GetName(), RESOURCE_NAME_SIZE );
        }
        else            
        {
            const char* pStr = rPropQuery.GetVarExternal();
            m_ScreenEdgeBmp.SetName( pStr );
        }
        return TRUE;
    }    

    if( rPropQuery.VarColor( "Nav Point\\ScreenEdge Color", m_ScreenEdgeColor ) )
        return TRUE;

    if( rPropQuery.IsVar( "Nav Point\\ScreenCenter Bmp" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_ScreenCenterBmp.GetName(), RESOURCE_NAME_SIZE );
        }
        else            
        {
            const char* pStr = rPropQuery.GetVarExternal();
            m_ScreenCenterBmp.SetName( pStr );
        }
        return TRUE;
    }    

    if( rPropQuery.VarColor( "Nav Point\\ScreenCenter Color", m_ScreenCenterColor ) )
        return TRUE;

    if( rPropQuery.VarBool( "Nav Point\\Start Active", m_Active ) )
        return TRUE;

    return FALSE;
}

//==============================================================================

void hud_icon::OnEnumProp( prop_enum&  List )
{
    List.PropEnumHeader  ( "Nav Point", "Point to navigate the player too.", 0 );

    List.PropEnumExternal( "Nav Point\\ScreenEdge Bmp",   "Resource\0xbmp\0", "Bitmap to use when the nav point is outside of the view", PROP_TYPE_MUST_ENUM  );
    List.PropEnumColor   ( "Nav Point\\ScreenEdge Color", "The color for the bitmap that will be draw when the nav point is not in view.", 0 );

    List.PropEnumExternal( "Nav Point\\ScreenCenter Bmp",   "Resource\0xbmp\0", "Bitmap to use when the nav point is inside of the view", PROP_TYPE_MUST_ENUM  );
    List.PropEnumColor   ( "Nav Point\\ScreenCenter Color", "The color for the bitmap that will be draw when the nav point is in view.", 0 );

    List.PropEnumBool    ( "Nav Point\\Start Active", "Do you want this object to start active.", PROP_TYPE_EXPOSE );
}

//==============================================================================
