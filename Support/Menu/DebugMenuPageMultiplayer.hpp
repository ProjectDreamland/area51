//==============================================================================
//  DebugMenuPageMultiplayer.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is the specific page for multiplayer debug
//  
//==============================================================================

#ifndef DEBUG_MENU_PAGE_MULTIPLAYER_HPP
#define DEBUG_MENU_PAGE_MULTIPLAYER_HPP

//==============================================================================
//  DATA STRUCTURES
//==============================================================================

class debug_menu_page_multiplayer : public debug_menu_page
{
public:
                        debug_menu_page_multiplayer     ( void );
    virtual            ~debug_menu_page_multiplayer     ( void ) { };
    virtual void        OnChangeItem                    ( debug_menu_item* pItem );
    virtual void        OnPreRender                     ( void );

protected:

    void                RenderText ( const xwstring& strOutput, s32& x, s32& y, const s32& font, irect& Rect );

    debug_menu_item*    m_pSkinItem;
    s32                 m_AvatarSkin;
    
    debug_menu_item*    m_pVoiceActorItem;
    s32                 m_VoiceActor;
    debug_menu_item*    m_pPlayerCount;   

    xbool               m_bToggleNetInfo;
    debug_menu_item*    m_pClientIndex;   
    s32                 m_iClientIndex;
    
    s32                 m_iSpawnPoint;
    debug_menu_item*    m_pSpawnPoint;
    xstring             m_SpawnPointName;
};

//==============================================================================
#endif  // DEBUG_MENU_PAGE_NULTIPLAYER_HPP
//==============================================================================
