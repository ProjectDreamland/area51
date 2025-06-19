#include "GamePad.hpp"

//=========================================================================
// VARS
//=========================================================================

ingame_pad g_IngamePad[ MAX_LOCAL_PLAYERS ];

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

ingame_pad::ingame_pad( void )
{
    m_pName = "Ingame Pad";
    g_InputMgr.RegisterPad( *this );
}

//=========================================================================

void ingame_pad::OnInitialize( void )
{
    //
    // Set all my logical actions
    //
    SetLogical( MOVE_STRAFE,            "Strafe" );
    SetLogical( MOVE_FOWARD_BACKWARDS,  "Fowards/Backwards" );
    SetLogical( LOOK_HORIZONTAL,        "Horizontral Look" );
    SetLogical( LOOK_VERTICAL,          "Vertical Look" );
    SetLogical( LEAN_LEFT,              "Lean Left" );
    SetLogical( LEAN_RIGHT,             "Lean Right" );

    SetLogical( ACTION_RELOAD,          "Reload" );
    SetLogical( ACTION_PRIMARY,         "Primary" );
    SetLogical( ACTION_SECONDARY,       "Secondary" );
    SetLogical( ACTION_JUMP,            "Jump" );
    SetLogical( ACTION_CROUCH,          "Crouch" );
    SetLogical( ACTION_MUTATION,        "Toggle Mutation");
    SetLogical( ACTION_FIRE_PARASITES,  "Fire Parasites" );
    SetLogical( ACTION_FIRE_CONTAGION,  "Fire Contagion" );
    SetLogical( ACTION_MUTANT_MELEE,    "Mutant Melee" );
    SetLogical( ACTION_CYCLE_RIGHT,     "Cycle Weapons Right" );
    SetLogical( ACTION_CYCLE_LEFT,      "Cycle Weapons Left" );
    SetLogical( ACTION_USE,             "Use Object");
    SetLogical( ACTION_FLASHLIGHT,      "Toggle Flashlight");
    SetLogical( ACTION_TOGGLE_PRECISE_AIM, "Toggle Precise Aim");
    
    // Conversation mappings.
    SetLogical( ACTION_SPEAK_FOLLOW_STAY,   "Speak: Follow Me" );
    SetLogical( ACTION_SPEAK_USE_ACTIVATE,  "Speak: Use / Activate" );
    SetLogical( ACTION_SPEAK_COVER_ME,      "Speak: Cover Me" );
    SetLogical( ACTION_SPEAK_ATTACK_COVER,  "Speak: Attack / Take Cover" );


    SetLogical( ACTION_HUD_CONTEXT,         "Hud Context switch" );
    SetLogical( ACTION_PAUSE_CONTEXT,       "Pause menu context switch" );

    SetLogical( ACTION_HUD_MOVEMENT_HORIZONTAL, "Move selection cursor horizontal" );
    SetLogical( ACTION_HUD_MOVEMENT_VERTICAL,   "Move selection cursor vertical" );
    SetLogical( ACTION_HUD_SET_HOTKEY_0,        "Assign the item as hot key 0" );
    SetLogical( ACTION_HUD_SET_HOTKEY_1,        "Assign the item as hot key 1" );
    SetLogical( ACTION_USE_HOTKEY_0,            "Use the item in hot key 0" );
    SetLogical( ACTION_USE_HOTKEY_1,            "Use the item in hot key 1" );
    SetLogical( ACTION_WEAPON_ITEM_SWITCH,      "Toggle between item and weapons" );
    SetLogical( ACTION_THROW_GRENADE,           "Throw a grenade" );
    SetLogical( ACTION_CYCLE_GRENADE_TYPE,      "Cycle grenade type" );
    SetLogical( ACTION_MELEE_ATTACK,            "Melee attack" );


    SetLogical( ACTION_VOTE_MENU_ON,            "Vote: Menu On" );
    SetLogical( ACTION_VOTE_MENU_OFF,           "Vote: Menu Off" );
    SetLogical( ACTION_VOTE_YES,                "Vote: Yes" );
    SetLogical( ACTION_VOTE_NO,                 "Vote: No" );
    SetLogical( ACTION_VOTE_ABSTAIN,            "Vote: Abstain" );

    SetLogical( ACTION_CHAT,                    "Voice Chat" );
    SetLogical( ACTION_TALK_MODE_TOGGLE,        "Talk: Mode Toggle" );

    SetLogical( ACTION_MP_FLASHLIGHT,           "Multiplayer Toggle Flashlight" );
    SetLogical( ACTION_MP_MUTATE,               "Multiplayer Toggle Mutation" );
    SetLogical( ACTION_DROP_FLAG,               "Drop Flag" );

// todo: this is all bogus! should be ripped out for both platforms?
#ifdef TARGET_XBOX
    // Set the default controler
    // Left Analog
    AddMapping( INPUT_PLATFORM_XBOX, MOVE_STRAFE,            INPUT_XBOX_STICK_LEFT_X, FALSE );
    AddMapping( INPUT_PLATFORM_XBOX, MOVE_FOWARD_BACKWARDS,  INPUT_XBOX_STICK_LEFT_Y, FALSE );

    // Right Analog
    AddMapping( INPUT_PLATFORM_XBOX, LOOK_HORIZONTAL,        INPUT_XBOX_STICK_RIGHT_X, FALSE );
    AddMapping( INPUT_PLATFORM_XBOX, LOOK_VERTICAL,          INPUT_XBOX_STICK_RIGHT_Y, FALSE );

    // Shoulder buttons
    AddMapping( INPUT_PLATFORM_XBOX, ACTION_PRIMARY,         INPUT_XBOX_BTN_R_STICK,  TRUE );
    AddMapping( INPUT_PLATFORM_XBOX, ACTION_SECONDARY,       INPUT_XBOX_R_TRIGGER,    TRUE );
    AddMapping( INPUT_PLATFORM_XBOX, ACTION_JUMP,            INPUT_XBOX_BTN_L_STICK,  TRUE );
    AddMapping( INPUT_PLATFORM_XBOX, ACTION_CROUCH,          INPUT_XBOX_L_TRIGGER,    TRUE );
    
    // Buttons
    AddMapping( INPUT_PLATFORM_XBOX, ACTION_RELOAD,          INPUT_XBOX_BTN_Y,     TRUE );
    AddMapping( INPUT_PLATFORM_XBOX, ACTION_USE,             INPUT_XBOX_BTN_X,     TRUE );
    AddMapping( INPUT_PLATFORM_XBOX, ACTION_FLASHLIGHT,      INPUT_XBOX_BTN_WHITE, TRUE );

    AddMapping( INPUT_PLATFORM_XBOX, ACTION_CHAT,            INPUT_XBOX_BTN_BLACK, TRUE );

    // D - Pad
    AddMapping( INPUT_PLATFORM_XBOX, ACTION_SPEAK_FOLLOW_STAY,   INPUT_XBOX_BTN_UP,    TRUE );
    AddMapping( INPUT_PLATFORM_XBOX, ACTION_SPEAK_USE_ACTIVATE,  INPUT_XBOX_BTN_LEFT,  TRUE );
    AddMapping( INPUT_PLATFORM_XBOX, ACTION_SPEAK_COVER_ME,      INPUT_XBOX_BTN_RIGHT, TRUE );
    AddMapping( INPUT_PLATFORM_XBOX, ACTION_SPEAK_ATTACK_COVER,  INPUT_XBOX_BTN_DOWN,  TRUE );

    AddMapping( INPUT_PLATFORM_XBOX, ACTION_HUD_CONTEXT,             INPUT_XBOX_L_TRIGGER, TRUE );
    AddMapping( INPUT_PLATFORM_XBOX, ACTION_PAUSE_CONTEXT,           INPUT_XBOX_BTN_START,     TRUE );

    AddMapping( INPUT_PLATFORM_XBOX, ACTION_HUD_MOVEMENT_HORIZONTAL, INPUT_XBOX_STICK_LEFT_X, FALSE );
    AddMapping( INPUT_PLATFORM_XBOX, ACTION_HUD_MOVEMENT_VERTICAL,   INPUT_XBOX_STICK_LEFT_Y, FALSE );

    AddMapping( INPUT_PLATFORM_XBOX, ACTION_TALK_MODE_TOGGLE, INPUT_XBOX_BTN_BLACK, TRUE );
#endif

#ifdef TARGET_PC
    //Set the default PC controls.
    //Mouse
    AddMapping( INPUT_PLATFORM_PC, ACTION_PRIMARY,       INPUT_MOUSE_BTN_L,      TRUE );
    AddMapping( INPUT_PLATFORM_PC, ACTION_SECONDARY,     INPUT_MOUSE_BTN_R,      TRUE );
    AddMapping( INPUT_PLATFORM_PC, LOOK_HORIZONTAL,      INPUT_MOUSE_X_REL,      FALSE );
    AddMapping( INPUT_PLATFORM_PC, LOOK_VERTICAL,        INPUT_MOUSE_Y_REL,      FALSE );
    
    //Movement
    AddMapping( INPUT_PLATFORM_PC, MOVE_FORWARD,         INPUT_KBD_W,            FALSE );
    AddMapping( INPUT_PLATFORM_PC, MOVE_BACKWARD,        INPUT_KBD_S,            FALSE );
    AddMapping( INPUT_PLATFORM_PC, STRAFE_LEFT,          INPUT_KBD_A,            FALSE );
    AddMapping( INPUT_PLATFORM_PC, STRAFE_RIGHT,         INPUT_KBD_D,            FALSE );
                                                                                 
    AddMapping( INPUT_PLATFORM_PC, ACTION_JUMP,          INPUT_KBD_SPACE,        TRUE );
    AddMapping( INPUT_PLATFORM_PC, ACTION_CROUCH,        INPUT_KBD_LCONTROL,     TRUE );
                                                                                 
    AddMapping( INPUT_PLATFORM_PC, LEAN_LEFT,            INPUT_KBD_Q,            TRUE );
    AddMapping( INPUT_PLATFORM_PC, LEAN_RIGHT,           INPUT_KBD_E,            TRUE );
                                                                                 
    //Buttons                                                                    
    AddMapping( INPUT_PLATFORM_PC, ACTION_RELOAD,        INPUT_KBD_R,            TRUE );
    AddMapping( INPUT_PLATFORM_PC, ACTION_THROW_GRENADE, INPUT_KBD_G,            TRUE );
    AddMapping( INPUT_PLATFORM_PC, ACTION_USE,           INPUT_KBD_TAB,          TRUE );
    AddMapping( INPUT_PLATFORM_PC, ACTION_MUTATION,      INPUT_KBD_X,            TRUE );
    AddMapping( INPUT_PLATFORM_PC, ACTION_MELEE_ATTACK,  INPUT_KBD_V,            TRUE );
    AddMapping( INPUT_PLATFORM_PC, ACTION_FLASHLIGHT,    INPUT_KBD_F,            TRUE );
                                                                                 
    //UI                                                                         
    AddMapping( INPUT_PLATFORM_PC, ACTION_HUD_CONTEXT,   INPUT_KBD_F1,           TRUE );
    AddMapping( INPUT_PLATFORM_PC, ACTION_PAUSE_CONTEXT, INPUT_KBD_ESCAPE,       TRUE );
#endif

#ifdef TARGET_PS2
    // Set the default controler
    // Left Analog
    AddMapping( INPUT_PLATFORM_PS2, MOVE_STRAFE,            INPUT_PS2_STICK_LEFT_X, FALSE );
    AddMapping( INPUT_PLATFORM_PS2, MOVE_FOWARD_BACKWARDS,  INPUT_PS2_STICK_LEFT_Y, FALSE );
    AddMapping( INPUT_PLATFORM_PS2, ACTION_CHAT,            INPUT_PS2_BTN_L_STICK, TRUE   );  

    // Right Analog
    AddMapping( INPUT_PLATFORM_PS2, LOOK_HORIZONTAL,        INPUT_PS2_STICK_RIGHT_X, FALSE );
    AddMapping( INPUT_PLATFORM_PS2, LOOK_VERTICAL,          INPUT_PS2_STICK_RIGHT_Y, FALSE );

    // Shoulder buttons
    AddMapping( INPUT_PLATFORM_PS2, ACTION_PRIMARY,         INPUT_PS2_BTN_R1,       TRUE );
    AddMapping( INPUT_PLATFORM_PS2, ACTION_SECONDARY,       INPUT_PS2_BTN_R2,       TRUE );
    AddMapping( INPUT_PLATFORM_PS2, ACTION_JUMP,            INPUT_PS2_BTN_L1,       TRUE );
    AddMapping( INPUT_PLATFORM_PS2, ACTION_CROUCH,          INPUT_PS2_BTN_L2,       TRUE );
    
    // Buttons
    AddMapping( INPUT_PLATFORM_PS2, ACTION_RELOAD,          INPUT_PS2_BTN_SQUARE,   TRUE );
    AddMapping( INPUT_PLATFORM_PS2, ACTION_USE,             INPUT_PS2_BTN_CROSS,    TRUE );
    AddMapping( INPUT_PLATFORM_PS2, ACTION_FLASHLIGHT,      INPUT_PS2_BTN_CIRCLE,   TRUE );

    // D - Pad
    AddMapping( INPUT_PLATFORM_PS2, ACTION_SPEAK_FOLLOW_STAY,   INPUT_PS2_BTN_L_UP,     TRUE );
    AddMapping( INPUT_PLATFORM_PS2, ACTION_SPEAK_USE_ACTIVATE,  INPUT_PS2_BTN_L_LEFT,   TRUE );
    AddMapping( INPUT_PLATFORM_PS2, ACTION_SPEAK_COVER_ME,      INPUT_PS2_BTN_L_RIGHT,  TRUE );
    AddMapping( INPUT_PLATFORM_PS2, ACTION_SPEAK_ATTACK_COVER,  INPUT_PS2_BTN_L_DOWN,   TRUE );

    AddMapping( INPUT_PLATFORM_PS2, ACTION_HUD_CONTEXT,             INPUT_PS2_BTN_L2,       TRUE );
    AddMapping( INPUT_PLATFORM_PS2, ACTION_PAUSE_CONTEXT,           INPUT_PS2_BTN_START,    TRUE );

    AddMapping( INPUT_PLATFORM_PS2, ACTION_HUD_MOVEMENT_HORIZONTAL, INPUT_PS2_STICK_LEFT_X, FALSE );
    AddMapping( INPUT_PLATFORM_PS2, ACTION_HUD_MOVEMENT_VERTICAL,   INPUT_PS2_STICK_LEFT_Y, FALSE );

    AddMapping( INPUT_PLATFORM_PS2, LEAN_LEFT, INPUT_PS2_BTN_L_LEFT, TRUE);
    AddMapping( INPUT_PLATFORM_PS2, LEAN_RIGHT, INPUT_PS2_BTN_L_RIGHT, TRUE);

    AddMapping( INPUT_PLATFORM_PS2, ACTION_TALK_MODE_TOGGLE, INPUT_PS2_BTN_SELECT, TRUE );

#endif
}

//=========================================================================

void ingame_pad::OnUpdate( f32 DeltaTime )
{
#if defined(TARGET_XBOX)
    s32 iPlatform = INPUT_PLATFORM_XBOX;
#elif defined(TARGET_PS2)
    s32 iPlatform = INPUT_PLATFORM_PS2;
#elif defined(TARGET_PC)
    s32 iPlatform = INPUT_PLATFORM_PC;
#endif
    //
    // First lets read all the input what we need
    //
    input_pad::OnUpdate( iPlatform, DeltaTime );
    // Nothing to do for now
    //OnDebugRender();
}

//===========================================================================

const char* ingame_pad::GetLogicalIDName( s32 Index )
{
    // Which state?
    switch( Index )
    {
        default:
            ASSERTS(0, "Add your new state to this list or properties will not work!");


        case MOVE_STRAFE:               return "Strafe";  //Deprecated
        case MOVE_FOWARD_BACKWARDS:     return "Move";    //Deprecated
        case MOVE_FORWARD:              return "Move";
        case MOVE_BACKWARD:             return "Move";
        case STRAFE_LEFT:               return "Strafe";
        case STRAFE_RIGHT:              return "Strafe";
        case LOOK_HORIZONTAL:           return "Look Horiz";
        case LOOK_VERTICAL:             return "Look Vert";
        case LEAN_LEFT:                 return "Lean Left";
        case LEAN_RIGHT:                return "Lean Right";
        case ACTION_JUMP:               return "Jump";
        case ACTION_CROUCH:             return "Crouch";
        case ACTION_PRIMARY:            return "Primary Fire";
        case ACTION_SECONDARY:          return "Secondary Fire";
        case ACTION_RELOAD:             return "Reload";
        case ACTION_MUTATION:           return "Mutation";
        case ACTION_FIRE_PARASITES:     return "Fire Parasites";
        case ACTION_FIRE_CONTAGION:     return "Fire Contagion";
        case ACTION_MUTANT_MELEE:       return "Mutant Melee";
        case ACTION_CYCLE_RIGHT:        return "Cycle Right";
        case ACTION_CYCLE_LEFT:         return "Cycle Left";
        case ACTION_USE:                return "Use";
        case ACTION_FLASHLIGHT:         return "Flashlight";
        case ACTION_TOGGLE_PRECISE_AIM: return "Toggle Precise Aim";
        case ACTION_MP_FLASHLIGHT:      return "Multiplayer Flashlight";
        case ACTION_MP_MUTATE:          return "Multiplayer Mutation";
        case ACTION_DROP_FLAG:          return "Drop Flag";


        // Friendly interaction controls.
        case ACTION_SPEAK_FOLLOW_STAY:  return "Talk: Follow";
        case ACTION_SPEAK_USE_ACTIVATE: return "Talk: Activate";
        case ACTION_SPEAK_COVER_ME:     return "Talk: Cover";
        case ACTION_SPEAK_ATTACK_COVER: return "Talk: Attack";

        //Interface controls
        case ACTION_HUD_CONTEXT:            return "";
        case ACTION_HUD_MOVEMENT_HORIZONTAL:return "";
        case ACTION_HUD_MOVEMENT_VERTICAL:  return "";
        case ACTION_HUD_SET_HOTKEY_0:       return "Set Hkey 0";
        case ACTION_HUD_SET_HOTKEY_1:       return "Set Hkey 1";

        // Hot key stuff
        case ACTION_USE_HOTKEY_0:           return "Use Hkey 0";
        case ACTION_USE_HOTKEY_1:           return "Use Hkey 1";
        case ACTION_PAUSE_CONTEXT:          return "Pause";
        case ACTION_FRONTEND_CONTEXT:       return "";
        case ACTION_RIFT:                   return "";

        case ACTION_WEAPON_ITEM_SWITCH:     return "";
        case ACTION_THROW_GRENADE:          return "";
        case ACTION_CYCLE_GRENADE_TYPE:     return "";
        case ACTION_MELEE_ATTACK:           return "";

        case ACTION_TALK_MODE_TOGGLE:       return "Talk: Mode Toggle";
        case ACTION_VOTE_MENU_ON:           return "Vote: Menu On";
        case ACTION_VOTE_MENU_OFF:          return "Vote: Menu Off";
        case ACTION_VOTE_YES:               return "Vote: Yes";
        case ACTION_VOTE_NO:                return "Vote: No";
        case ACTION_VOTE_ABSTAIN:           return "Vote: Abstain";

        case ACTION_CHAT:                   return "Voice Chat";

    }
}

//===========================================================================

const char* ingame_pad::GetLogicalIDEnum( void )
{
    // Build enum list
    static char s_Enum[1024] = {0};
        
    // Already built?
    if (s_Enum[0])
        return s_Enum;

    // Add all states to enum
    char* pDest = s_Enum;
    for (s32 i = 0; i < MAX_ACTION; i++)
    {
        // Lookup state name
        const char* pState = GetLogicalIDName(i);

        // Add to enum list
        x_strcpy(pDest, pState);

        // Next
        pDest += x_strlen(pState)+1;
    }

    // Make sure we didn't overrun the array!
    ASSERT(pDest <= &s_Enum[1024]);

    // Done
    return s_Enum;
}

//=========================================================================

ingame_pad::logical_id ingame_pad::GetLogicalIDByName( const char* pName )
{
    // Check all states
    for (s32 i = 0; i < MAX_ACTION; i++)
    {
        // Found?
        if (x_stricmp(pName, GetLogicalIDName(i)) == 0)
            return (ingame_pad::logical_id)i;
    }

    // Not found
    return ACTION_NULL;
}