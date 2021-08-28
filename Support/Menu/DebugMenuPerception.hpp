//==============================================================================
//  DebugMenuPerception.hpp
//
//  Copyright (c) 2002-2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This class is the specific page for AI and scripting options.
//  
//==============================================================================

#ifndef DEBUG_MENU_PERCEPTION_HPP
#define DEBUG_MENU_PERCEPTION_HPP

//==============================================================================

class debug_menu_perception : public debug_menu_page
{
public:
                                debug_menu_perception( );
    virtual                    ~debug_menu_perception( ) { };

    virtual void                OnChangeItem         ( debug_menu_item* pItem );

protected:
    debug_menu_item*            m_pSetMutantTargetGlobalTimeDialation;
    debug_menu_item*            m_pSetMutantTargetPlayerTimeDialation;
    debug_menu_item*            m_pSetMutantTargetAudioTimeDialation;
    debug_menu_item*            m_pSetMutantTargetForwardSpeedFactor;
    debug_menu_item*            m_pSetMutantBeginDelay;
    debug_menu_item*            m_pSetMutantBeginLength;
    debug_menu_item*            m_pSetMutantEndDelay;
    debug_menu_item*            m_pSetMutantEndLength;
};

//==============================================================================

#endif // DEBUG_MENU_PERCEPTION_HPP
