//==============================================================================
//
//  Monkey.hpp
//
//==============================================================================

#ifndef MONKEY_HPP
#define MONKEY_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "e_Input.hpp"


//==============================================================================
//  To add a Monkey Mode:
//      1.  Increment k_NumMonkeyModes below.
//      2.  Add monkey_mode enum below.
//      3.  In DebugMenuPageMonkey.cpp, add a corresponding switch to the menu.
//      4.  In Monkey.cpp, initialize extra bool in g_MonkeyOptions.
//      4.  In Monkey.cpp, add stick data to the MonkeyStickData array.
//      5.  In Monkey.cpp, add button data to MonkeyButtonPercentages array.
//==============================================================================

//==============================================================================
//  CONSTANTS
//==============================================================================

const s32 k_NumMonkeyModes = 9;

//==============================================================================
//  TYPES
//==============================================================================

enum monkey_mode
{
    MONKEY_NORMAL       = 0,
    MONKEY_JUMPMAN      = 1,
    MONKEY_CROUCHMAN    = 2,
    MONKEY_GUNMAN       = 3,
    MONKEY_GRENADIER    = 4,
    MONKEY_MUTATION     = 5,
    MONKEY_MENUMONKEY   = 6,
    MONKEY_TWITCH       = 7,
    MONKEY_MEMHOG		= 8
};

enum monkey_ui_button
{
    MONKEY_UI_UP,
    MONKEY_UI_DOWN,
    MONKEY_UI_RIGHT,
    MONKEY_UI_LEFT,
    MONKEY_UI_SELECT,
    MONKEY_UI_BACK,
    MONKEY_UI_DELETE,
    MONKEY_UI_ACTIVATE,
    MONKEY_UI_SHOULDERL,
    MONKEY_UI_SHOULDERR,
    MONKEY_UI_SHOULDERL2,
    MONKEY_UI_SHOULDERR2,
    MONKEY_UI_HELP
};

struct monkey_options
{
    xbool   Enabled;
    xbool   ModeEnabled[k_NumMonkeyModes];
    xbool   Dirty;
    xbool   bTestOutOfWorld;
};

struct pad_data
{
    f32 pad_l_x;
    f32 pad_l_y;
    f32 pad_r_x;
    f32 pad_r_y;
};

class monkey
{
    public:
        monkey();
        ~monkey();

        void        Update( f32 DeltaTime );
        f32         GetValue( s32 LogicalPadMapping );
        s32         GetCurrentMode() const;
        xbool       GetUIButtonValue( monkey_ui_button button );
        xbool       ShouldPause() const;
        xbool       ShouldUnpause();
        void        SetAutoMonkeyMode(s32 mode);

        void        SetPlayerInfo( const vector3& player_pos, const radian& player_pitch, const radian& player_yaw, xbool player_invert_y, const vector3& closest_hostile_post, xbool is_mutant );        
        
    private:

        xbool       AreAllModesDisabled() const;
        s32         ChooseMode() const;
        xbool       IsLookingUp() const;

        pad_data    m_MonkeyPadData;

        f32         m_RetVal;
        s32         m_CurrentMonkeyMode;        
        f32         m_MonkeyPadTimer;
        f32         m_MonkeyModeTimer;
        vector3     m_PlayerPosition;
        radian      m_PlayerPitch;
        radian      m_PlayerYaw;
        xbool       m_PlayerInvertY;
        xbool       m_bFastMenuMonkey;
        xbool       m_bExitingMenuMonkeyMode;
        xbool       m_bIsMutant;
        vector3     m_ClosestHostilePosition;
};

//==============================================================================
//  GLOBALS
//==============================================================================

extern monkey_options g_MonkeyOptions;
extern monkey         g_Monkey;

//==============================================================================
#endif // MONKEY_HPP
//==============================================================================
