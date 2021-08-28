///////////////////////////////////////////////////////////////////////////
// INCLUDES 
///////////////////////////////////////////////////////////////////////////

#ifdef TARGET_XBOX
#   include "xbox_private.hpp"
#endif
#include "..\e_Engine.hpp"

#define PAD_MAX_CONTROLLERS 4

extern s32 g_uiLastSelectController;
//==============================================================================

#define NUM_PORTS   4
#define NUM_SLOTS   2

char            s_chMUDrives[NUM_PORTS][NUM_SLOTS];
HANDLE          s_hPads[NUM_PORTS];

XINPUT_STATE    s_PadStates[NUM_PORTS][2];
s32             s_PadStatesIndex[NUM_PORTS];
s32             g_LastControllerCount = 0;
//static s32      s_iLowestController = 0;
static xbool    s_bLockedOut = FALSE;

struct DEVICE_STATE {
    XPP_DEVICE_TYPE *pxdt;
    DWORD dwState;
};

static xbool s_FeedbackSwitch[4]={0,0,0,0};
static xbool s_SuppressFeedback = FALSE;

s32 g_RumbleDisabler = TRUE;

DEVICE_STATE s_dsDevices[] =
{
    { XDEVICE_TYPE_GAMEPAD,          0 },
    { XDEVICE_TYPE_MEMORY_UNIT,      0 },
//    { XDEVICE_TYPE_VOICE_MICROPHONE, 0 },
//    { XDEVICE_TYPE_VOICE_HEADPHONE,  0 },
};

#define NUM_DEVICE_STATES   (sizeof(s_dsDevices)/sizeof(*s_dsDevices))

xbool g_NeedAccountEnumeration = TRUE;
//==============================================================================

void HandleDeviceChanges( XPP_DEVICE_TYPE *pxdt, DWORD dwInsert, DWORD dwRemove )
{
    DWORD iPort;
    char szDrive[] = "X:\\";

    // Gamepads
    if( XDEVICE_TYPE_GAMEPAD == pxdt )
    {
        for( iPort = 0; iPort < NUM_PORTS; iPort++ )
        {
            // Close removals.
            if( (1 << iPort & dwRemove) && s_hPads[iPort] )
            {
                XInputClose( s_hPads[iPort] );
                g_LastControllerCount--;
                s_hPads[iPort] = 0;
            }

            // Open insertions.
            if( 1 << iPort & dwInsert )
            {
                s_hPads[iPort] = XInputOpen( pxdt, iPort, XDEVICE_NO_SLOT, NULL );
                g_LastControllerCount++;
            }
        }
    }
    // Memory units
    else if( XDEVICE_TYPE_MEMORY_UNIT == pxdt )
    {
        DWORD iSlot, dwMask;
        for( iPort = 0; iPort < NUM_PORTS; iPort++ )
        {
            for( iSlot = 0; iSlot < NUM_SLOTS; iSlot++ )
            {
                // Get mask from port and slot.
                dwMask = iPort + (iSlot ? 16 : 0);
                dwMask = 1 << dwMask;

                // Unmount removals.
                if( (dwMask & dwRemove) && s_chMUDrives[iPort][iSlot] )
                {
                    XUnmountMU( iPort, iSlot );
                    s_chMUDrives[iPort][iSlot] = 0;
                    g_NeedAccountEnumeration = TRUE;
                }

                // Mount insertions.
                if( dwMask & dwInsert )
                {
                    XMountMU( iPort, iSlot, s_chMUDrives[iPort] + iSlot );
                    s_chMUDrives[iPort][iSlot] = 1;
                    g_NeedAccountEnumeration = TRUE;
                }
            }
        }
    }
}

//==============================================================================

void DeviceInit( void )
{
    XDEVICE_PREALLOC_TYPE xdpt[] =
    {
        {XDEVICE_TYPE_GAMEPAD,          NUM_PORTS},
        {XDEVICE_TYPE_MEMORY_UNIT,      NUM_PORTS*NUM_SLOTS},
        {XDEVICE_TYPE_VOICE_HEADPHONE,  4},
        {XDEVICE_TYPE_VOICE_MICROPHONE, 4},
    };

    // Initialize the peripherals.
    XInitDevices( sizeof(xdpt) / sizeof(XDEVICE_PREALLOC_TYPE), xdpt );

    // Set device handles to invalid.
    ZeroMemory( s_hPads, sizeof(s_hPads) );

    // Set drive letters to invalid.
    ZeroMemory( s_chMUDrives, sizeof(s_chMUDrives) );

    // Set Pad state
    ZeroMemory( s_PadStates, sizeof(s_PadStates) );

    // Initialize PadState indices
    ZeroMemory( s_PadStatesIndex, sizeof(s_PadStatesIndex) );

    // Get initial state of all connected devices.
    for( s32 i = 0; i < NUM_DEVICE_STATES; i++ )
    {
        s_dsDevices[i].dwState = XGetDevices( s_dsDevices[i].pxdt );
        HandleDeviceChanges( s_dsDevices[i].pxdt, s_dsDevices[i].dwState, 0 );
    }
}

//==============================================================================

xbool xbox_CheckDeviceChanges( void )
{
    DWORD dwInsert;
    DWORD dwRemove;
    DWORD Count=0;

    // Check each device type to see if any changes have occurred.
    for( s32 i = 0; i < NUM_DEVICE_STATES; i++ )
    {
        if( XGetDeviceChanges( s_dsDevices[i].pxdt, &dwInsert, &dwRemove ) )
        {
            // Call handler to service the insertion and/or removal.
            HandleDeviceChanges( s_dsDevices[i].pxdt, dwInsert, dwRemove );

            // Update new device state.
            s_dsDevices[i].dwState &= ~dwRemove;
            s_dsDevices[i].dwState |= dwInsert;
            Count++;
        }
    }
    return( Count > 0 );
}

//==============================================================================
//==============================================================================
//==============================================================================

enum feedback_type
{
    RT_NO_RUMBLE,
    RT_INTENSITY,
    RT_ENVELOPE,
    RT_DECAY
};

//==============================================================================

#ifdef X_RETAIL
    #define RUMBLE_DECAY  0.040f
#else
    static f32 RUMBLE_DECAY  = 0.040f;
#endif

//==============================================================================

static struct RumbleMgr
{
    XINPUT_FEEDBACK   Feedback[PAD_MAX_CONTROLLERS];
    feedback_envelope m_Env [PAD_MAX_CONTROLLERS];
    feedback_type     m_Type[PAD_MAX_CONTROLLERS];
    xtimer            m_Timer;

    RumbleMgr( void );
~   RumbleMgr( void );

    void SetEnvelope( u32 Input, feedback_envelope& Env )
    {
        x_memcpy( m_Env + Input, &Env, sizeof( feedback_envelope ));
        m_Env[Input].Duration *= 3000;
    }

    void SetType( u32 Input, feedback_type Type )
    {
        m_Type[Input] = Type;
    }

    void AddIntensity( u32 Input, f32 Value )
    {
        if( !Value )
            m_Env[Input].Intensity = 0.0f;
        else
            m_Env[Input].Intensity += Value * 2;
    }

    void SetDuration( u32 Input, f32 Value )
    {
        m_Env[Input].Duration = Value;
    }

    void Update( void );

} * s_pRumbleMgr = NULL;

//==============================================================================
s32 LastSetStateCount=0;
s32 MaxSetStateCount=-1;

static void SetState( s32 i )
{
    s32 Counter = 0;
    while( s_pRumbleMgr->Feedback[i].Header.dwStatus==ERROR_IO_PENDING )
    {
        Counter++;
        Sleep(0);
    }
    LastSetStateCount=Counter;
    if( Counter > MaxSetStateCount )
    {
        MaxSetStateCount=Counter;
    }

    XInputSetState( s_hPads[i], &s_pRumbleMgr->Feedback[i] );
}

//==============================================================================

void xbox_DisableRumble( void )
{
    if( g_RumbleDisabler )
        return;

    for( s32 i=0;i<PAD_MAX_CONTROLLERS;i++ )
    {
        if( s_hPads[i] )
        {
            s_pRumbleMgr->Feedback[i].Rumble. wLeftMotorSpeed = 0;
            s_pRumbleMgr->Feedback[i].Rumble.wRightMotorSpeed = 0;
            SetState( i );
        }
    }
    g_RumbleDisabler = TRUE;
}

//==============================================================================

void xbox_EnableRumble( void )
{
    g_RumbleDisabler = FALSE;
}

//==============================================================================

void RumbleMgr::Update( void )
{
    if( g_RumbleDisabler )
        return;

    feedback_type rt;
    //
    //  Get delta time
    //
    f32 TimeTaken;
    if( m_Timer.IsRunning( ))   
    {
        TimeTaken = m_Timer.StopMs( );
        m_Timer.Reset( );
    }
    else
        TimeTaken = 0.0f;

    m_Timer.Start( );

    //
    //  Rumble
    //
    for( s32 i=0; i<PAD_MAX_CONTROLLERS; i++ )
    {
        if( s_hPads[i] )
        {
            f32& Intensity = m_Env[i].Intensity;
            f32& Duration  = m_Env[i].Duration ;

            if( (s_FeedbackSwitch[i]) && !(s_SuppressFeedback) )
                rt = m_Type[i];
            else
                rt = RT_NO_RUMBLE;
            
            switch( rt )
            {
                case RT_NO_RUMBLE:
                    Feedback[i].Rumble. wLeftMotorSpeed = 0;
                    Feedback[i].Rumble.wRightMotorSpeed = 0;
                    break;

                case RT_DECAY:
                {   //
                    //  Decay
                    //
                    Intensity -= RUMBLE_DECAY;
                    if( Intensity <= 0.0f )
                    {
                        m_Type[i]=RT_NO_RUMBLE;
                        Intensity = 0.0f;
                        Duration  = 0.0f;
                        break;
                    }
                    //
                    //  Duration
                    //
                    Duration -= TimeTaken/1000;
                    if( Duration <= 0.0f )
                    {
                        m_Type[i]=RT_NO_RUMBLE;
                        Intensity = 0.0f;
                        Duration  = 0.0f;
                    }
                    break;
                }

                case RT_ENVELOPE:
                case RT_INTENSITY:
                {   //
                    //  Right 'tweeter' motor control
                    //
                    f32 RIntensity = 65536.0f * m_Env[i].Intensity;
                    if( RIntensity > 65535.0f )
                        RIntensity = 65535.0f ;
                    if( RIntensity <     0.0f )
                        RIntensity =     0.0f ;
                    //
                    //  Left 'heavy' motor control
                    //
                    f32 LIntensity = 65536.0f * m_Env[i].Intensity;
                    if( LIntensity > 65536.0f )
                        LIntensity = 65536.0f ;
                    if( LIntensity <     0.0f )
                        LIntensity =     0.0f ;
                    //
                    //  Let's get ready to rumble!
                    //
                    Feedback[i].Rumble. wLeftMotorSpeed = u16( LIntensity );
                    Feedback[i].Rumble.wRightMotorSpeed = u16( RIntensity );
                    m_Type[i] = RT_DECAY;
                    break;
                }
            }
            SetState( i );
        }
    }
}

//==============================================================================

RumbleMgr::RumbleMgr( void )
{
    Sleep(250);
    for( s32 i=0; i<PAD_MAX_CONTROLLERS; i++ )
    {
        ZeroMemory( &Feedback[i], sizeof( XINPUT_FEEDBACK ));
        ZeroMemory( m_Env + i, sizeof( feedback_envelope ));
        m_Type[i] = RT_NO_RUMBLE;
    }
    s_pRumbleMgr = this;
}

//==============================================================================

RumbleMgr::~RumbleMgr( void )
{
}

//==============================================================================
//==============================================================================
//==============================================================================

xbool input_UpdateState( void )
{
    static xbool ReturnState = TRUE;
    //
    //  Read from controllers
    //
    {
        // Check for changes in device connectivity
        xbox_CheckDeviceChanges();

        if( ReturnState )
        {
            s32 LeftMostUsed = -1;
            s32 i;

            // Read the pads
            for( i=0 ; i<PAD_MAX_CONTROLLERS ; i++ )
            {
                // If the is connected
                if( s_hPads[i] )
                {
                    s_PadStatesIndex[i] = 1-s_PadStatesIndex[i];
                    s32 Index = s_PadStatesIndex[i];

                    XInputGetState( s_hPads[i], &s_PadStates[i][Index] );
                }
            }
        }

#if 0
        // Find the lowest numbered controller that is present
        s32 i;
        for (i=0; i<PAD_MAX_CONTROLLERS; i++)
        {
            if( input_IsPresent( INPUT_XBOX_QRY_PAD_PRESENT, i ))
            {
                s_iLowestController = i;
                break;
            }
        }

        if (i==NUM_PORTS)
            s_iLowestController = 0;
#endif

        ReturnState = !ReturnState;
        //
        //  Record left most controller input
        //
        if( s_pRumbleMgr )//&& s_bLockedOut )
            s_pRumbleMgr->Update();
    }

    return ReturnState;
}

//==============================================================================

static xbool IsPressedEx( input_gadget GadgetID, s32 ControllerID )
{
    xbool State = FALSE;
    {
        ASSERT(ControllerID != -1);
        ASSERT(ControllerID < NUM_PORTS);

        if( (GadgetID >= INPUT_XBOX__DIGITAL_BUTTONS_BEGIN) && (GadgetID <= INPUT_XBOX__DIGITAL_BUTTONS_END) )
        {
            s32 Mask = 0;

            switch( GadgetID )
            {
            case INPUT_XBOX_BTN_START:    Mask = XINPUT_GAMEPAD_START;          break;
            case INPUT_XBOX_BTN_BACK:     Mask = XINPUT_GAMEPAD_BACK;           break;
            case INPUT_XBOX_BTN_LEFT:     Mask = XINPUT_GAMEPAD_DPAD_LEFT;      break;
            case INPUT_XBOX_BTN_RIGHT:    Mask = XINPUT_GAMEPAD_DPAD_RIGHT;     break;
            case INPUT_XBOX_BTN_UP:       Mask = XINPUT_GAMEPAD_DPAD_UP;        break;
            case INPUT_XBOX_BTN_DOWN:     Mask = XINPUT_GAMEPAD_DPAD_DOWN;      break;
            case INPUT_XBOX_BTN_L_STICK:  Mask = XINPUT_GAMEPAD_LEFT_THUMB;     break;
            case INPUT_XBOX_BTN_R_STICK:  Mask = XINPUT_GAMEPAD_RIGHT_THUMB;    break;
            default: ASSERTS( 0, "Bad GadgetID" );
            }

            State = (s_PadStates[ControllerID][s_PadStatesIndex[ControllerID]].Gamepad.wButtons & Mask) != 0;
        }

        else if( (GadgetID >= INPUT_XBOX__ANALOG_BUTTONS_BEGIN) && (GadgetID <= INPUT_XBOX__ANALOG_BUTTONS_END) )
        {
            s32 Index = 0;

            switch( GadgetID )
            {
            case INPUT_XBOX_BTN_WHITE:  Index = XINPUT_GAMEPAD_WHITE;            break;
            case INPUT_XBOX_BTN_BLACK:  Index = XINPUT_GAMEPAD_BLACK;            break;
            case INPUT_XBOX_BTN_A:      Index = XINPUT_GAMEPAD_A;                break;
            case INPUT_XBOX_BTN_B:      Index = XINPUT_GAMEPAD_B;                break;
            case INPUT_XBOX_BTN_X:      Index = XINPUT_GAMEPAD_X;                break;
            case INPUT_XBOX_BTN_Y:      Index = XINPUT_GAMEPAD_Y;                break;
            case INPUT_XBOX_L_TRIGGER:  Index = XINPUT_GAMEPAD_LEFT_TRIGGER;     break;
            case INPUT_XBOX_R_TRIGGER:  Index = XINPUT_GAMEPAD_RIGHT_TRIGGER;    break;
            default: ASSERTS( 0, "Bad GadgetID" );
            }

            State = s_PadStates[ControllerID][s_PadStatesIndex[ControllerID]].Gamepad.bAnalogButtons[Index] > XINPUT_GAMEPAD_MAX_CROSSTALK;
        }
        else
        {
            ASSERTS( GadgetID==INPUT_UNDEFINED, "Bad GadgetID" );
        }
    }
    return State;
}

//==============================================================================

xbool input_IsPressed( input_gadget GadgetID, s32 ControllerID )
{
    if( GadgetID == INPUT_MSG_EXIT )
        return FALSE;
    if( ControllerID==-1 )
    {
        s32 i=0;
        for( ;i<4;i++ )
        {
            if( !IsPressedEx( GadgetID,i ))
                continue;
            break;
        }
        return( i<4 );
    }
    return IsPressedEx( GadgetID,ControllerID );
}

//==============================================================================

static xbool WasPressedEx( input_gadget GadgetID, s32 ControllerID )
{
    xbool State = FALSE;
    {
        ASSERT(ControllerID != -1);
        ASSERT(ControllerID < NUM_PORTS);

        if( (GadgetID >= INPUT_XBOX_BTN_START) && (GadgetID <= INPUT_XBOX_BTN_R_STICK) )
        {
            s32 Mask = 0;

            switch( GadgetID )
            {
            case INPUT_XBOX_BTN_START:    Mask = XINPUT_GAMEPAD_START;          break;
            case INPUT_XBOX_BTN_BACK:     Mask = XINPUT_GAMEPAD_BACK;           break;
            case INPUT_XBOX_BTN_LEFT:     Mask = XINPUT_GAMEPAD_DPAD_LEFT;      break;
            case INPUT_XBOX_BTN_RIGHT:    Mask = XINPUT_GAMEPAD_DPAD_RIGHT;     break;
            case INPUT_XBOX_BTN_UP:       Mask = XINPUT_GAMEPAD_DPAD_UP;        break;
            case INPUT_XBOX_BTN_DOWN:     Mask = XINPUT_GAMEPAD_DPAD_DOWN;      break;
            case INPUT_XBOX_BTN_L_STICK:  Mask = XINPUT_GAMEPAD_LEFT_THUMB;     break;
            case INPUT_XBOX_BTN_R_STICK:  Mask = XINPUT_GAMEPAD_RIGHT_THUMB;    break;
            default: ASSERTS( 0, "Bad GadgetID" );
            }

            State = ((s_PadStates[ControllerID][  s_PadStatesIndex[ControllerID]].Gamepad.wButtons & Mask) != 0) &&
                    ((s_PadStates[ControllerID][1-s_PadStatesIndex[ControllerID]].Gamepad.wButtons & Mask) == 0);
        }

        else if( (GadgetID >= INPUT_XBOX_BTN_WHITE) && (GadgetID <= INPUT_XBOX_R_TRIGGER) )
        {
            s32 Index = 0;

            switch( GadgetID )
            {
            case INPUT_XBOX_BTN_WHITE:  Index = XINPUT_GAMEPAD_WHITE;            break;
            case INPUT_XBOX_BTN_BLACK:  Index = XINPUT_GAMEPAD_BLACK;            break;
            case INPUT_XBOX_BTN_A:      Index = XINPUT_GAMEPAD_A;                break;
            case INPUT_XBOX_BTN_B:      Index = XINPUT_GAMEPAD_B;                break;
            case INPUT_XBOX_BTN_X:      Index = XINPUT_GAMEPAD_X;                break;
            case INPUT_XBOX_BTN_Y:      Index = XINPUT_GAMEPAD_Y;                break;
            case INPUT_XBOX_L_TRIGGER:  Index = XINPUT_GAMEPAD_LEFT_TRIGGER;     break;
            case INPUT_XBOX_R_TRIGGER:  Index = XINPUT_GAMEPAD_RIGHT_TRIGGER;    break;
            default: ASSERTS( 0, "Bad GadgetID" );
            }

            State = (s_PadStates[ControllerID][  s_PadStatesIndex[ControllerID]].Gamepad.bAnalogButtons[Index]  > XINPUT_GAMEPAD_MAX_CROSSTALK) &&
                    (s_PadStates[ControllerID][1-s_PadStatesIndex[ControllerID]].Gamepad.bAnalogButtons[Index] <= XINPUT_GAMEPAD_MAX_CROSSTALK);
        }

        else
        {
            ASSERTS( 0, "Bad GadgetID" );
        }
    }
    return State;
}

//==============================================================================

xbool input_WasPressed( input_gadget GadgetID, s32 ControllerID )
{
    if( GadgetID == INPUT_MSG_EXIT )
        return FALSE;
    if( ControllerID==-1 )
    {
        s32 i=0;
        for( ;i<4;i++ )
        {
            if( !WasPressedEx( GadgetID,i ))
                continue;
            break;
        }
        return( i<4 );
    }
    return WasPressedEx( GadgetID,ControllerID );
}

//==============================================================================

f32 input_GetValue( input_gadget GadgetID, s32 ControllerID )
{
    f32 Value = 0.0f;

    x_BeginAtomic( );
    {
        if( ControllerID < 0 )
            ControllerID = g_uiLastSelectController;
        ASSERT(ControllerID >= -1);
        ASSERT(ControllerID < NUM_PORTS);

        if( !s_hPads[ControllerID] )
        {
            x_EndAtomic();
            return 0.0f;
        }

        if( (GadgetID >= INPUT_XBOX_BTN_START) && (GadgetID <= INPUT_XBOX_BTN_R_STICK) )
        {
            s32 Mask = 0;

            switch( GadgetID )
            {
            case INPUT_XBOX_BTN_START:    Mask = XINPUT_GAMEPAD_START;          break;
            case INPUT_XBOX_BTN_BACK:     Mask = XINPUT_GAMEPAD_BACK;           break;
            case INPUT_XBOX_BTN_LEFT:     Mask = XINPUT_GAMEPAD_DPAD_LEFT;      break;
            case INPUT_XBOX_BTN_RIGHT:    Mask = XINPUT_GAMEPAD_DPAD_RIGHT;     break;
            case INPUT_XBOX_BTN_UP:       Mask = XINPUT_GAMEPAD_DPAD_UP;        break;
            case INPUT_XBOX_BTN_DOWN:     Mask = XINPUT_GAMEPAD_DPAD_DOWN;      break;
            case INPUT_XBOX_BTN_L_STICK:  Mask = XINPUT_GAMEPAD_LEFT_THUMB;     break;
            case INPUT_XBOX_BTN_R_STICK:  Mask = XINPUT_GAMEPAD_RIGHT_THUMB;    break;
            default: ASSERTS( 0, "Bad GadgetID" );
            }

            if( (s_PadStates[ControllerID][  s_PadStatesIndex[ControllerID]].Gamepad.wButtons & Mask) != 0 )
                Value = 1.0f;
        }

        else if( (GadgetID >= INPUT_XBOX_BTN_WHITE) && (GadgetID <= INPUT_XBOX_R_TRIGGER) )
        {
            s32 Index = 0;

            switch( GadgetID )
            {
            case INPUT_XBOX_BTN_WHITE:  Index = XINPUT_GAMEPAD_WHITE;            break;
            case INPUT_XBOX_BTN_BLACK:  Index = XINPUT_GAMEPAD_BLACK;            break;
            case INPUT_XBOX_BTN_A:      Index = XINPUT_GAMEPAD_A;                break;
            case INPUT_XBOX_BTN_B:      Index = XINPUT_GAMEPAD_B;                break;
            case INPUT_XBOX_BTN_X:      Index = XINPUT_GAMEPAD_X;                break;
            case INPUT_XBOX_BTN_Y:      Index = XINPUT_GAMEPAD_Y;                break;
            case INPUT_XBOX_L_TRIGGER:  Index = XINPUT_GAMEPAD_LEFT_TRIGGER;     break;
            case INPUT_XBOX_R_TRIGGER:  Index = XINPUT_GAMEPAD_RIGHT_TRIGGER;    break;
            default: ASSERTS( 0, "Bad GadgetID" );
            }

            Value = s_PadStates[ControllerID][  s_PadStatesIndex[ControllerID]].Gamepad.bAnalogButtons[Index] / 255.0f;
        }

        else if( (GadgetID >= INPUT_XBOX_STICK_LEFT_X) && (GadgetID <= INPUT_XBOX_STICK_RIGHT_Y) )
        {
            switch( GadgetID )
            {
                case INPUT_XBOX_STICK_LEFT_X:
                    Value = s_PadStates[ControllerID][s_PadStatesIndex[ControllerID]].Gamepad.sThumbLX;
                    break;

                case INPUT_XBOX_STICK_LEFT_Y:
                    Value = s_PadStates[ControllerID][s_PadStatesIndex[ControllerID]].Gamepad.sThumbLY;
                    break;

                case INPUT_XBOX_STICK_RIGHT_X:
                    Value = s_PadStates[ControllerID][s_PadStatesIndex[ControllerID]].Gamepad.sThumbRX;
                    break;

                case INPUT_XBOX_STICK_RIGHT_Y:
                    Value = s_PadStates[ControllerID][s_PadStatesIndex[ControllerID]].Gamepad.sThumbRY;
                    break;

                default:
                    ASSERTS( 0, "Bad GadgetID" );
                    break;
            }

            Value /= 32768.0f;
            if( x_abs(Value) < 0.37f )
                Value = 0.0f;
        }
        else
        {
            ASSERTS( 0, "Bad GadgetID" );
        }
    }
    x_EndAtomic( );

    return Value;
}

//==============================================================================

xbool input_IsPresent( input_gadget GadgetID, s32 ControllerID /* = -1  */)
{
    ASSERT(ControllerID >= -1);
    ASSERT(ControllerID < NUM_PORTS);

    switch( GadgetID )
    {
    case INPUT_XBOX_QRY_PAD_PRESENT:
        if( ControllerID == -1 )
        {
            s32 i=0;
            for( ; i < NUM_PORTS; i++ )
            {
                if (s_hPads[i] == NULL)
                    continue;
                break;
            }
            return( i < NUM_PORTS );
        }

        return (s_hPads[ ControllerID ] != NULL);
        break;

    case INPUT_XBOX_QRY_ANALOG_MODE:           
        return (TRUE);  // is there such a thing as NO ANALOG on XBox? 
        break;

    case INPUT_XBOX_QRY_KBD_PRESENT:
    case INPUT_XBOX_QRY_MOUSE_PRESENT:
    case INPUT_XBOX_QRY_MEM_CARD_PRESENT:
        ASSERTS( FALSE, "Not implemented" );
        break;

    default:
        ASSERTS( FALSE, "Invalid input Query" );
        break;
    }

    return FALSE;
}

//==============================================================================
//==============================================================================
//==============================================================================

void input_Init( void )
{
    ASSERT( !s_pRumbleMgr );

    DeviceInit();

    new RumbleMgr;
}

//==============================================================================

void input_Feedback( f32 Duration, f32 Intensity, s32 ControllerID )
{
    ASSERT(ControllerID >= -1);
    ASSERT(ControllerID < NUM_PORTS);

    if( s_pRumbleMgr )
    {
        s_pRumbleMgr->SetType     ( ControllerID, RT_INTENSITY );
        s_pRumbleMgr->AddIntensity( ControllerID, Intensity    );
        s_pRumbleMgr->SetDuration ( ControllerID, Duration     );
    }
}

//==============================================================================

void input_Feedback( s32 Count, feedback_envelope* pEnvelope, s32 ControllerID )
{
    ASSERT(ControllerID >= -1);
    ASSERT(ControllerID < NUM_PORTS);

    if( s_pRumbleMgr )
    {
        s_pRumbleMgr->SetType    ( ControllerID, RT_ENVELOPE );
        s_pRumbleMgr->SetEnvelope( ControllerID, *pEnvelope  );
    }
}

//==============================================================================

void input_EnableFeedback( xbool state, s32 ControllerID )
{
    ASSERT(ControllerID >= -1);
    ASSERT(ControllerID < NUM_PORTS);

    if( s_pRumbleMgr )
    {
        s_FeedbackSwitch[ControllerID] = state;

        if( !state )
        {
            ZeroMemory( s_pRumbleMgr->m_Env + ControllerID, sizeof( feedback_envelope ));
            s_pRumbleMgr->m_Type[ControllerID] = RT_NO_RUMBLE;
        }
    }
}

//==============================================================================

void input_SuppressFeedback( xbool Suppress )
{
    s_SuppressFeedback = Suppress;
    if( Suppress )
        xbox_DisableRumble();
    else
        xbox_EnableRumble();
}

//==============================================================================

void input_Kill( void )
{
    ASSERT( s_pRumbleMgr );
    delete s_pRumbleMgr;
    s_pRumbleMgr = 0;
}

//==============================================================================

void input_ClearFeedback( void )
{
    if (NULL == s_pRumbleMgr)
        return;
    
    for( s32 i=0; i<PAD_MAX_CONTROLLERS; i++ )
    {
        ZeroMemory( &(s_pRumbleMgr->Feedback[i]), sizeof( XINPUT_FEEDBACK ));
        ZeroMemory( s_pRumbleMgr->m_Env + i, sizeof( feedback_envelope ));
        s_pRumbleMgr->m_Type[i] = RT_NO_RUMBLE;
    }
}

//=========================================================================
