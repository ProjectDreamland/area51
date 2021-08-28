//==============================================================================
//
//  Monkey.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Monkey.hpp"
#include "GamePad.hpp"

#ifndef CONFIG_RETAIL
#include "Gamelib/DebugCheats.hpp"
#endif


//==============================================================================
// EXTERNS
//==============================================================================

extern s32 x_GetMainThreadID    (void);

//==============================================================================

monkey g_Monkey;

//==============================================================================

#define CHANCE(Percent)  ((x_frand(0.00000001f,100) <= Percent) ? 1.0f : 0.0f)

//==============================================================================
//  CONSTANTS
//==============================================================================

const f32 k_TimeBetweenMonkeyModesMin = 10.f;
const f32 k_TimeBetweenMonkeyModesMax = 20.f;
const f32 k_MutationModeTime          = 30.f;

//==============================================================================
//  LOCAL DATA STRUCTURES
//==============================================================================

struct virtual_control_entry
{
    ingame_pad::logical_id      LogicalID;                          // represents a logical button press (jump, fire, etc.)    
    f32                         PercentChoice[k_NumMonkeyModes];    // what percentage of the time should we press this button?    
};

struct stick_data
{
    monkey_mode                 Mode;    
    f32                         MoveMaxX;                           // max X and Y values for left stick (movement)
    f32                         MoveMaxY;        
    f32                         LookMaxX;                           // max X and Y values for right stick (camera look / rotation)
    f32                         LookMaxY;
    f32                         MinChangeTime;                      // min/max time for updating stick direction
    f32                         MaxChangeTime;
    f32                         MaxPitchDegrees;                    // how much is monkey allowed to aim up / down
    xbool                       bAimAtNearbyHostiles;               // if set to true, settings will be overridden to aim at nearby hostile characters
};

//==============================================================================
//  STORAGE
//==============================================================================

stick_data MonkeyStickData[] =
{
    // mode                 MoveMaxX    MoveMaxY    LookMaxX    LookMaxY    MinChangeTime   MaxChangeTime   MaxPitchDegrees bAimAtNearbyHostiles
    { MONKEY_NORMAL,        1.0f,       1.0f,       1.0f,       1.0f,       1.5f,           3.5f,           10.f,           true  },
    { MONKEY_JUMPMAN,       1.0f,       1.0f,       1.0f,       1.0f,       1.5f,           3.5f,           10.f,           false },  // don't look up or down too much
    { MONKEY_CROUCHMAN,     1.0f,       1.0f,       1.0f,       1.0f,       1.5f,           3.5f,           10.f,           false },
    { MONKEY_GUNMAN,        1.0f,       1.0f,       1.0f,       1.0f,       1.5f,           3.5f,           25.f,           true  },
    { MONKEY_GRENADIER,     1.0f,       1.0f,       1.0f,       1.0f,       1.5f,           3.5f,           25.f,           true  },
    { MONKEY_MUTATION,      1.0f,       1.0f,       1.0f,       1.0f,       1.5f,           3.5f,           10.f,           true  },
    { MONKEY_MENUMONKEY,    1.0f,       1.0f,       1.0f,       1.0f,       1.5f,           3.5f,           10.f,           false },
    { MONKEY_TWITCH,        1.0f,       1.0f,       1.0f,       1.0f,       0.1f,           0.2f,           90.f,           false },  // change stick direction frequently
    { MONKEY_MEMHOG,        1.0f,       1.0f,       1.0f,       1.0f,       1.5f,           3.5f,           10.f,           true  },
};

virtual_control_entry MonkeyButtonPercentages[] = 
{    
    //  logical pad id                                normal     jumpman    crouchman      gunman   grenadier    mutation   menumonkey  twitch		memhog
    { ingame_pad::ACTION_JUMP,                      {  1.0f,      50.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       5.0f,	0.5f   } },
    { ingame_pad::ACTION_CROUCH,                    {  1.0f,       0.0f,      100.0f,       1.0f,       1.0f,       1.0f,       1.0f,       5.0f,	0.5f   } },    
    { ingame_pad::ACTION_RELOAD,                    {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       5.0f,	0.0f   } },       
    { ingame_pad::ACTION_USE,                       {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       5.0f,	0.0f   } },
    { ingame_pad::ACTION_FLASHLIGHT,                {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       5.0f,	0.0f   } },
    { ingame_pad::ACTION_MP_FLASHLIGHT,             {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       5.0f,	0.0f   } },
    
    // attacks
    { ingame_pad::ACTION_PRIMARY,                   { 10.0f,      10.0f,       10.0f,      50.0f,      20.0f,      20.0f,       1.0f,       5.0f,	0.0f   } },
    { ingame_pad::ACTION_SECONDARY,                 {  2.0f,       2.0f,        2.0f,       5.0f,       5.0f,      10.0f,       1.0f,       1.0f,	0.0f   } },
    { ingame_pad::ACTION_THROW_GRENADE,             {  0.0f,       0.0f,        0.0f,       0.0f,       2.0f,       0.0f,       0.0f,       1.0f,  13.0f   } },
    { ingame_pad::ACTION_MELEE_ATTACK,              {  1.0f,       1.0f,        0.0f,       1.0f,       1.0f,       1.0f,       1.0f,       5.0f,   0.0f   } },            

    // weapon switching
    { ingame_pad::ACTION_CYCLE_GRENADE_TYPE,        {  0.5f,       0.5f,        0.5f,       0.5f,       0.5f,       0.5f,       0.5f,       1.0f,	0.0f   } },
    { ingame_pad::ACTION_WEAPON_ITEM_SWITCH,        {  0.5f,       0.5f,        0.5f,       0.5f,       0.5f,       0.5f,       0.5f,       1.0f,	0.0f   } },      
    { ingame_pad::ACTION_CYCLE_LEFT,                {  0.5f,       0.5f,        0.5f,       0.5f,       0.5f,       0.5f,       0.5f,       1.0f,	0.0f   } },              
    { ingame_pad::ACTION_CYCLE_RIGHT,               {  0.5f,       0.5f,        0.5f,       0.5f,       0.5f,       0.5f,       0.5f,       1.0f,	0.0f   } },
    
    // mutation & mutant attacks
    { ingame_pad::ACTION_MUTATION,                  {  0.1f,       0.0f,        0.0f,       0.0f,       0.0f,       1.0f,       0.0f,       1.0f,	3.0f   } },    
    { ingame_pad::ACTION_MP_MUTATE,                 {  0.1f,       0.0f,        0.0f,       0.0f,       0.0f,       1.0f,       0.0f,       1.0f,	3.0f   } },    
    { ingame_pad::ACTION_FIRE_PARASITES,            {  1.0f,       0.0f,        0.0f,       0.0f,       0.0f,       2.0f,       0.0f,       5.0f,  20.0f   } },
    { ingame_pad::ACTION_FIRE_CONTAGION,            {  1.0f,       0.0f,        0.0f,       0.0f,       0.0f,       2.0f,       0.0f,       5.0f,	1.0f   } },
    { ingame_pad::ACTION_MUTANT_MELEE,              {  1.0f,       0.0f,        0.0f,       0.0f,       0.0f,       2.0f,       0.0f,       5.0f,	0.0f   } },
    
    //Interface controls -- not actually used!
    { ingame_pad::ACTION_HUD_CONTEXT,               {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       5.0f,       0.0f,   0.0f   } },             
    { ingame_pad::ACTION_HUD_MOVEMENT_HORIZONTAL,   {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,      10.0f,       0.0f,   0.0f   } }, 
    { ingame_pad::ACTION_HUD_MOVEMENT_VERTICAL,     {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,      10.0f,       0.0f,   0.0f   } },   
    { ingame_pad::ACTION_HUD_SET_HOTKEY_0,          {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,      10.0f,       0.0f,   0.0f   } },        
    { ingame_pad::ACTION_HUD_SET_HOTKEY_1,          {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,      10.0f,       0.0f,   0.0f   } },    
    { ingame_pad::ACTION_USE_HOTKEY_0,              {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },            
    { ingame_pad::ACTION_USE_HOTKEY_1,              {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },                
    { ingame_pad::ACTION_FRONTEND_CONTEXT,          {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,      10.0f,       0.0f,   0.0f   } },        
    { ingame_pad::ACTION_PAUSE_CONTEXT,             {  0.0f,       0.0f,        0.0f,       0.0f,       0.0f,       0.0f,       5.0f,       0.0f,   0.0f   } },

    { ingame_pad::ACTION_RIFT,                      {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },                    

    // toggle precise aim seems to switch weapons
    { ingame_pad::ACTION_TOGGLE_PRECISE_AIM,        {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },      

    { ingame_pad::ACTION_VOTE_MENU_ON,              {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },
    { ingame_pad::ACTION_VOTE_MENU_OFF,             {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },
    { ingame_pad::ACTION_VOTE_YES,                  {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },
    { ingame_pad::ACTION_VOTE_NO,                   {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },
    { ingame_pad::ACTION_VOTE_ABSTAIN,              {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },

    { ingame_pad::ACTION_CHAT,                      {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },
    { ingame_pad::ACTION_TALK_MODE_TOGGLE,          {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },

    // Friendly interaction controls.
    { ingame_pad::ACTION_SPEAK_FOLLOW_STAY,         {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },       
    { ingame_pad::ACTION_SPEAK_USE_ACTIVATE,        {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },      
    { ingame_pad::ACTION_SPEAK_COVER_ME,            {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },          
    { ingame_pad::ACTION_SPEAK_ATTACK_COVER,        {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } },      

    { ingame_pad::LEAN_LEFT,                        {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,   0.0f   } },
    { ingame_pad::LEAN_RIGHT,                       {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       1.0f,   0.0f   } },

    { ingame_pad::ACTION_DROP_FLAG,                 {  1.0f,       1.0f,        1.0f,       1.0f,       1.0f,       1.0f,       1.0f,       0.0f,   0.0f   } }
};

//==============================================================================

monkey_options g_MonkeyOptions = 
{    
    FALSE,  // xbool    Enabled;
    {   
#if defined(sbroumley)    
        TRUE,   //ModeEnabled[MONKEY_NORMAL]        
        TRUE,   //ModeEnabled[MONKEY_JUMPMAN]       
        TRUE,   //ModeEnabled[MONKEY_CROUCHMAN]     
        TRUE,   //ModeEnabled[MONKEY_GUNMAN]        
        TRUE,   //ModeEnabled[MONKEY_GRENADIER]       
        TRUE,   //ModeEnabled[MONKEY_MUTATION]        
        FALSE,  //ModeEnabled[MONKEY_MENUMONKEY] -- doesn't do anything except pause the game right now
        TRUE,   //ModeEnabled[MONKEY_TWITCH]
        TRUE,   //ModeEnabled[MONKEY_MEMHOG]
#else

        TRUE,   //ModeEnabled[MONKEY_NORMAL]        
        FALSE,  //ModeEnabled[MONKEY_JUMPMAN]       
        FALSE,  //ModeEnabled[MONKEY_CROUCHMAN]     
        FALSE,  //ModeEnabled[MONKEY_GUNMAN]        
        FALSE,  //ModeEnabled[MONKEY_GRENADIER]       
        FALSE,  //ModeEnabled[MONKEY_MUTATION]        
        FALSE,  //ModeEnabled[MONKEY_MENUMONKEY] -- doesn't do anything except pause the game right now
        FALSE,  //ModeEnabled[MONKEY_TWITCH]
        FALSE,  //ModeEnabled[MONKEY_MEMHOG]
#endif
        
    },
    FALSE,   // xbool    Dirty;
    FALSE    // xbool    bTestOutOfWorld;
};

//==============================================================================

//====================================================[dkalina 01/05/05]======== 
//
// class:  monkey 
//      used by InputMgr for diagnostic button mashing
//      monkey is enabled via debug menu in non-release builds
//
//============================================================================== 

monkey::monkey() 
    : m_RetVal(0.0f)
    , m_CurrentMonkeyMode(0)    
    , m_MonkeyPadTimer(0.f)
    , m_MonkeyModeTimer(k_TimeBetweenMonkeyModesMin)        // initialize to min value so that we don't immediately switch modes
    , m_PlayerPosition(0,0,0)
    , m_PlayerPitch(0.0f)
    , m_PlayerYaw(0.0f)
    , m_PlayerInvertY(TRUE)
    , m_bFastMenuMonkey(FALSE)
    , m_bExitingMenuMonkeyMode(FALSE)
    , m_bIsMutant(FALSE)
    , m_ClosestHostilePosition(0,0,0)        
{}

monkey::~monkey() {}



//====================================================[dkalina 01/05/05]======== 
//
// IsLookingUp - return true if y-stick is being held in such a way to pitch
//      the player's view upwards, based on current invert-y config setting
//
//============================================================================== 

xbool monkey::IsLookingUp() const
{
    if ( m_PlayerInvertY && m_MonkeyPadData.pad_r_y < 0.f )
        return TRUE;

    if ( !m_PlayerInvertY && m_MonkeyPadData.pad_r_y > 0.f )
        return TRUE;

    return FALSE;
}

//====================================================[dkalina 01/05/05]======== 
//
// Update - called 1x / frame; updates any internal timers for the monkey class
//
//============================================================================== 

static f32 s_FastMenuMonkeyTimer = 15.f;
static f32 s_MutationModeTimer   = k_MutationModeTime;

void monkey::Update( f32 DeltaTime )
{   
    // SB: 2/22/05
    // Make sure we only happen in the main thread or we get VU0 corruption crashes during loading
    if( x_GetThreadID() != x_GetMainThreadID() )
        return;

    if ( AreAllModesDisabled() )
        return;

    // update mutation mode timer -- will be reset elsewhere
    s_MutationModeTimer -= DeltaTime;

    // update menu monkey timer
    if ( m_CurrentMonkeyMode == MONKEY_MENUMONKEY )
    {
        s_FastMenuMonkeyTimer -= DeltaTime;
        if ( s_FastMenuMonkeyTimer < 0.f )
        {
            m_bFastMenuMonkey = !m_bFastMenuMonkey;
            s_FastMenuMonkeyTimer = 15.0f;
        }
    }

    const stick_data& StickData = MonkeyStickData[m_CurrentMonkeyMode];

    // get the player's current view pitch in degrees
    f32 PitchDegrees = -1.f * RAD_TO_DEG(m_PlayerPitch);        // NOTE:  pitch is 'backwards' -- i.e., negative pitch is camera facing upwards

    // when timer expires, update return values for analog sticks and reset timer
    m_MonkeyPadTimer -= DeltaTime;
    if ( m_MonkeyPadTimer < 0.f )
    {
        m_MonkeyPadTimer = x_frand( StickData.MinChangeTime, StickData.MaxChangeTime );

        m_MonkeyPadData.pad_l_x = x_frand( -1.0f * StickData.MoveMaxX, 1.0f * StickData.MoveMaxX);
        m_MonkeyPadData.pad_l_y = x_frand( -1.0f * StickData.MoveMaxY, 1.0f * StickData.MoveMaxY);
        m_MonkeyPadData.pad_r_x = x_frand( -1.0f * StickData.LookMaxX, 1.0f * StickData.LookMaxX);
        m_MonkeyPadData.pad_r_y = x_frand( -1.0f * StickData.LookMaxY, 1.0f * StickData.LookMaxY);

        // if we're already exceeding the maximum allowed pitch by more than some allowed threshold, push back the other way
        const f32 AdjustedMaxPitch = StickData.MaxPitchDegrees + 5.f;
                
        if ( (PitchDegrees > AdjustedMaxPitch) && IsLookingUp() )
            m_MonkeyPadData.pad_r_y *= -1.f;
        else if ( PitchDegrees < (-1.f * AdjustedMaxPitch) && !IsLookingUp() )
            m_MonkeyPadData.pad_r_y *= -1.f;
    }
    
    // clamp pad_r_y to 0 if we've exceeded the maximum pitch    
    if ( (PitchDegrees > StickData.MaxPitchDegrees) && IsLookingUp() )
        m_MonkeyPadData.pad_r_y = 0.f;
    else if ( (PitchDegrees < (-1.f * StickData.MaxPitchDegrees) ) && !IsLookingUp() )
        m_MonkeyPadData.pad_r_y = 0.f;

    // is there a closeby hostile character to aim at?
    if ( StickData.bAimAtNearbyHostiles && m_ClosestHostilePosition.Length() > 0.00001f )
    {
        vector3 DirToHostile = m_ClosestHostilePosition - m_PlayerPosition;
        DirToHostile.Normalize();

        vector3 ForwardDir(m_PlayerPitch, m_PlayerYaw);
        ForwardDir.Normalize();

        vector3 ForwardDirNoY(ForwardDir);
        ForwardDirNoY.GetY() = 0.f;
        ForwardDirNoY.Normalize();

        f32 yaw_dot_to_hostile = ForwardDirNoY.Dot(DirToHostile);
        vector3 cross1         = ForwardDirNoY.Cross(DirToHostile);
        radian RotAngleYaw     = x_acos(yaw_dot_to_hostile) * (cross1.GetY() < 0.f ? 1.f : -1.f);

        vector3 DirToHostileNoY(DirToHostile);
        DirToHostileNoY.GetY() = ForwardDir.GetY();
        DirToHostileNoY.Normalize();

        f32 pitch_dot_to_hostile = DirToHostileNoY.Dot(DirToHostile);        
        radian RotAnglePitch     = x_acos(pitch_dot_to_hostile) * (DirToHostileNoY.GetY() > DirToHostile.GetY() ? -1.0f : 1.f);

        // modify aim yaw based on calculated angle difference
        static f32 MaxYawScalar = 1.0f;
        static f32 MinYawScalar = 0.7f;
        f32 PadYawScalar = (x_abs(RotAngleYaw) > 1.f) ? (x_sign(RotAngleYaw) * MaxYawScalar) : (MaxYawScalar * (RotAngleYaw / 1.f));
        if ( PadYawScalar > 0.f && PadYawScalar < MinYawScalar )
            PadYawScalar = MinYawScalar;
        else if ( PadYawScalar < 0.f && (PadYawScalar > (-1.f * MinYawScalar)) )
            PadYawScalar = -1.f * MinYawScalar;
        
        // apply yaw scalar to aim left/right at target
        if ( x_abs(RotAngleYaw) > DEG_TO_RAD(1.f) )
            m_MonkeyPadData.pad_r_x = PadYawScalar;        
        else
            m_MonkeyPadData.pad_r_x = 0.0f;

        // modify aim pitch
        static f32 MaxPitchScalar = 1.0f;
        static f32 MinPitchScalar = 0.7f;
        f32 PadPitchScalar = (x_abs(RotAnglePitch) > 1.f) ? MaxPitchScalar : (MaxPitchScalar * (RotAnglePitch / 1.f));
        if ( PadPitchScalar > 0.f && PadPitchScalar < MinPitchScalar )
            PadPitchScalar = MinPitchScalar;
        else if ( PadPitchScalar < 0.f && (PadPitchScalar > (-1.f * MinPitchScalar)) )
            PadPitchScalar = -1.f * MinPitchScalar;

        // apply pitch scalar to aim up/down at target.  make sure to invert value if controls are inverted.
        if ( x_abs(RotAnglePitch) > DEG_TO_RAD(5.f) )
            m_MonkeyPadData.pad_r_y = PadPitchScalar * (m_PlayerInvertY ? -1.f : 1.f);
        else
            m_MonkeyPadData.pad_r_y = 0.0f;
    }
    
    // is it time to update the monkey mode?  (either timer expired or menu changed current mode options)
    m_MonkeyModeTimer -= DeltaTime;
    if ( m_MonkeyModeTimer < 0.f || (g_MonkeyOptions.Dirty && !g_MonkeyOptions.ModeEnabled[m_CurrentMonkeyMode]) )
    {
        g_MonkeyOptions.Dirty   = false;
        s32 NewMode             = ChooseMode();

        if ( NewMode != MONKEY_MENUMONKEY && m_CurrentMonkeyMode == MONKEY_MENUMONKEY )
            m_bExitingMenuMonkeyMode = TRUE;
        
        m_CurrentMonkeyMode     = NewMode;
        m_MonkeyModeTimer       = x_frand(k_TimeBetweenMonkeyModesMin, k_TimeBetweenMonkeyModesMax);
    }
}

//====================================================[dkalina 01/05/05]======== 
//
// ChooseMode - Choose mode randomly from available modes
// 
// Returns:  
//      index of an enabled monkey mode
// 
//============================================================================== 

s32 monkey::ChooseMode() const
{
    ASSERT( AreAllModesDisabled() == FALSE );

    s32 nActiveModes = 0;
    for ( s32 i=0; i < k_NumMonkeyModes; i++ )
    {
        if ( g_MonkeyOptions.ModeEnabled[i] )
            nActiveModes++;
    }

    s32 choice = x_rand() % nActiveModes;
    
    // iterate through all available modes until we reach the choice-th 
    s32 j = 0;
    for ( s32 i=0; i < k_NumMonkeyModes; i++ )
    {
        if ( g_MonkeyOptions.ModeEnabled[i] )
        {
            j++;
            if ( j > choice )
                return i;
        }
    }
  
    return 0;
}

//====================================================[dkalina 01/05/05]======== 
//
// Returns true if no monkey modes are currently available as toggled on / off by the menu)
//
//============================================================================== 

xbool monkey::AreAllModesDisabled() const
{
    for ( s32 i = 0; i < k_NumMonkeyModes; i++ )
    {
        if ( g_MonkeyOptions.ModeEnabled[i] == TRUE )
            return FALSE;
    }

    return TRUE;
}


//====================================================[dkalina 01/05/05]======== 
//
// GetValue - input system calls into this function when the monkey is active 
//      and it wants a fake value for some logical button press
//
// Returns
//      0 or 1 for pad buttons
//      [0..1] for analog inputs
//
//============================================================================== 

f32 monkey::GetValue( s32 LogicalPadMapping )
{   
    if ( AreAllModesDisabled() )
        return 0.f;

    // if logical pad mapping corresponds to an analog stick, use stick data
    switch ( LogicalPadMapping )
    {
        case ingame_pad::MOVE_STRAFE : 
            return ( m_MonkeyPadData.pad_l_x );            

        case ingame_pad::MOVE_FOWARD_BACKWARDS :
            return ( m_MonkeyPadData.pad_l_y );            

        case ingame_pad::ACTION_HUD_MOVEMENT_HORIZONTAL :
        case ingame_pad::LOOK_HORIZONTAL :
            return ( m_MonkeyPadData.pad_r_x );

        case ingame_pad::ACTION_HUD_MOVEMENT_VERTICAL :
        case ingame_pad::LOOK_VERTICAL : 
            return ( m_MonkeyPadData.pad_r_y );
    }

    // if mutated and in memory hog mode, don't press the mutation buttons again
    if ( m_CurrentMonkeyMode == MONKEY_MEMHOG && (LogicalPadMapping == ingame_pad::ACTION_MUTATION || LogicalPadMapping == ingame_pad::ACTION_MP_MUTATE) )
    {   
        // every 15 seconds, hit one of the mutation buttons regardless of state until out of mutant mode
        if ( s_MutationModeTimer < 0.f )
        {       
            // only reset the timer when no longer a mutant!
            if ( !m_bIsMutant )
            {
                s_MutationModeTimer = k_MutationModeTime;
                return 0.f;
            }
            return 1.f;
        }

        // if player is mutated, typically we do not want to press the mutation button
        if ( m_bIsMutant )
            return 0.f;
    }
    
    // if input logical pad mapping corresponds to an entry in the MonkeyButtonPercentages table, roll the dice according to the table value for the current mode
    s32 nEntries = (sizeof(MonkeyButtonPercentages) / sizeof(virtual_control_entry));
    for ( s32 i=0; i < nEntries; i++ )
    {
        virtual_control_entry& entry = MonkeyButtonPercentages[i];

        if ( entry.LogicalID == LogicalPadMapping )
        {
            return CHANCE( entry.PercentChoice[m_CurrentMonkeyMode] );
        }
    }

    return 0.f;
}



//====================================================[dkalina 01/07/05]======== 
//
// SetPlayerInfo - called by Player class to inform monkey of associated data
//
//============================================================================== 


void monkey::SetPlayerInfo( const vector3& player_pos, const radian& player_pitch, const radian& player_yaw, xbool player_invert_y, const vector3& closest_hostile_pos, xbool is_mutant )
{
    m_PlayerPosition            = player_pos;
    m_PlayerPitch               = player_pitch;
    m_PlayerYaw                 = player_yaw;
    m_PlayerInvertY             = player_invert_y;
    m_ClosestHostilePosition    = closest_hostile_pos;
    m_bIsMutant                 = is_mutant;
}


//====================================================[dkalina 01/12/05]======== 
//
// GetCurrentMode - returns the currently selected monkey mode
//
//============================================================================== 

s32 monkey::GetCurrentMode() const
{
    return m_CurrentMonkeyMode;
}

//====================================================[dkalina 01/06/05]======== 
//
// ShouldPause - special call to see if monkey should pause the game
//
//============================================================================== 

static f32 s_ShouldPausePercentChance = 2.5f;

xbool monkey::ShouldPause() const
{   
    if ( m_CurrentMonkeyMode == MONKEY_MENUMONKEY )
    {
        return ( ( CHANCE(s_ShouldPausePercentChance) > 0.0f ) ? TRUE : FALSE );
    }

    return FALSE;
}

//====================================================[dkalina 01/17/05]======== 
//
// ShouldUnpause - special call to see if monkey should unpause the game
//      after exiting the menu-monkey mode
//
//============================================================================== 

xbool monkey::ShouldUnpause()
{
    // if monkey was just in MenuMonkey mode, return true
    if ( m_bExitingMenuMonkeyMode )
    {
        m_bExitingMenuMonkeyMode = false;
        return TRUE;
    }

    return FALSE;
}

//====================================================[dkalina 01/11/05]======== 
//
// GetUIButtonValue - special call made by ui_manager to determine button 
//      keypresses in the menumonkey mode (which does not use the normal input path)
//
// Returns true if the input button should be pressed by the monkey.
//
//============================================================================== 


static f32 s_PressArrowButtonChance     = 2.5f; 
static f32 s_PressUIButtonChance        = 1.25f;
static f32 s_PressUIBackButtonChance    = 0.75f;
static f32 s_FastMenuMonkeyMultiplier   = 4.0f;

xbool monkey::GetUIButtonValue( monkey_ui_button button )
{   
    const f32 Multiplier = (m_bFastMenuMonkey ? s_FastMenuMonkeyMultiplier : 1.f);

    switch ( button )
    {
        case MONKEY_UI_UP:
        case MONKEY_UI_DOWN:
        case MONKEY_UI_RIGHT:
        case MONKEY_UI_LEFT:
            return (CHANCE( s_PressArrowButtonChance * Multiplier ) > 0.f);
        case MONKEY_UI_SELECT:        
        case MONKEY_UI_DELETE:
        case MONKEY_UI_ACTIVATE:
            return (CHANCE( s_PressUIButtonChance * Multiplier ) > 0.f);
        case MONKEY_UI_BACK:
            return (CHANCE( s_PressUIBackButtonChance * Multiplier ) > 0.f);
        case MONKEY_UI_SHOULDERL:
        case MONKEY_UI_SHOULDERR:
        case MONKEY_UI_SHOULDERL2:
        case MONKEY_UI_SHOULDERR2:
        case MONKEY_UI_HELP:
        default:
            return (CHANCE( s_PressUIButtonChance * Multiplier ) > 0.f);
    }

    return false;
}

//====================================================[dkalina 01/11/05]======== 
//
// SetAutoMonkeyMode - called when AutoServer is enabled so that monkey data
//      can be automatically activated on game start
//
//============================================================================== 

void monkey::SetAutoMonkeyMode(s32 mode)
{

#ifndef CONFIG_RETAIL
    // always turn on infinite ammo
    if ( mode > 0 )
        DEBUG_INFINITE_AMMO = TRUE;
#endif

    switch ( mode )
    {
        case 0 : break;

        // 1:  enable all monkey modes
        // 3:  enable all monkey modes + out of world detection
        case 1 : 
        case 3 :
        {
            g_MonkeyOptions.Enabled = TRUE;
            for ( s32 i = 0 ; i < k_NumMonkeyModes; i++ )
            {
                g_MonkeyOptions.ModeEnabled[i] = TRUE;
            }
            break;
        }

        // 2:  enable all monkey modes except menumonkey
        // 4:  enable all monkey modes except menumonkey + out of world detection
        case 2 : 
        case 4 :
        {
            g_MonkeyOptions.Enabled = TRUE;
            for ( s32 i = 0 ; i < k_NumMonkeyModes; i++ )
            {
                g_MonkeyOptions.ModeEnabled[i] = TRUE;
            }
            g_MonkeyOptions.ModeEnabled[MONKEY_MENUMONKEY] = FALSE;
            break;
        }
        
        // 5:  enable memhog monkey mode
        case 5 :
        {
        	g_MonkeyOptions.Enabled = true;
            for ( s32 i = 0 ; i < k_NumMonkeyModes; i++ )
            {
                g_MonkeyOptions.ModeEnabled[i] = FALSE;
            }
        	g_MonkeyOptions.ModeEnabled[MONKEY_MEMHOG] = true;
        	break;
        }
    }

    // modes 3 and 4 support out of world detection
    if ( mode == 3 || mode == 4 )
    {
        g_MonkeyOptions.bTestOutOfWorld = TRUE;
    }

    input_SuppressFeedback( g_MonkeyOptions.Enabled );
}