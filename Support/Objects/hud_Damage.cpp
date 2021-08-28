//==============================================================================
//
//  hud_Damage.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
/*

    aharp TODO - See list below...

    - Seems like the damage bar should render in alpha mode (multiplicative)
      so it will show up against bright backgrounds.  Check this.

*/
//==============================================================================
// INCLUDES
//==============================================================================

#include "hud_Damage.hpp"
#include "HudObject.hpp"

//==============================================================================
// STORAGE
//==============================================================================

rhandle<xbitmap>            hud_damage::m_DamageBitmap;
f32                         hud_damage::m_DamageTimeTillFade;
f32                         hud_damage::m_DamageBitmapOffset;
f32                         hud_damage::m_DamageFadeOutTime;
f32                         hud_damage::m_ScreenFlashDeltaTime;

xcolor                      hud_damage::m_FragGrenadeDamageColor;
xcolor                      hud_damage::m_GravGrenadeDamageColor;

//==============================================================================
// FUNCTIONS
//==============================================================================

hud_damage::hud_damage( void ) 
{
    m_DamageBitmapOffset        = 65.0f;
    m_DamageTimeTillFade        =  3.0f;
    m_DamageFadeOutTime         = 10.0f;
    m_ScreenFlashDeltaTime      =  0.0f;

    s32 i;
    for( i = 0; i < MAX_PAIN_EVENTS; i++ )
    {
        m_pPain[i].LocalSlot    = -1;
        m_pPain[i].Overlay      = 0;
    }
}

//==============================================================================

void hud_damage::OnRender( player* pPlayer )
{
    ASSERT( pPlayer );

    s32      g_MaxDamageAlpha = 255;
    player&  rPlayer          = *pPlayer;
    xbitmap* pBitmap          = m_DamageBitmap.GetPointer();

    if( pBitmap == NULL )
        return;

    draw_Begin( DRAW_SPRITES, DRAW_USE_ALPHA|DRAW_BLEND_ADD|DRAW_TEXTURED|DRAW_2D|DRAW_NO_ZBUFFER|DRAW_UV_CLAMP );
    draw_SetTexture( *pBitmap );

    for( s32 iPain = 0; iPain < MAX_PAIN_EVENTS; iPain++ )
    {
        // Skip the entries not in use
        if( m_pPain[iPain].LocalSlot == -1 )
            continue;

        // Is this for split screen?  VERIFY.
        if( m_pPain[iPain].LocalSlot != rPlayer.GetLocalSlot() )
            continue;
    
        //
        // Render pain.
        //
        {
            view& rView = rPlayer.GetView();

            radian Pitch;
            radian Yaw;
            rView.GetPitchYaw( Pitch, Yaw );

            radian Diff = m_pPain[iPain].LastRot + Yaw;
            
            vector3 ScreenPos;
            vector2 WH( (f32)pBitmap->GetWidth(), (f32)pBitmap->GetHeight() );

            // aharp TODO Make sure this scales for split screen.
            f32 DamageOffsetX = 75.0f; //m_DamageBitmapOffset * (m_ViewDimensions.GetWidth() /512.0f);
            f32 DamageOffsetY = 75.0f; //m_DamageBitmapOffset * (m_ViewDimensions.GetHeight()/448.0f);

            // Go 2D.

            // Get the screen center.
            f32 CenterX = m_XPos; //((f32)m_ViewDimensions.Min.X+(m_ViewDimensions.GetWidth()) /2.0f);
            f32 CenterY = m_YPos; //((f32)m_ViewDimensions.Min.Y+(m_ViewDimensions.GetHeight())/2.0f);
            
            f32 TempX = ((x_cos( Diff ) * DamageOffsetX) + CenterX);
            f32 TempY = ((x_sin( Diff ) * DamageOffsetY) + CenterY);
            ScreenPos.GetX() = TempX; // - (WH.X/2.0f);
            ScreenPos.GetY() = TempY; // - (WH.Y/2.0f);

            vector2 UV0( 0.0f, 0.0f );
            vector2 UV1( 1.0f, 1.0f );

            xcolor Color = XCOLOR_WHITE;
            
            if( m_pPain[iPain].PainTime > m_DamageFadeOutTime )
            {
                Color.A = g_MaxDamageAlpha;
            }
            else
            {
                Color.A = (u8)( MAX( 0.0f, (m_pPain[iPain].PainTime / m_DamageFadeOutTime) * g_MaxDamageAlpha) );
            }

            // Check for pulsing.
            if( m_bPulsing )
            {
                Color.A = (u8)(((f32)Color.A / 255) * m_bPulsing);
            }
            
            //
            // NOTE: The draw mode has to be additive.
            //
            for( s32 iRender = 0; iRender < m_pPain[iPain].Overlay; iRender++ )
            {
                draw_SpriteUV( ScreenPos, WH, UV0, UV1, Color, -Diff ); 
            }
        }
    }
    
    draw_End();
}

//==============================================================================

void hud_damage::OnAdvanceLogic( player* pPlayer, f32 DeltaTime )
{
    ASSERT( pPlayer );
    player& rPlayer = *pPlayer;

    f32 g_ScreenFlashTime = 0.1f;

    // Get the pain info.
    const xarray<pain>& rPain = rPlayer.GetLastPainEvents();

    // If the player dies reset the pain.
    if( rPlayer.GetHealth() <= 0.0f )   // TODO - Use the IsDead function!
    {    
        for( s32 iPain = 0; iPain < MAX_PAIN_EVENTS; iPain++ )
            m_pPain[ iPain ].LocalSlot = -1;
        
        rPlayer.ClearPainEvent();
        m_ScreenFlashDeltaTime = 0.0f;
        return;
    }
    else
    {
        // The player is still alive, lets do a screen flash if we got hit.
        if( rPain.GetCount() > 0 )
            m_ScreenFlashDeltaTime = g_ScreenFlashTime;
    }
    
    //
    // Manage existing pain.
    //
    for( s32 iPain = 0; iPain < MAX_PAIN_EVENTS; iPain++ )
    {
        if( m_pPain[ iPain ].PainTime > 0.0f )
        {
            m_pPain[ iPain ].PainTime -= DeltaTime;
        }
        else
        {                
            // Remove the pain event from the list.
            m_pPain[iPain].LocalSlot = -1;
        }
    }
    
    //
    // Handle all new pain events.
    //
    for( s32 i = 0; i < rPain.GetCount(); i++ )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( rPain[i].GetOriginGuid() );

        if( pObj )
        {
            vector3 Position = (rPlayer.GetGuid() != rPain[i].GetOriginGuid()) ? pObj->GetPosition() : rPain[i].GetPosition();
           
            // Get the enemy's position in the players local space.
            vector3 ToSource = rPlayer.GetPosition() - Position;
            
            ToSource.GetY() = 0.0f;
            ToSource.Normalize();

            vector2 DamageVec( ToSource.GetX(), ToSource.GetZ() );
            radian DamageRotation = DamageVec.Angle();

            s32 j;
            
            //
            // If a pain event with the same owner already exists in the list then increase its overlay count.
            //
            for( j = 0; j < MAX_PAIN_EVENTS; j++ )
            {
                if( (m_pPain[j].LocalSlot != -1 ) &&
                    (m_pPain[j].Pain.GetOriginGuid() == rPain[i].GetOriginGuid()) )
                {
                    if( m_pPain[j].LastRot == DamageRotation )
                    {
                        m_pPain[j].PainTime = m_DamageTimeTillFade + m_DamageFadeOutTime;
                        m_pPain[j].Overlay++;
                        break;
                    }
                }
            }

            if( j != MAX_PAIN_EVENTS )
                continue;
            
            //
            // Put a new pain event in the list.
            // 
            for( j = 0; j < MAX_PAIN_EVENTS; j++ )
            {
                if( m_pPain[j].LocalSlot == -1 )
                {                
                    m_pPain[j].Pain     = rPain[i];
                    m_pPain[j].PainTime = m_DamageTimeTillFade + m_DamageFadeOutTime;
                    m_pPain[j].LastRot  = (rPlayer.GetGuid() != rPain[i].GetOriginGuid()) ? DamageRotation : DamageRotation;
                    m_pPain[j].Overlay  = 1;
                    m_pPain[j].LocalSlot= rPlayer.GetLocalSlot();

                    break;
                }
            }

#ifdef aharp
            if( j == MAX_PAIN_EVENTS )
                ASSERTS( 0, "Increase MAX_PAIN_EVENTS" );
#endif
        }
    }
    rPlayer.ClearPainEvent();

    m_ScreenFlashDeltaTime -= DeltaTime;
}

//==============================================================================

xbool hud_damage::OnProperty( prop_query& rPropQuery )
{
    (void)rPropQuery;

    //----------------------------------------------------------------------
    // Damage Stage.
    //----------------------------------------------------------------------
    if( rPropQuery.IsVar( "Hud\\Damage State\\Bitmap Resource" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_DamageBitmap.GetName(), RESOURCE_NAME_SIZE );
        }
        else            
        {
            const char* pStr = rPropQuery.GetVarExternal();
            m_DamageBitmap.SetName( pStr );
        }
        return TRUE;
    }    

    if( rPropQuery.VarFloat( "Hud\\Damage State\\Damage Render Offset", m_DamageBitmapOffset ) )
        return TRUE;

    if( rPropQuery.VarFloat( "Hud\\Damage State\\Damage Time Till Fade Begin", m_DamageTimeTillFade ) )
        return TRUE;

    if( rPropQuery.VarFloat( "Hud\\Damage State\\Damage Fade Out Time", m_DamageFadeOutTime ) )
        return TRUE;

    if( rPropQuery.VarColor( "Hud\\Damage State\\Frag Grenade Damage Color", m_FragGrenadeDamageColor ) )
        return TRUE;

    if( rPropQuery.VarColor( "Hud\\Damage State\\Grav Grenade Damage Color", m_GravGrenadeDamageColor ) )
        return TRUE;


    return FALSE;
}

//==============================================================================

void hud_damage::OnEnumProp( prop_enum&  List )
{
    //----------------------------------------------------------------------
    // Damage State.
    //----------------------------------------------------------------------
    List.PropEnumHeader  ( "Hud\\Damage State", "Damage state variables", 0 );
    List.PropEnumExternal( "Hud\\Damage State\\Bitmap Resource", "Resource\0xbmp\0", "The Damage arrow bitmap", 0 );
    List.PropEnumFloat   ( "Hud\\Damage State\\Damage Render Offset", "How far from the center of the screen is the damage going to be moving around.", 0 );
    List.PropEnumFloat   ( "Hud\\Damage State\\Damage Time Till Fade Begin", "How long is the damage indicator going to stay on the screen before it starts fading.", 0 );
    List.PropEnumFloat   ( "Hud\\Damage State\\Damage Fade Out Time", "How many seconds to fade the damage indicator across.", 0 );
    List.PropEnumColor   ( "Hud\\Damage State\\Frag Grenade Damage Color", "What color the screen should flash when taking damage from a frag grenade", 0 );
    List.PropEnumColor   ( "Hud\\Damage State\\Grav Grenade Damage Color", "What color the screen should flash when taking damage from a graviton charge grenade", 0 );
}

//==============================================================================
