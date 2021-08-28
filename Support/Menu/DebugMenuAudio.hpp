//==============================================================================
//  DebugMenuAudio.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is the specific page for AI and scripting options.
//  
//==============================================================================

#ifndef DEBUG_MENU_AUDIO_HPP
#define DEBUG_MENU_AUDIO_HPP

//==============================================================================

class debug_menu_audio : public debug_menu_page
{
public:
                                debug_menu_audio      ( );
    virtual                    ~debug_menu_audio     ( ) { };

    virtual void                OnChangeItem        ( debug_menu_item* pItem );
    virtual void                OnPreRenderActive   ( void );

protected:
    debug_menu_item*            m_pDumpLoadedPackages;
    debug_menu_item*            m_pEnableVUMeter;    
};

//==============================================================================

#endif // DEBUG_MENU_AUDIO_HPP
