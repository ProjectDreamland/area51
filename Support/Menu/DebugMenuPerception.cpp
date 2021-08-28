//==============================================================================
//  DebugMenuPerception.cpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This is the implementation for the Debug menu gameplay page.
//  
//==============================================================================

#include "DebugMenu2.hpp"
#include "PerceptionMgr\PerceptionMgr.hpp"

//==============================================================================

#if defined( ENABLE_DEBUG_MENU )

f32 g_SetMutantTargetGlobalTimeDialation = 0.65f;
f32 g_SetMutantTargetPlayerTimeDialation = 1.0f;
f32 g_SetMutantTargetAudioTimeDialation  = 0.65f;
f32 g_SetMutantTargetForwardSpeedFactor  = 1.0f;
f32 g_SetMutantBeginDelay                = 1.3f;
f32 g_SetMutantBeginLength               = 2.5f;
f32 g_SetMutantEndDelay                  = 0.0f;
f32 g_SetMutantEndLength                 = 2.5f;

//==============================================================================

debug_menu_perception::debug_menu_perception( ) : debug_menu_page()
{
    m_pTitle = "Mutation Perception";

    m_pSetMutantTargetGlobalTimeDialation = AddItemFloat( "Global Time Dialation", 
                                                          g_SetMutantTargetGlobalTimeDialation, 
                                                          0.01f, 1.0f, 0.01f );
    m_pSetMutantTargetPlayerTimeDialation = AddItemFloat( "Player Time Dialation", 
                                                          g_SetMutantTargetPlayerTimeDialation, 
                                                          0.01f, 2.0f, 0.01f );
    m_pSetMutantTargetAudioTimeDialation  = AddItemFloat( "Audio Time Dialation", 
                                                          g_SetMutantTargetAudioTimeDialation, 
                                                          0.01f, 2.0f, 0.01f );
    m_pSetMutantTargetForwardSpeedFactor  = AddItemFloat( "Player Forward Speed", 
                                                          g_SetMutantTargetForwardSpeedFactor, 
                                                          0.01f, 2.0f, 0.01f );
    m_pSetMutantBeginDelay                = AddItemFloat( "Pre-RampIn Delay", 
                                                          g_SetMutantBeginDelay, 
                                                          0.0f, 5.0f, 0.1f );

    m_pSetMutantBeginLength               = AddItemFloat( "RampIn Time", 
                                                          g_SetMutantBeginLength, 
                                                          0.1f, 5.0f, 0.1f );

    m_pSetMutantEndDelay                  = AddItemFloat( "Pre-RampOut Delay", 
                                                          g_SetMutantEndDelay, 
                                                          0.0f, 5.0f, 0.1f );

    m_pSetMutantEndLength                 = AddItemFloat( "RampOut Time", 
                                                          g_SetMutantEndLength, 
                                                          0.1f, 5.0f, 0.1f );
}

//==============================================================================

void debug_menu_perception::OnChangeItem( debug_menu_item* pItem )
{
    if( pItem == m_pSetMutantTargetGlobalTimeDialation )
    {
        g_PerceptionMgr.SetMutantTargetGlobalTimeDialation( g_SetMutantTargetGlobalTimeDialation );
    }
    if( pItem == m_pSetMutantTargetPlayerTimeDialation  )
    {
        g_PerceptionMgr.SetMutantTargetPlayerTimeDialation( g_SetMutantTargetPlayerTimeDialation );
    }
    if( pItem == m_pSetMutantTargetAudioTimeDialation )
    {
        g_PerceptionMgr.SetMutantTargetAudioTimeDialation( g_SetMutantTargetAudioTimeDialation );
    }
    if( pItem == m_pSetMutantTargetForwardSpeedFactor )
    {
        g_PerceptionMgr.SetMutantTargetForwardSpeedFactor( g_SetMutantTargetForwardSpeedFactor );
    }
    if( pItem == m_pSetMutantBeginDelay  )
    {
        g_PerceptionMgr.SetMutantBeginDelay( g_SetMutantBeginDelay );
    }
    if( pItem == m_pSetMutantBeginLength  )
    {
        g_PerceptionMgr.SetMutantBeginLength( g_SetMutantBeginLength );
    }      
    if( pItem == m_pSetMutantEndDelay )
    {
        g_PerceptionMgr.SetMutantEndDelay( g_SetMutantEndDelay );
    }     
    if( pItem == m_pSetMutantEndLength )
    {
        g_PerceptionMgr.SetMutantEndLength( g_SetMutantEndLength );
    }               
}

#endif // defined( ENABLE_DEBUG_MENU )
