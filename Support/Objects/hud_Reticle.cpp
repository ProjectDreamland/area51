//==============================================================================
//
//  hud_Reticle.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================

#include "hud_Reticle.hpp"
#include "HudObject.hpp"
#include "x_debug.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "characters\Character.hpp"

#ifndef X_EDITOR 
#include "NetGhost.hpp"
#endif

//==============================================================================
// STORAGE
//==============================================================================

xcolor  g_ReticleSmlColor           ( 134, 255, 255, 255 );
xcolor  g_ReticleTargetColor        ( 125,   0,   0, 229 );

xcolor  g_ReticleMutantTargetColor  ( 255,  255,  66, 255 );
xcolor  g_ReticleMutantColor        ( 255,   55,  66, 255 );

//==============================================================================
// FUNCTIONS
//==============================================================================

hud_reticle::hud_reticle( void )
{
    m_LastTarget = 0;
    m_LockonTime = 0.0f;
}

//==============================================================================

void hud_reticle::OnRender( player* pPlayer )
{    
    // Get the screen center.
    rect m_ViewDimensions;
    view& rView = ((player*)pPlayer)->GetView();
    rView.GetViewport( m_ViewDimensions );                                   

    f32 CenterX = m_XPos;
    f32 CenterY = m_YPos;

    new_weapon* pWeapon = pPlayer->GetCurrentWeaponPtr();
    if( pWeapon == NULL )
        return;

    if( !pWeapon->ShouldDrawReticle() )
    {
        return;
    }

    f32 MainWidth  = 0.0f;
    f32 MainHeight = 0.0f;

    xcolor tempSmallColor = g_ReticleSmlColor;
    vector2 UV0( 0.0f, 0.0f );
    vector2 UV1( 1.0f, 1.0f );

    guid    EnemyGuid = pPlayer->GetEnemyOnReticle();
    xcolor  Color = g_ReticleSmlColor;

    // Only display if the target exists, is of a valid type, and is alive.
#ifndef X_EDITOR
    if( !GameMgr.IsGameMultiplayer() )
#endif
    {
        if( EnemyGuid )
        {
            object* pObject = g_ObjMgr.GetObjectByGuid( EnemyGuid );
           
            if( pObject && pObject->IsKindOf( actor::GetRTTI() ) )
            {
                actor& ActorObj =  actor::GetSafeType( *pObject );
                if( !ActorObj.IsDead() )
                {
                    Color = g_ReticleTargetColor;
                }
            }
        }
        else
        {
            guid FriendlyGuid = pPlayer->GetFriendlyOnReticle();

            if( FriendlyGuid )
            {
                object* pObject = g_ObjMgr.GetObjectByGuid( FriendlyGuid );

                if( pObject && pObject->IsKindOf( actor::GetRTTI() ) )
                {
                    actor& ActorObj =  actor::GetSafeType( *pObject );
                    if( !ActorObj.IsDead() )
                    {
                        // KSS -- Do anything for friendly targetting here
                    }
                }
            }
        }
    }

    tempSmallColor = Color;

    xcolor PulseColor( Color );
    vector3 Center    ( CenterX, CenterY, 0.0f );

    // If we're zoomed we don't want anything but a small cross.
    if( pWeapon->IsZoomEnabled() )
    {
        vector3 P0 = Center;
        vector3 P1 = Center;
        vector3 P2 = Center;
        vector3 P3 = Center;

        vector3 P4 = Center;
        vector3 P5 = Center;
        vector3 P6 = Center;
        vector3 P7 = Center;

        f32 Inner = 3.0f;
        f32 Outer = 6.0f;
        P0.GetX() -= Inner;
        P1.GetX() -= Outer;
        P2.GetX() += Inner;
        P3.GetX() += Outer;

        P4.GetY() -= Inner;
        P5.GetY() -= Outer;
        P6.GetY() += Inner;
        P7.GetY() += Outer;
        
        draw_Begin( DRAW_LINES, DRAW_2D | DRAW_NO_ZBUFFER );
        draw_Color( Color );
        draw_Vertex( P0 );
        draw_Vertex( P1 );
        draw_Vertex( P2 );
        draw_Vertex( P3 );
        draw_Vertex( P4 );
        draw_Vertex( P5 );
        draw_Vertex( P6 );
        draw_Vertex( P7 );
        draw_End();
        return;
    }

    //
    // Draw the dot in the center.
    //
    {
        // BEGIN DRAW
        draw_Begin( DRAW_POINTS, DRAW_2D | DRAW_NO_ZBUFFER );

        // check for pulsing
        if( m_bPulsing )
        {
            draw_Color( PulseColor );
        }
        else
        {
            draw_Color( Color );
        }

        draw_Vertex( Center );
        draw_End();
        // END DRAW
    }

    //
    // Draw the reticle circle.
    //
    {
        // BEGIN DRAW
        draw_Begin( DRAW_LINES, DRAW_2D | DRAW_NO_ZBUFFER );

        // Check for pulsing.
        if( m_bPulsing )
        {
            PulseColor.A = (u8)(((f32)PulseColor.A / 255) * hud_object::m_PulseAlpha);
            draw_Color( PulseColor );
        }
        else
        {
            draw_Color( Color );
        }

    #ifdef nmreed // this draws the enemy's shure-hit bbox
        object* pEnemyObj = g_ObjMgr.GetObjectByGuid( EnemyGuid );
        if ( pEnemyObj )
        {
            // draw the enemy box
            bbox ScreenEnemyBBox( pEnemyObj->GetScreenBBox( pPlayer->GetView() ) );
            vector3 Size( ScreenEnemyBBox.GetSize() );
            extern f32 g_EnemyShootBBoxPct;
            ScreenEnemyBBox.Inflate( (Size.X * g_EnemyShootBBoxPct) - Size.X, 0.0f, 0.0f );
            vector3 UL( ScreenEnemyBBox.Min );
            vector3 BR( ScreenEnemyBBox.Max );
            vector3 BL( UL.X, BR.Y, UL.Z );
            vector3 UR( BR.X, UL.Y, UL.Z );

            draw_Color( XCOLOR_BLUE );
            draw_Vertex( UL );
            draw_Vertex( UR );

            draw_Vertex( BR );
            draw_Vertex( UR );

            draw_Vertex( BR );
            draw_Vertex( BL );

            draw_Vertex( BL );
            draw_Vertex( UL );
        }
    #endif
 
        draw_End();
        // END DRAW
    }
    
    xbitmap* pMainReticle = pWeapon->GetCenterReticleBmp();
    if( pMainReticle  )
    {
        // BEGIN DRAW
        draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA|DRAW_TEXTURED|DRAW_2D|DRAW_NO_ZBUFFER );

        // Since the bitmap is going to be centered we want to get half it length and width.
        MainWidth  = (f32)pMainReticle->GetWidth()/2.0f;
        MainHeight = (f32)pMainReticle->GetHeight()/2.0f;

        vector3 ReticlePosRelative (  CenterX-MainWidth,    CenterY-MainHeight, 0.0f );
    
        draw_SetTexture( *pMainReticle );

        draw_DisableBilinear();

#ifndef X_EDITOR
        if( !GameMgr.IsGameMultiplayer() )
#endif
        {
            guid EnemyGuid = pPlayer->GetEnemyOnReticle();
            if( EnemyGuid != NULL_GUID )
            {
                object* pEnemyObj = g_ObjMgr.GetObjectByGuid( EnemyGuid );
            
                if( pEnemyObj )
                {
                    vector3 ReticlePosMin( ReticlePosRelative );
                    vector3 ReticlePosMax( CenterX+MainWidth, CenterY+MainHeight, 0.0f );

                    bbox ReticleRect( ReticlePosMin, ReticlePosMax);
                    bbox ScreenEnemyBBox( pEnemyObj->GetScreenBBox( pPlayer->GetView() ) );

                    if( ReticleRect.Intersect( ScreenEnemyBBox ) )
                        tempSmallColor = g_ReticleTargetColor;
                }
            }
        }

        // check for pulsing
        if( m_bPulsing )
        {
            tempSmallColor.A = (u8)(((f32)tempSmallColor.A / 255) * hud_object::m_PulseAlpha);
        }
   
        draw_SpriteUV( ReticlePosRelative , vector2((f32)pMainReticle->GetWidth(), 
                    (f32)pMainReticle->GetHeight()), UV0, UV1, tempSmallColor );

        draw_End();
        draw_EnableBilinear();
        // END DRAW
    }
    
    xbitmap* pEdgeReticle       = pWeapon->GetEdgeReticleBmp();
    s32      CenterPixelOffset  = (s32)pWeapon->GetCenterPixelOffset();

    // aharp HACK - For E3 to draw mutant reticle.
    if( pPlayer->GetCurrentWeapon2() == INVEN_WEAPON_MUTATION )
    {
        rhandle<xbitmap> m_ScreenEdgeBmp;
        m_ScreenEdgeBmp.SetName(PRELOAD_FILE("HUD_reticle_mutation.xbmp"));
        pEdgeReticle = m_ScreenEdgeBmp.GetPointer();
    
        guid    EnemyGuid = pPlayer->GetEnemyOnReticle();

        xbool bMultiplayer = FALSE;

#ifndef X_EDITOR
        bMultiplayer = GameMgr.IsGameMultiplayer();
#endif


        xcolor  Color     =  (EnemyGuid && !bMultiplayer) ? g_ReticleMutantTargetColor : g_ReticleMutantColor;

        // BEGIN DRAW
        draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA|DRAW_TEXTURED|DRAW_2D|DRAW_BLEND_ADD|DRAW_NO_ZBUFFER );


        draw_SetTexture( *pEdgeReticle );

        f32 BmpWidth  = (f32)32.0f;//pEdgeReticle->GetWidth();
        f32 BmpHeight = (f32)32.0f;//pEdgeReticle->GetHeight();

        vector3 ScreenCenter( CenterX, CenterY, 0 );
        vector3 RenderPos;

        vector2 UV2( 1.0f, 0.0f );
        vector2 UV3( 0.0f, 1.0f );

        // Top Left.
        RenderPos = ScreenCenter;
        RenderPos.GetX() -= BmpWidth  / 2.0f;
        RenderPos.GetY() -= BmpHeight / 2.0f;
        draw_SpriteUV( RenderPos, vector2(BmpWidth, BmpHeight), UV0, UV1, Color, R_0 );

        // Bottom Left.
        RenderPos = ScreenCenter;
        RenderPos.GetX() -= BmpWidth  / 2.0f;
        RenderPos.GetY() += BmpHeight / 2.0f;
        draw_SpriteUV( RenderPos, vector2(BmpWidth, BmpHeight), UV3, UV2, Color, R_0 );

        // Top Right.
        RenderPos = ScreenCenter;
        RenderPos.GetX() += BmpWidth  / 2.0f;
        RenderPos.GetY() -= BmpHeight / 2.0f;
        draw_SpriteUV( RenderPos, vector2(BmpWidth, BmpHeight), UV2, UV3, Color, R_0 );

        // Bottom Right.
        RenderPos = ScreenCenter;
        RenderPos.GetX() += BmpWidth  / 2.0f;
        RenderPos.GetY() += BmpHeight / 2.0f;
        draw_SpriteUV( RenderPos, vector2(BmpWidth, BmpHeight), UV1, UV0, Color, R_0 );


        draw_End();
        // END DRAW
        return;
    }
    
    // Draw the second reticle.
    if( pEdgeReticle == NULL )
    {
        return;
    }

    f32 DegWidth  = (f32)pEdgeReticle->GetWidth()/2.0f;
    f32 DegHeight = (f32)pEdgeReticle->GetHeight()/2.0f;


    f32 fSpace = (40.0f * pPlayer->GetAimDegradation() );

    vector3 DegPosRelative1;
    vector3 DegPosRelative2;
    vector3 DegPosRelative3;
    vector3 DegPosRelative4;

    radian  DegRot1;
    radian  DegRot2;
    radian  DegRot3;
    radian  DegRot4;

    //         |
    //      ___2___
    //     |   |   |
    //     |   |   |
    // ---3|---C---|1---
    //     |   |   |
    //     |___|___|
    //         4
    //         |
    
    if( (pPlayer->GetCurrentWeapon2() == INVEN_WEAPON_SMP ) )
    {
        DegPosRelative1 (  (CenterX+(MainWidth+CenterPixelOffset)+fSpace), CenterY, 0.0f );
        DegPosRelative2 (  (CenterX),                   CenterY-(MainWidth+CenterPixelOffset)-fSpace, 0.0f );    
        DegPosRelative3 (  (CenterX-(MainWidth+CenterPixelOffset)-fSpace),  CenterY, 0.0f );    
        DegPosRelative4 (  (CenterX),                   CenterY+(MainWidth+CenterPixelOffset)+fSpace, 0.0f );

        DegRot1 = RADIAN(270);
        DegRot2 = RADIAN(0);
        DegRot3 = RADIAN(90);
        DegRot4 = RADIAN(180);

    }
    //         |
    //     2_______1
    //     |   |   |
    //     |   |   |
    // ----|---C---|----
    //     |   |   |
    //     |___|___|
    //     3   |   4
    //         |
    else
    {
        // This piece is going to be moving diagonal so drop the number of pixel that the degradation pieces
        // will be by half so that make it 1/4.
        MainWidth  = MainWidth/2.0f;
        MainHeight = MainHeight/2.0f;

        DegPosRelative1 (  (CenterX+(MainWidth+CenterPixelOffset)+fSpace),   (CenterY-(MainHeight+CenterPixelOffset)-fSpace), 0.0f );
        DegPosRelative4 (  (CenterX-(MainWidth+CenterPixelOffset)-fSpace),   (CenterY-(MainHeight+CenterPixelOffset)-fSpace), 0.0f );    
        DegPosRelative3 (  (CenterX-(MainWidth+CenterPixelOffset)-fSpace),   (CenterY+(MainHeight+CenterPixelOffset)+fSpace), 0.0f );    
        DegPosRelative2 (  (CenterX+(MainWidth+CenterPixelOffset)+fSpace),   (CenterY+(MainHeight+CenterPixelOffset)+fSpace), 0.0f );

        DegRot1 = RADIAN(315);
        DegRot2 = RADIAN(225);
        DegRot3 = RADIAN(135);
        DegRot4 = RADIAN(45);
    }
    
    {
        // BEGIN DRAW
        draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA|DRAW_TEXTURED|DRAW_2D|DRAW_NO_ZBUFFER );

        draw_SetTexture( *pEdgeReticle );

        DegWidth  = (f32)pEdgeReticle->GetWidth();
        DegHeight = (f32)pEdgeReticle->GetHeight();

        draw_SpriteUV( DegPosRelative1, vector2(DegWidth, DegHeight), UV0, UV1, tempSmallColor, DegRot1 );

        draw_SpriteUV( DegPosRelative2, vector2(DegWidth, DegHeight), UV0, UV1, tempSmallColor, DegRot2 );

        draw_SpriteUV( DegPosRelative3, vector2(DegWidth, DegHeight), UV0, UV1, tempSmallColor, DegRot3 );

        draw_SpriteUV( DegPosRelative4, vector2(DegWidth, DegHeight), UV0, UV1, tempSmallColor, DegRot4 );

        draw_End();
        // END DRAW
    }   
}

//==============================================================================

void hud_reticle::OnAdvanceLogic( player* pPlayer, f32 DeltaTime )
{
    (void)pPlayer;
    (void)DeltaTime;
}

//==============================================================================

xbool hud_reticle::OnProperty( prop_query& rPropQuery )
{
    (void)rPropQuery;
    return FALSE;
}

//==============================================================================

void hud_reticle::OnEnumProp( prop_enum&  List )
{
    (void)List;
}
