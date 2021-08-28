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

#include "hud_Sniper.hpp"
#include "WeaponSniper.hpp"
#include "Ui\ui_manager.hpp"
#include "Ui\ui_font.hpp"

#include "NetworkMgr\NetworkMgr.hpp"
#include "NetworkMgr\GameMgr.hpp"

#if defined(TARGET_PS2)
#include "Entropy\PS2\ps2_misc.hpp"
#endif

//==============================================================================
// STORAGE
//==============================================================================

vector3 s_ZoomPos           ( 365.0f, 345.0f, 0.0f );
vector3 s_DistancePos       ( 230.0f, 100.0f, 0.0f );
f32     s_ZoomWidth         = 20.0f;
f32     s_ZoomHeight        = 20.0f;  
xcolor  s_ZoomColor         ( 49, 255, 49, 255 );
xcolor  g_MipColor          (64,64,64,64);
s32     s_TrackerCount      = 5;

f32     g_MipParam1         = 8.0f;
f32     g_MipParam2         = 100.0f;


// aharp HACK These colors appears in hud_Ammo as well.
xcolor  s_ShadowColor       ( 49,  49,  49, 255 );
s32     s_ShadowOffsetX     = 2;
s32     s_ShadowOffsetY     = 2;

s32     g_nFilters          = 3;
f32     g_MipOffset         = 4.0f;
s32     g_MipFn             = 0;

f32     g_ZoomDistance      = 10000.0f;

#define ALPHA_CHANNEL_MASK      0x00FFFFFF

xcolor hud_sniper::m_SniperHudColor;
xcolor hud_sniper::m_SniperScanLineColor;
xcolor hud_sniper::m_SniperTrackerLineColor;
xcolor hud_sniper::m_SniperZoomTrackerColor;

rhandle<xbitmap> hud_sniper::m_SniperHud;
rhandle<xbitmap> hud_sniper::m_SniperStencilHud;
rhandle<xbitmap> hud_sniper::m_SniperTrackerLine;
rhandle<xbitmap> hud_sniper::m_SniperScanLine;
rhandle<xbitmap> hud_sniper::m_SniperZoomPitchTracker;

//==============================================================================
// FUNCTIONS
//==============================================================================

void hud_sniper::OnRender( player* pPlayer )
{
    // Are we in the sniper zoom mode?
    // Does the current player have the sniper rifle with the zoom mode.
    if( !pPlayer->RenderSniperZoom() )
    {
        return;
    }

    // update distance meter
    {
        f32 ZoomDistance;
        radian Pitch;
        radian Yaw;

        // set up positional information
        const vector3 ViewPos = pPlayer->GetEyesPosition();
        pPlayer->GetEyesPitchYaw(Pitch, Yaw);
        vector3 Dest( radian3( Pitch, Yaw, 0.0f ) );
        Dest *= g_ZoomDistance;
        vector3 EndPos = ViewPos + Dest;

        // set up collision info (NOTE: you can not use low poly here or it won't hit characters).
        g_CollisionMgr.AddToIgnoreList( pPlayer->GetGuid() );
        g_CollisionMgr.RaySetup( pPlayer->GetGuid(), ViewPos, EndPos );
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE);

        if( g_CollisionMgr.m_nCollisions > 0 )
        {
            ZoomDistance = g_CollisionMgr.m_Collisions[0].T * g_ZoomDistance;
            ZoomDistance = ZoomDistance/100.0f; // convert to meters
            xstring DistanceString( xfs("> %05.1f <", ZoomDistance) );
            m_WeaponZoomDistance = DistanceString;
        }
        else
        {
            xstring DistanceString( xfs("> >%05.1f <", g_ZoomDistance/100.0f) );
            m_WeaponZoomDistance = DistanceString;
        }
#ifndef X_RETAIL
    #ifdef ksaffel
            draw_Line( ViewPos, g_CollisionMgr.m_Collisions[0].Point,XCOLOR_YELLOW );
            draw_Sphere( g_CollisionMgr.m_Collisions[0].Point, 20.0f );
    #endif
#endif
    }
    
    new_weapon* pWeapon = pPlayer->GetCurrentWeaponPtr();
    ASSERT( pWeapon );
    weapon_sniper_rifle* pSniper = (weapon_sniper_rifle*)pWeapon;
    s32 WeaponZoomLevel = (s32)pSniper->GetZoomLevel();

    xstring ZoomString( xfs("%2dx", WeaponZoomLevel) );
    m_WeaponZoomLevel = ZoomString;

    vector3 ScreenCenter( (f32)m_ViewDimensions.Min.X + m_ViewDimensions.GetWidth()*0.5f, 
                          (f32)m_ViewDimensions.Min.Y + m_ViewDimensions.GetHeight()*0.5f, 0.0f );
    vector3 Pos( ScreenCenter );
    (void)Pos;


    // XBOX SNIPER BACKGROUND *************************************************

#ifdef TARGET_XBOX

    D3DVIEWPORT8 Vd;
    D3DVIEWPORT8 Vs;

    xbitmap* pStencilBmp = m_SniperStencilHud.GetPointer();
    //
    // Do the blurring, scan lines, and green coloring in here.
    //
    if( pStencilBmp )
    {   //
        // CLEAR THE ZBUFFER AND THE alpha channel simultaneously
        //
        irect iRect;
        iRect.l = (s32)m_ViewDimensions.Min.X;
        iRect.r = (s32)m_ViewDimensions.Max.X;
        iRect.t = (s32)m_ViewDimensions.Min.Y;
        iRect.b = (s32)m_ViewDimensions.Min.Y;

        f32 NearZ, FarZ;
        view rView;

        rView.GetZLimits( NearZ, FarZ );

        rect Rect;
        view& View = pPlayer->GetView();
        View.GetViewport( Rect );

        //
        // Blur the outside
        //
        extern void xbox_ApplyScopeBlur( xbitmap& BMP,D3DVIEWPORT8& Vd,D3DVIEWPORT8& Vs );
        extern void xbox_SetBackWithZ  ( void );

        g_RenderState.Set( D3DRS_STENCILPASS,D3DSTENCILOP_INCRSAT );
        g_RenderState.Set( D3DRS_STENCILZFAIL,D3DSTENCILOP_KEEP );
        g_RenderState.Set( D3DRS_STENCILFUNC,D3DCMP_ALWAYS );
        g_RenderState.Set( D3DRS_STENCILENABLE, TRUE );

        xbox_SetBackWithZ();
        eng_SetViewport( View );

        xbox_ApplyScopeBlur( *pStencilBmp,Vd,Vs );

        g_RenderState.Set( D3DRS_STENCILZFAIL,D3DSTENCILOP_KEEP );
        g_RenderState.Set( D3DRS_STENCILPASS,D3DSTENCILOP_KEEP );
        g_RenderState.Set( D3DRS_STENCILFUNC,D3DCMP_NOTEQUAL );
        g_RenderState.Set( D3DRS_STENCILREF,0 );
        g_Texture.Clear( 0 );

        // --------------------------------------------------------------------
        //
        // Draw the stencil to punch out the far the z and fill the alpha.
        //
        draw_Begin( DRAW_SPRITES,DRAW_USE_ALPHA|DRAW_TEXTURED|DRAW_2D|DRAW_UV_CLAMP|DRAW_NO_ZBUFFER|DRAW_XBOX_NO_BEGIN );

            g_RenderState.Set( D3DRS_BLENDCOLOR, D3DCOLOR_RGBA( 0,0,0,32 ));
            g_RenderState.Set( D3DRS_DESTBLEND,D3DBLEND_INVCONSTANTALPHA );
            g_RenderState.Set( D3DRS_SRCBLEND,D3DBLEND_CONSTANTALPHA );
            g_RenderState.Set( D3DRS_BLENDOP,D3DBLENDOP_ADD );

            f32 Gx = f32(g_PhysW-640)*0.5f;

        draw_Begin( DRAW_SPRITES,DRAW_KEEP_STATES );
        {
            rect Rect( f32(Vd.X)+Gx,
                       f32(Vd.Y),
                       f32(Vd.X)+f32(Vd.Width)+Gx,
                       f32(Vd.Y)+f32(Vd.Height) );

            // Just draw a big quad over the screen letting the stencil
            // buffer mask out the gun sight.

            draw_Color( XCOLOR_WHITE );
            draw_Vertex( Rect.Min.X  , Rect.Min.Y  , 0.0f );
            draw_Vertex( Rect.Min.X  , Rect.Max.Y-1, 0.0f );
            draw_Vertex( Rect.Min.X  , Rect.Max.Y-1, 0.0f );
            draw_Vertex( Rect.Max.X-1, Rect.Max.Y-1, 0.0f );
            draw_Vertex( Rect.Max.X-1, Rect.Max.Y-1, 0.0f );
            draw_Vertex( Rect.Max.X-1, Rect.Min.Y  , 0.0f );
            draw_Vertex( Rect.Max.X-1, Rect.Min.Y  , 0.0f );
            draw_Vertex( Rect.Min.X  , Rect.Min.Y  , 0.0f );
        }
        draw_End();

        // --------------------------------------------------------------------
        //
        // Render the dark green part
        //
        draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA|DRAW_TEXTURED|DRAW_2D|DRAW_NO_ZBUFFER );
        draw_DisableBilinear();
        //
        // Render the scan lines.
        //
        xbitmap* pScanLine = m_SniperScanLine.GetPointer();
        draw_SetTexture( *pScanLine );
    
        s32 ViewMinX = (s32)Vd.X;
        s32 ViewMinY = (s32)Vd.Y;

        vector2 ScanLineWH( (f32)pScanLine->GetWidth(), (f32)pScanLine->GetHeight() );
        ScanLineWH.X = f32(Vd.Width);

        vector3 ScanLinePos( 0.0f, 0.0f,( FarZ/NearZ)/2.0f );
        for( s32 y = ViewMinY; y < (m_ViewDimensions.GetHeight()+ViewMinY); y += (s32)ScanLineWH.Y )
        {
            ScanLinePos.GetX() = (f32)ViewMinX;
            ScanLinePos.GetY() = (f32)y;

            draw_Sprite( ScanLinePos, ScanLineWH, m_SniperScanLineColor );
        }

        draw_End(); 
        draw_EnableBilinear();
    }

    g_RenderState.Set( D3DRS_STENCILENABLE, FALSE );

#endif

    // PS2 SNIPER BACKGROUND **************************************************

#ifdef TARGET_PS2
    
    xbitmap* pStencilBmp = m_SniperStencilHud.GetPointer();
    
    //
    // Do the blurring, scan lines, and green coloring in here.
    //
    if( pStencilBmp )
    {
        draw_Begin( DRAW_QUADS, DRAW_USE_ALPHA|DRAW_2D );

        //
        // CLEAR THE ZBUFFER AND THE alpha channel simultaneously
        //
        irect iRect;
        iRect.l = (s32)m_ViewDimensions.Min.X;
        iRect.r = (s32)m_ViewDimensions.Max.X;
        iRect.t = (s32)m_ViewDimensions.Min.Y;
        iRect.b = (s32)m_ViewDimensions.Min.Y;
        //draw_ClearZBuffer( iRect );

        draw_Color( xcolor(255,255,255,0) );
            
        gsreg_Begin( 3 );

            gsreg_SetZBufferUpdate(TRUE);
            gsreg_SetFBMASK( ALPHA_CHANNEL_MASK );
            gsreg_SetAlphaAndZBufferTests(  FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                            FALSE, DEST_ALPHA_TEST_1, TRUE, ZBUFFER_TEST_ALWAYS );
        gsreg_End();

            f32 NearZ, FarZ;
            view rView;

            rView.GetZLimits( NearZ, FarZ );

            rect Rect( m_ViewDimensions );
            Rect.Min.X = 0.0f;
            Rect.Max.X = 512.0f;
            draw_Vertex( Rect.Min.X, Rect.Min.Y, NearZ+10.0f );
            draw_Vertex( Rect.Min.X, Rect.Max.Y, NearZ+10.0f );
            draw_Vertex( Rect.Max.X, Rect.Max.Y, NearZ+10.0f );
            draw_Vertex( Rect.Max.X, Rect.Min.Y, NearZ+10.0f );

        draw_End();

        //
        // Draw the stencil to punch out the far the z and fill the alpha.
        //
        draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA|DRAW_TEXTURED|DRAW_2D|DRAW_UV_CLAMP| DRAW_2D_KEEP_Z );

        gsreg_Begin( 2 );
            gsreg_SetFBMASK( ALPHA_CHANNEL_MASK );
                gsreg_SetAlphaAndZBufferTests(  TRUE, ALPHA_TEST_GEQUAL, 64, ALPHA_TEST_FAIL_KEEP,
                                                FALSE, DEST_ALPHA_TEST_1, TRUE, ZBUFFER_TEST_ALWAYS );
            
        gsreg_End();

            // Get the scale of the bitmap relative to how long and the wide the screen is.
            f32 ScaleX  = ((pStencilBmp->GetWidth()*2.0f) < m_ViewDimensions.GetWidth() ) ? 
                            1.0f : (m_ViewDimensions.GetWidth()/(pStencilBmp->GetWidth()*2.0f));

            f32 ScaleY  = ((pStencilBmp->GetHeight()*2.0f) < m_ViewDimensions.GetHeight() ) ? 
                            1.0f : (m_ViewDimensions.GetHeight()/(pStencilBmp->GetHeight()*2.0f));
    
            f32 Scale   = MIN( ScaleX, ScaleY );

            vector2 WH( (f32)m_ViewDimensions.Min.X + m_ViewDimensions.GetWidth()*0.5f, 
                        (f32)m_ViewDimensions.Min.Y + m_ViewDimensions.GetHeight()*0.5f );

            vector3 ScreenCenter( (f32)m_ViewDimensions.Min.X + m_ViewDimensions.GetWidth()*0.5f, 
                                    (f32)m_ViewDimensions.Min.Y + m_ViewDimensions.GetHeight()*0.5f, 
                                    0.0f );

            vector3 Pos( ScreenCenter );
            Pos.GetZ() = FarZ;

            // Get the UV offsets.
            f32 dx = WH.X/(pStencilBmp->GetWidth()*Scale);
            f32 dy = WH.Y/(pStencilBmp->GetHeight()*Scale);

            vector2 UV0(1.0f - dx, 1.0f - dy );
            vector2 UV1(1.0f, 1.0f);

            draw_SetTexture( *pStencilBmp );

            // Top Left.
            Pos.GetX() -= WH.X;
            Pos.GetY() -= WH.Y;
            draw_SpriteUV( Pos, WH, UV0, UV1, XCOLOR_WHITE );

            // Bottom Left.
            Pos.GetX() = ScreenCenter.GetX() - WH.X;
            Pos.GetY() = ScreenCenter.GetY();
            UV0.Y = 1.0f;
            UV1.Y = 1.0f - dy;  
            draw_SpriteUV( Pos, WH, UV0, UV1, XCOLOR_WHITE );

            // Bottom Right.
            WH.X  = 512.0f - ScreenCenter.GetX();
            dx    = WH.X/(pStencilBmp->GetWidth()*Scale);

            Pos.GetX() = ScreenCenter.GetX();
            Pos.GetY() = ScreenCenter.GetY();
            UV0.X = 1.0f;
            UV1.X = 1.0f - dx;

            draw_SpriteUV( Pos, WH, UV0, UV1, XCOLOR_WHITE );

            // Top Right.
            Pos.GetX() = ScreenCenter.GetX();
            Pos.GetY() = ScreenCenter.GetY() - WH.Y;
            UV0.Y = 1.0f - dy;
            UV1.Y = 1.0f;
            draw_SpriteUV( Pos, WH, UV0, UV1, XCOLOR_WHITE );

        draw_End();

        
        //
        // Blur the outside
        //
        render::BeginPostEffects();
        render::MipFilter( g_nFilters, g_MipOffset, (render::post_falloff_fn)g_MipFn, g_MipColor, g_MipParam1, g_MipParam2, 2 );
        render::EndPostEffects();


        //
        // Render the dark green part
        //
        draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA|DRAW_TEXTURED|DRAW_2D|DRAW_2D_KEEP_Z );
        //draw_Color( g_SniperStencilHudColor );
        draw_DisableBilinear();
    
        gsreg_Begin( 2 );

            gsreg_SetFBMASK( 0 );
            gsreg_SetAlphaAndZBufferTests(  FALSE, ALPHA_TEST_GEQUAL, 1, ALPHA_TEST_FAIL_KEEP,
                                            FALSE, DEST_ALPHA_TEST_1, TRUE, ZBUFFER_TEST_GEQUAL );
        gsreg_End();


        //
        // Render the scan lines.
        //
        xbitmap* pScanLine = m_SniperScanLine.GetPointer();
        draw_SetTexture( *pScanLine );
    
        s32 ViewMinX = (s32)m_ViewDimensions.Min.X;
        s32 ViewMinY = (s32)m_ViewDimensions.Min.Y;

        vector2 ScanLineWH( pScanLine->GetWidth(), pScanLine->GetHeight() );

        vector3 ScanLinePos( 0.0f, 0.0f,( FarZ/NearZ)/2.0f );
        for( s32 x = ViewMinX; x < (m_ViewDimensions.GetWidth()+ViewMinX); x += (s32)ScanLineWH.X)
        {
            for( s32 y = ViewMinY; y < (m_ViewDimensions.GetHeight()+ViewMinY); y += (s32)ScanLineWH.Y )
            {
                ScanLinePos.GetX() = (f32)x;
                ScanLinePos.GetY() = (f32)y;
                draw_Sprite( ScanLinePos, ScanLineWH, m_SniperScanLineColor );
            }
        }
        
        draw_End(); 
        draw_EnableBilinear();
    }
#endif

    //
    // Draw the zoom scope bitmap.
    //
    {
        xbitmap* pMainBmp       = m_SniperHud.GetPointer();
        xbitmap* pStencilBmp    = m_SniperStencilHud.GetPointer();

        if( !pMainBmp || !pStencilBmp )
            return;

        draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA|DRAW_TEXTURED|DRAW_2D|DRAW_NO_ZBUFFER|DRAW_UV_CLAMP|DRAW_BLEND_ADD );

        f32 ScaleX  = ((pMainBmp->GetWidth()*2.0f) < m_ViewDimensions.GetWidth() ) ? 
                      1.0f : (m_ViewDimensions.GetWidth()/(pMainBmp->GetWidth()*2.0f));

        f32 ScaleY  = ((pMainBmp->GetHeight()*2.0f) < m_ViewDimensions.GetHeight() ) ? 
                      1.0f : (m_ViewDimensions.GetHeight()/(pMainBmp->GetHeight()*2.0f));
        
        f32 Scale   = MIN( ScaleX, ScaleY );

        vector2 WH( pMainBmp->GetWidth () * Scale, 
                    pMainBmp->GetHeight() * Scale);

        Pos = ScreenCenter;

        vector2 UV0(0.0f, 0.0f );
        vector2 UV1(1.0f, 1.0f );

        draw_SetTexture( *pMainBmp );
    
        // Top Left.
        Pos.GetX() -= WH.X;
        Pos.GetY() -= WH.Y;
        draw_SpriteUV( Pos, WH, UV0, UV1, m_SniperHudColor );

        // Bottom Left.
        Pos.GetX() = ScreenCenter.GetX() - WH.X;
        Pos.GetY() = ScreenCenter.GetY();
        UV0.Y = 1.0f;
        UV1.Y = 0.0f;  
        draw_SpriteUV( Pos, WH, UV0, UV1, m_SniperHudColor );

        // Bottom Right.
        Pos.GetX() = ScreenCenter.GetX();
        Pos.GetY() = ScreenCenter.GetY();
        UV0.X = 1.0f;
        UV1.X = 0.0f;
        draw_SpriteUV( Pos, WH, UV0, UV1, m_SniperHudColor );

        // Top Right.
        Pos.GetX() = ScreenCenter.GetX();
        Pos.GetY() = ScreenCenter.GetY() - WH.Y;
        UV0.Y = 0.0f;
        UV1.Y = 1.0f;
        draw_SpriteUV( Pos, WH, UV0, UV1, m_SniperHudColor );
    
        xbitmap* pTrackerLines = m_SniperTrackerLine.GetPointer();
        
        //
        // Draw the tracker lines on the side of the zoom bitmap.
        //
        if( pTrackerLines )
        {
            vector2 TrackerWH( (f32)pTrackerLines->GetWidth(), (f32)pTrackerLines->GetHeight() );

            Pos.GetX() = ScreenCenter.GetX() - (WH.X + TrackerWH.X);
            Pos.GetY() = ScreenCenter.GetY() - TrackerWH.Y;

            vector3 tPos( Pos );

            draw_SetTexture( *pTrackerLines );

            // Draw the left side first.
            draw_Sprite( tPos, TrackerWH, m_SniperTrackerLineColor );
            s32 i = 1;
            for( i = 1; i < s_TrackerCount; i++ )
            {    
                tPos.GetY() = Pos.GetY() + (f32)i*TrackerWH.Y;

                draw_SpriteUV( tPos, TrackerWH, vector2( 0.0f, 0.0f ), vector2( 1.0f, 1.0f ), m_SniperTrackerLineColor );

                tPos.GetY() = Pos.GetY() - (f32)i*TrackerWH.Y;
                draw_SpriteUV( tPos, TrackerWH, vector2( 0.0f, 0.0f ), vector2( 1.0f, 1.0f ), m_SniperTrackerLineColor );
            }

            tPos.GetY() = Pos.GetY() + (f32)(i-1)*TrackerWH.Y + TrackerWH.Y*0.5f;

            vector2 TempWH( TrackerWH );
            TempWH.Y = TempWH.Y*0.5f;
            draw_SpriteUV( tPos, TempWH, vector2( 0.0f, 0.0f ), vector2( 1.0f, 0.f), m_SniperTrackerLineColor );

            // Draw the right side.
            tPos.GetX() = ScreenCenter.GetX() + WH.X;
            tPos.GetY() = ScreenCenter.GetY() - TrackerWH.Y;
            draw_SpriteUV( tPos, TrackerWH, vector2( 1.0f, 0.0f ), vector2( 0.0f, 1.0f ),m_SniperTrackerLineColor );
            for( i = 1; i < s_TrackerCount; i++ )
            {    
                tPos.GetY() = Pos.GetY() + (f32)i*TrackerWH.Y;

                draw_SpriteUV( tPos, TrackerWH, vector2( 1.0f, 0.0f ), vector2( 0.0f, 1.0f ), m_SniperTrackerLineColor );

                tPos.GetY() = Pos.GetY() - (f32)i*TrackerWH.Y;
                draw_SpriteUV( tPos, TrackerWH, vector2( 1.0f, 0.0f ), vector2( 0.0f, 1.0f ), m_SniperTrackerLineColor );
            }

            tPos.GetY() = Pos.GetY() + (f32)(i-1)*TrackerWH.Y + TrackerWH.Y*0.5f;

            draw_SpriteUV( tPos, TempWH, vector2( 1.0f, 0.0f ), vector2( 0.0f, 0.5f ), m_SniperTrackerLineColor );
            

        }
        
        //
        // Draw the tracker arrow.
        //
        xbitmap* pPitchTracker = m_SniperZoomPitchTracker.GetPointer();
        if( pPitchTracker && pTrackerLines )
        {
            
            vector2 PitchTrackerWH( (f32)pPitchTracker->GetWidth(), (f32)pPitchTracker->GetHeight() );
            vector2 TrackerWH( (f32)pTrackerLines->GetWidth(), (f32)pTrackerLines->GetHeight() );

            draw_SetTexture( *pPitchTracker );

            f32 Pitch,Yaw;
            pPlayer->GetEyesPitchYaw( Pitch, Yaw );
            
            f32 YDelta = (f32)(s_TrackerCount-1)*TrackerWH.Y  * (Pitch/(PI/2.0f));

            // Draw the left side first.
            Pos.GetX() = ScreenCenter.GetX() - (WH.X + PitchTrackerWH.X + TrackerWH.X);
            Pos.GetY() = (ScreenCenter.GetY()+YDelta) - (PitchTrackerWH.Y*0.5f);
           
            draw_Sprite( Pos, PitchTrackerWH, m_SniperZoomTrackerColor );

            Pos.GetX() = ScreenCenter.GetX() + WH.X + TrackerWH.X;
            Pos.GetY() = (ScreenCenter.GetY()+YDelta) - (PitchTrackerWH.Y*0.5f);
            draw_SpriteUV( Pos, PitchTrackerWH, vector2( 1.0f, 0.0f ), vector2( 0.0f, 1.0f ),m_SniperZoomTrackerColor );
        }
        
        draw_End();
    }

    //
    // Draw the text.
    //
#ifndef X_EDITOR
    {
        irect ZoomRect;

        // Get the position of the text in the view.
        vector3 SniperZoomTextPos( s_ZoomPos );
        SniperZoomTextPos.GetX() = (f32)((s32)( (SniperZoomTextPos.GetX()/512.0f)*(f32)m_ViewDimensions.GetWidth() ));
        SniperZoomTextPos.GetY() = (f32)((s32)( (SniperZoomTextPos.GetY()/448.0f)*(f32)m_ViewDimensions.GetHeight() ));

        ZoomRect.l = (s32)((SniperZoomTextPos.GetX() - s_ZoomWidth)  + m_ViewDimensions.Min.X);
        ZoomRect.r = (s32)((SniperZoomTextPos.GetX() + s_ZoomWidth)  + m_ViewDimensions.Min.X);;
        ZoomRect.t = (s32)((SniperZoomTextPos.GetY() - s_ZoomHeight) + m_ViewDimensions.Min.Y);;
        ZoomRect.b = (s32)((SniperZoomTextPos.GetY() + s_ZoomHeight) + m_ViewDimensions.Min.Y);;

        // Draw the shadow.
        ZoomRect.Translate( s_ShadowOffsetX, s_ShadowOffsetY );
        g_UiMgr->RenderText( 1, ZoomRect, ui_font::h_left|ui_font::v_top, s_ShadowColor, m_WeaponZoomLevel, TRUE, TRUE );

        // Now the real deal.
        ZoomRect.Translate( -s_ShadowOffsetX, -s_ShadowOffsetY );
        g_UiMgr->RenderText( 1, ZoomRect, ui_font::h_left|ui_font::v_top, s_ZoomColor, m_WeaponZoomLevel, TRUE, TRUE );
    }

    // draw distance meter
    {
        irect ZoomRect;

        // Get the position of the distance meter text in the view.
        vector3 SniperZoomDistanceTextPos( s_DistancePos );

        ui_font *pFont = g_UiMgr->GetFont("small");
        s32 count = m_WeaponZoomDistance.GetLength();
        s32 width = pFont->TextWidth(m_WeaponZoomDistance, count);
        s32 height = pFont->TextHeight(m_WeaponZoomDistance, count);

        rect screenRect;
        
        // set screenRect with actual values of active view
        eng_GetView()->GetViewport(screenRect);

        SniperZoomDistanceTextPos.GetX() = (f32)((screenRect.GetWidth()/2.0f) - width/2.0f);
        SniperZoomDistanceTextPos.GetY() = 
            (f32)((s32)( (SniperZoomDistanceTextPos.GetY()/screenRect.GetHeight())*(f32)m_ViewDimensions.GetHeight() ));

        ZoomRect.l = (s32)SniperZoomDistanceTextPos.GetX();
        ZoomRect.r = (s32)(SniperZoomDistanceTextPos.GetX() + width);
        ZoomRect.t = (s32)(SniperZoomDistanceTextPos.GetY() - height);
        ZoomRect.b = (s32)(SniperZoomDistanceTextPos.GetY() + height);

        // Draw the shadow.
        ZoomRect.Translate( s_ShadowOffsetX, s_ShadowOffsetY );
        g_UiMgr->RenderText( 1, ZoomRect, ui_font::h_left|ui_font::v_top, s_ShadowColor, m_WeaponZoomDistance, TRUE, TRUE );

        // Now the real deal.
        ZoomRect.Translate( -s_ShadowOffsetX, -s_ShadowOffsetY );
        g_UiMgr->RenderText( 1, ZoomRect, ui_font::h_left|ui_font::v_top, s_ZoomColor, m_WeaponZoomDistance, TRUE, TRUE );
        
    }

#endif

}

//==============================================================================

void hud_sniper::OnAdvanceLogic( player* pPlayer, f32 DeltaTime )
{
    (void)DeltaTime;
    (void)pPlayer;
}

//==============================================================================

xbool hud_sniper::OnProperty( prop_query& rPropQuery )
{
    (void)rPropQuery;

    //----------------------------------------------------------------------
    // Weapon hud overlay resources.
    //----------------------------------------------------------------------
    if( rPropQuery.IsVar( "Hud\\Weapon Hud\\Sniper Main" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_SniperHud.GetName(), RESOURCE_NAME_SIZE );
        }
        else            
        {
            const char* pStr = rPropQuery.GetVarExternal();
            m_SniperHud.SetName( pStr );
        }
        return TRUE;
    }

    if( rPropQuery.IsVar( "Hud\\Weapon Hud\\Sniper Stencil" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_SniperStencilHud.GetName(), RESOURCE_NAME_SIZE );
        }
        else            
        {
            const char* pStr = rPropQuery.GetVarExternal();
            m_SniperStencilHud.SetName( pStr );
        }
        return TRUE;
    }

    if( rPropQuery.IsVar( "Hud\\Weapon Hud\\Sniper Traker Line" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_SniperTrackerLine.GetName(), RESOURCE_NAME_SIZE );
        }
        else            
        {
            const char* pStr = rPropQuery.GetVarExternal();
            m_SniperTrackerLine.SetName( pStr );
        }
        return TRUE;
    }

    if( rPropQuery.IsVar( "Hud\\Weapon Hud\\Sniper Scan Line" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_SniperScanLine.GetName(), RESOURCE_NAME_SIZE );
        }
        else            
        {
            const char* pStr = rPropQuery.GetVarExternal();
            m_SniperScanLine.SetName( pStr );
        }
        return TRUE;
    }

    if( rPropQuery.IsVar( "Hud\\Weapon Hud\\Sniper Zoom Tracker" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_SniperZoomPitchTracker.GetName(), RESOURCE_NAME_SIZE );
        }
        else            
        {
            const char* pStr = rPropQuery.GetVarExternal();
            m_SniperZoomPitchTracker.SetName( pStr );
        }
        return TRUE;
    }

    if( rPropQuery.VarColor( "Hud\\Weapon Hud\\Sniper Hud Color", m_SniperHudColor ) )
        return TRUE;

    if( rPropQuery.VarColor( "Hud\\Weapon Hud\\Sniper Tracker Line Color", m_SniperTrackerLineColor ) )
        return TRUE;

    if( rPropQuery.VarColor( "Hud\\Weapon Hud\\Sniper Scan Line Color", m_SniperScanLineColor  ) )
        return TRUE;

    if( rPropQuery.VarColor( "Hud\\Weapon Hud\\Sniper Zoom Tracker Color", m_SniperZoomTrackerColor  ) )
        return TRUE;

    return FALSE;
}

//==============================================================================

void hud_sniper::OnEnumProp( prop_enum&  List )
{
    //----------------------------------------------------------------------
    // Weapon hud overlays.
    //----------------------------------------------------------------------
    List.PropEnumHeader  ( "Hud\\Weapon Hud", "The bitmaps for the weapons that will be overlayed on the hud.", 0 );
    List.PropEnumExternal( "Hud\\Weapon Hud\\Sniper Main", "Resource\0xbmp\0","The main sniper rifle hud overlay.", 0 );
    List.PropEnumExternal( "Hud\\Weapon Hud\\Sniper Stencil", "Resource\0xbmp\0","The stencil sniper rifle hud overlay.", 0 );
    List.PropEnumExternal( "Hud\\Weapon Hud\\Sniper Traker Line", "Resource\0xbmp\0","The sniper rifle tracker line.", 0 );
    List.PropEnumExternal( "Hud\\Weapon Hud\\Sniper Scan Line", "Resource\0xbmp\0","The sniper rifle scan line.", 0 );
    List.PropEnumExternal( "Hud\\Weapon Hud\\Sniper Center Reticle", "Resource\0xbmp\0","The sniper rifle center reticle piece.", 0 );
    List.PropEnumExternal( "Hud\\Weapon Hud\\Sniper Zoom Tracker", "Resource\0xbmp\0","The sniper zoom pitch tracking piece.", 0 );

    List.PropEnumColor   ( "Hud\\Weapon Hud\\Sniper Hud Color", "The color of the main sniper rifle hud.", 0 );
    List.PropEnumColor   ( "Hud\\Weapon Hud\\Sniper Tracker Line Color", "The color of the main sniper rifle hud.", 0 );
    List.PropEnumColor   ( "Hud\\Weapon Hud\\Sniper Scan Line Color", "The color of the main sniper rifle hud.", 0 );
    List.PropEnumColor   ( "Hud\\Weapon Hud\\Sniper Zoom Tracker Color", "The color of the zoom tracker piece.", 0 );
}
