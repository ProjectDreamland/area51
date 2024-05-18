
///////////////////////////////////////////////////////////////////////////
// INCLUDES 
///////////////////////////////////////////////////////////////////////////

#include "..\e_Engine.hpp"

#define DIRECTINPUT_VERSION  0x0800
#include "dinput.h"

///////////////////////////////////////////////////////////////////////////
// DEFINES
///////////////////////////////////////////////////////////////////////////

#define MAX_DEVICES      8          // Maximun number of device per type
#define REFRESH_RATE    15          // times a second. 30
#define MAX_STATES      64           // ( REFRESH_RATE - MAX_STATES ) is how 
                                    // long we can go without loosing info. 16
#define MAX_EVENTS      64           // This is how long DirectX can go 
                                    // before loosing input. 16

enum device_flags
{
    DEVICE_FLG_PS2_PAD = (1<<0),
    DEVICE_FLG_SOYO_PS2_PAD = (1<<1),
    DEVICE_FLG_DUALSHOCK_CONVERTER_PS2_PAD = (1<<2),
    DEVICE_FLG_PSX_USB_PAD = (1<<3),
};

enum digital_type
{
    DIGITAL_ON         = (1<<0),
    DIGITAL_DEBAUNCE   = (1<<1),
};

enum
{
    DIGITAL_COUNT_PCPAD  = INPUT_PC__ANALOG     - INPUT_PC__DIGITAL,
    DIGITAL_COUNT_PS2PAD = 32,
    DIGITAL_COUNT_MOUSE  = INPUT_MOUSE__ANALOG  - INPUT_MOUSE__DIGITAL,
    DIGITAL_COUNT_KBD    = INPUT_KBD__END       - INPUT_KBD__DIGITAL,

    ANALOG_COUNT_PCPAD   = INPUT_PC__END        - INPUT_PC__ANALOG,
    ANALOG_COUNT_PS2PAD  = 32,
    ANALOG_COUNT_MOUSE   = INPUT_MOUSE__END     - INPUT_MOUSE__ANALOG,
};

///////////////////////////////////////////////////////////////////////////
// TYPES
///////////////////////////////////////////////////////////////////////////

struct device
{
    IDirectInputDevice8*    pDevice;
    u32                     Flags;
};

struct input_mouse
{
    byte    Digital[ DIGITAL_COUNT_MOUSE ];
    f32     Anolog [ ANALOG_COUNT_MOUSE  ];
};

struct input_keyboard
{
    byte    Digital[ DIGITAL_COUNT_KBD ];
};

struct input_pc_pad
{
    byte    Digital[ DIGITAL_COUNT_PCPAD ];
    f32     Anolog [ ANALOG_COUNT_PCPAD  ];
};

struct input_ps2_pad
{
    byte    Digital[ DIGITAL_COUNT_PS2PAD ];
    f32     Anolog [ ANALOG_COUNT_PS2PAD  ];
};

struct state
{
    s64                 TimeStamp;
    input_keyboard      Keyboard[ MAX_DEVICES ];
    input_mouse         Mouse   [ MAX_DEVICES ];
    input_pc_pad        PCPad   [ MAX_DEVICES ];
    input_ps2_pad       PS2Pad  [ MAX_DEVICES ];
};

///////////////////////////////////////////////////////////////////////////
// VARIABLES
///////////////////////////////////////////////////////////////////////////

static struct
{
    HWND                    Window;
    IDirectInput8*          pDInput;
    MSG                     Msg;
    xbool                   bProcessEvents;

    s32                     nMouses;
    s32                     nKeyboards;
    s32                     nJoysticks;

    device                  Mouse   [ MAX_DEVICES ];
    device                  Keyboard[ MAX_DEVICES ];
    device                  Joystick[ MAX_DEVICES ];

    s32                     nPCPads;
    s32                     PCPadDevice [ MAX_DEVICES ];

    s32                     nPS2Pads;
    s32                     PS2PadDevice[ MAX_DEVICES ];

    s32                     KeybdDevice [ MAX_DEVICES ];
    s32                     MouseDevice [ MAX_DEVICES ];

    xbool                   bExclusive;
    xbool                   bForeground;
    xbool                   bImmediate;
    xbool                   bDisableWindowsKey;

    state                   State[ MAX_STATES ];
    s32                     nStates;
    s32                     iState;
    s32                     iStateNext;

    s64                     CurrentTimeFrame;
    s64                     LastTimeFrame;
    s64                     TicksPerRefresh;

    xbool                   ExitApp;

} s_Input = {0};

///////////////////////////////////////////////////////////////////////////
// FUNCTIONS
///////////////////////////////////////////////////////////////////////////

static dxerr CreateMouse   ( device& Device, const DIDEVICEINSTANCE* pInstance, s32 SampleBufferSize );
static dxerr CreateKeyboard( device& Device, const DIDEVICEINSTANCE* pInstance, s32 SampleBufferSize );
static dxerr CreateJoystick( device& Device, const DIDEVICEINSTANCE* pInstance, s32 SampleBufferSize );
        void d3deng_KillInput( void );

static xbool s_DoNotProcessWindowsMessages = FALSE;

//=========================================================================
static 
void ClearDebounce( state& State )
{
    s32 i;

    //
    // Clear all the debounce for the PCPads
    // 
    for( i=0; i<s_Input.nPCPads; i++ )
    {
        for( s32 d=0; d<DIGITAL_COUNT_PCPAD; d++ )
        {
            State.PCPad[i].Digital[d] &= ~DIGITAL_DEBAUNCE;
        }
    }

    //
    // Clear all the debounce for the PS2Pads
    // 
    for( i=0; i<s_Input.nPS2Pads; i++ )
    {
        for( s32 d=0; d<DIGITAL_COUNT_PS2PAD; d++ )
        {
            State.PS2Pad[i].Digital[d] &= ~DIGITAL_DEBAUNCE;
        }

        // Since allot of the keys in this joystick as tought to
        // be anolog we builded a debounce on it call 0.001f
        // so we must clear that fractional bit in here
        // make sure to skip the atual joysticks in the pad.
        for( s32 a=0; a<ANALOG_COUNT_PS2PAD; a++ )// - 5; a++ )
        {
            if( a >= (INPUT_PS2_STICK_LEFT_X-INPUT_PS2_BTN_L2) && a <= (INPUT_PS2_STICK_RIGHT_Y-INPUT_PS2_BTN_L2) )
            {
                // Nothing to debounce
            }
            else
            if( (State.PS2Pad[i].Anolog[a] - 1) >= 0 )
                State.PS2Pad[i].Anolog[a] = 1;
            else
                State.PS2Pad[i].Anolog[a] = 0;
        }
    }

    //
    // Clear all the debounce for the Mouse
    // 
    for( i=0; i<s_Input.nMouses; i++ )
    {
        for( s32 d=0; d<DIGITAL_COUNT_MOUSE; d++ )
        {
            State.Mouse[i].Digital[d] &= ~DIGITAL_DEBAUNCE;
        }

        // All the mouse anolog are relative so they need to be 
        // clean up everytime.
        for( s32 a=0; a<ANALOG_COUNT_MOUSE; a++ )
        {
            State.Mouse[i].Anolog[a] = 0;
        }
    }


    //
    // Clear all the debounce for the Keyboard
    // 
    for( i=0; i<s_Input.nKeyboards; i++ )
    {
        for( s32 d=0; d<DIGITAL_COUNT_KBD; d++ )
        {
            State.Keyboard[i].Digital[d] &= ~DIGITAL_DEBAUNCE;
        }
    }
}

//=========================================================================
static 
void PageFlipQueue( void )
{
    //
    //  Clear all the time stamps
    //
    for( s32 i=0; i<MAX_STATES; i++ )
    {
        s_Input.State[ i ].TimeStamp = 0;
    }

    //
    // Prepare the first event in the queue
    //
    if( s_Input.bImmediate )
    {
        s_Input.State[ 0 ].TimeStamp = s_Input.CurrentTimeFrame;    
    }
    else
    {
        //
        // Copy the previous old state
        //
        ASSERT( s_Input.nStates > 0 );
        s_Input.State[ 0 ] = s_Input.State[ (s_Input.nStates-1) ];

        //
        // Update the time to our old one to make sure that we don't loose precision
        //
        s_Input.State[ 0 ].TimeStamp = s_Input.LastTimeFrame;
    }

    //
    // Clear the events
    //
    s_Input.iState      = 0;
    s_Input.nStates     = 1;        // We always have at least one state
    s_Input.iStateNext  = 0;        


    //
    // Make sure that all the debounces are clear
    //
    ClearDebounce( s_Input.State[0] );
}

//=========================================================================
static 
state& GetState( s64 TimeStamp )
{
    s32 i = 0;

    //
    // Try to find the right state
    //
//    DebugMessage( "%f, %f %f\n", (f64)ABS( TimeStamp - s_Input.LastTimeFrame ), (f64)TimeStamp, (f64)s_Input.LastTimeFrame );

    while( TimeStamp > ( s_Input.State[ i ].TimeStamp + s_Input.TicksPerRefresh )  ) 
    {
        //
        // Make sure that the next time interval is initialize
        //
        i++;
        ASSERT( i < MAX_STATES );
        if( s_Input.State[ i ].TimeStamp == 0)
        {            
            // Copy the previous time frame
            s_Input.State[ i ]            = s_Input.State[ i-1 ];
            s_Input.State[ i ].TimeStamp += s_Input.TicksPerRefresh;

            // Clear the debounce for the new state
            ClearDebounce( s_Input.State[ i ] );

            // Update the queue count
            s_Input.nStates++;
            ASSERT( s_Input.nStates <= MAX_STATES );
        }
    }

    ASSERT( i < MAX_STATES );
    return s_Input.State[ i ];
}

//=========================================================================
static 
BOOL CALLBACK EnumKeyboardCallback( const DIDEVICEINSTANCE* pdidInstance,
                                    VOID* pContext )
{
    dxerr Error;

    // Is the main keyboard? If so then do some quick nothing.
//    if( GUID_SysKeyboard == pdidInstance ) {}

    // If it failed, then we can't use this Keyboard. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    Error = CreateKeyboard( s_Input.Keyboard[ s_Input.nKeyboards ], pdidInstance, MAX_EVENTS );
    if( !FAILED(Error) ) 
    {
        s_Input.nKeyboards++;
    }

    // If it failed, then we can't use this keyboard. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    return DIENUM_CONTINUE;
}

//=========================================================================
static 
BOOL CALLBACK EnumMouseCallback( const DIDEVICEINSTANCE* pdidInstance,
                                 VOID* pContext )
{
    dxerr Error;

    // Is the main mouse If so then do some quick nothing.
    //if( GUID_SysMouse == pdidInstance ) {}

    Error = CreateMouse( s_Input.Mouse[ s_Input.nMouses ], pdidInstance, MAX_EVENTS );

    // If it failed, then we can't use this Mouse. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    if( !FAILED(Error) ) 
    {
        s_Input.nMouses++;
    }

    // If it failed, then we can't use this keyboard. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    return DIENUM_CONTINUE;
}

//=========================================================================
static 
BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance,
                                     VOID* pContext )
{
    dxerr Error;

    Error = CreateJoystick( s_Input.Joystick[ s_Input.nJoysticks ], pdidInstance, MAX_EVENTS );

    // If it failed, then we can't use this joystick. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    if( !FAILED(Error) ) 
    {
        s_Input.nJoysticks++;
    }

    // If it failed, then we can't use this keyboard. (Maybe the user unplugged
    // it while we were in the middle of enumerating it.)
    return DIENUM_CONTINUE;
}



//=========================================================================
static 
dxerr CreateMouse( device& Device, const DIDEVICEINSTANCE* pInstance, s32 SampleBufferSize )
{
    dxerr   Error;
    DWORD   dwCoopFlags;

    // Detrimine where the buffer would like to be allocated 
    if( s_Input.bExclusive )
        dwCoopFlags = DISCL_EXCLUSIVE;
    else
        dwCoopFlags = DISCL_NONEXCLUSIVE;

    if( s_Input.bForeground )
        dwCoopFlags |= DISCL_FOREGROUND;
    else
        dwCoopFlags |= DISCL_BACKGROUND;
    
    // Obtain an interface to the system mouse device.
    Error = s_Input.pDInput->CreateDevice( pInstance->guidInstance, &Device.pDevice, NULL );
    if( FAILED( Error ) )
        return Error;
    
    // Set the data format to "mouse format" - a predefined data format 
    //
    // A data format specifies which controls on a device we
    // are interested in, and how they should be reported.
    //
    // This tells DirectInput that we will be passing a
    // DIMOUSESTATE2 structure to IDirectInputDevice::GetDeviceState.
    Error = Device.pDevice->SetDataFormat( &c_dfDIMouse2 );
    if( FAILED( Error ) )
        return Error;
    
    // Set the cooperativity level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications_Input.
    Error = Device.pDevice->SetCooperativeLevel( s_Input.Window, dwCoopFlags );
    if( Error == DIERR_UNSUPPORTED && !s_Input.bForeground && s_Input.bExclusive )
    {
        d3deng_KillInput();
        MessageBox( s_Input.Window, "SetCooperativeLevel() returned DIERR_UNSUPPORTED.\n"
                              "For security reasons, background exclusive Mouse\n"
                              "access is not allowed.", 
                              "Mouse", MB_OK );
        return Error;
    }

    if( FAILED(Error) )
        return Error;

    if( !s_Input.bImmediate )
    {
        // IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
        //
        // DirectInput uses unbuffered I/O (buffer size = 0) by default.
        // If you want to read buffered data, you need to set a nonzero
        // buffer size.
        //
        // Set the buffer size to SAMPLE_BUFFER_SIZE (defined above) elements_Input.
        //
        // The buffer size is a DWORD property associated with the device.
        DIPROPDWORD dipdw;
        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj        = 0;
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.dwData            = SampleBufferSize; // Arbitary buffer size

        Error = Device.pDevice->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );
        if( FAILED( Error ) )
            return Error;
    }

    // Acquire the newly created device
    Device.pDevice->Acquire();

    // Set the device for this specific mouse
    s_Input.MouseDevice[ s_Input.nMouses ] = s_Input.nMouses;

    return Error;
}

//=========================================================================
static 
dxerr CreateKeyboard( device& Device, const DIDEVICEINSTANCE* pInstance, s32 SampleBufferSize )
{
    dxerr   Error;
    DWORD   dwCoopFlags;

    if( s_Input.bExclusive )
        dwCoopFlags = DISCL_EXCLUSIVE;
    else
        dwCoopFlags = DISCL_NONEXCLUSIVE;

    if( s_Input.bForeground )
        dwCoopFlags |= DISCL_FOREGROUND;
    else
        dwCoopFlags |= DISCL_BACKGROUND;

    // Disabling the windows key is only allowed only if we are in foreground nonexclusive
    if( s_Input.bDisableWindowsKey && !s_Input.bExclusive && s_Input.bForeground )
        dwCoopFlags |= DISCL_NOWINKEY;

    // Obtain an interface to the keyboard device.
    Error = s_Input.pDInput->CreateDevice( pInstance->guidInstance, &Device.pDevice, NULL );
    if( FAILED( Error ) )
        return Error;
    
    // Set the data format to "keyboard format" - a predefined data format 
    //
    // A data format specifies which controls on a device we
    // are interested in, and how they should be reported.
    //
    // This tells DirectInput that we will be passing an array
    // of 256 bytes to IDirectInputDevice::GetDeviceState.
    Error = Device.pDevice->SetDataFormat( &c_dfDIKeyboard );
    if( FAILED( Error ) )
        return Error;
    
    // Set the cooperativity level to let DirectInput know how
    // this device should interact with the system and with other
    // DirectInput applications_Input.
    Error = Device.pDevice->SetCooperativeLevel( s_Input.Window, dwCoopFlags );
    if( Error == DIERR_UNSUPPORTED && !s_Input.bForeground && s_Input.bExclusive )
    {
        d3deng_KillInput();
        MessageBox( s_Input.Window,"SetCooperativeLevel() returned DIERR_UNSUPPORTED.\n"
                                   "For security reasons, background exclusive keyboard\n"
                                   "access is not allowed.", "Keyboard", MB_OK );
        return Error;
    }

    if( FAILED(Error) )
        return Error;

    if( !s_Input.bImmediate )
    {
        // IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
        //
        // DirectInput uses unbuffered I/O (buffer size = 0) by default.
        // If you want to read buffered data, you need to set a nonzero
        // buffer size.
        //
        // Set the buffer size to DINPUT_BUFFERSIZE (defined above) elements_Input.
        //
        // The buffer size is a DWORD property associated with the device.
        DIPROPDWORD dipdw;

        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj        = 0;
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.dwData            = SampleBufferSize; // Arbitary buffer size

        Error = Device.pDevice->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );
        if( FAILED( Error ) )
            return Error;
    }

    // Acquire the newly created device
    Device.pDevice->Acquire();

    // Set the device for this specific keyboard
    s_Input.KeybdDevice[ s_Input.nKeyboards ] = s_Input.nKeyboards;

    return Error;
}

//=========================================================================
static 
BOOL CALLBACK EnumAxesCallback( const DIDEVICEOBJECTINSTANCE* pdidoi,
                                VOID* pContext )
{
    IDirectInputDevice8*    pDevice = (IDirectInputDevice8*)pContext;

    //
    // Set the range
    //
    DIPROPRANGE diprg; 
    diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    diprg.diph.dwHow        = DIPH_BYID; 
    diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
    diprg.lMin              = -1000; 
    diprg.lMax              = +1000; 
    
	// Set the range for the axis
	if( FAILED( pDevice->SetProperty( DIPROP_RANGE, &diprg.diph ) ) )
		return DIENUM_STOP;

/*
    //
    // Set the dead zone
    //
    DIPROPRANGE diprg; 
    diprg.diph.dwSize       = sizeof(DIPROPRANGE); 
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
    diprg.diph.dwHow        = DIPH_BYID; 
    diprg.diph.dwObj        = pdidoi->dwType; // Specify the enumerated axis
    diprg.lMin              = -1000; 
    diprg.lMax              = +1000; 
    
	// Set the range for the axis
	if( FAILED( pDevice->SetProperty( DIPROP_DEADZONE, &diprg.diph ) ) )
		return DIENUM_STOP;
*/

    return DIENUM_CONTINUE;
}    

//=========================================================================
static 
dxerr CreateJoystick( device& Device, const DIDEVICEINSTANCE* pInstance, s32 SampleBufferSize )
{
    dxerr Error;
    DWORD   dwCoopFlags;

    if( s_Input.bExclusive )
        dwCoopFlags = DISCL_EXCLUSIVE;
    else
        dwCoopFlags = DISCL_NONEXCLUSIVE;

    if( s_Input.bForeground )
        dwCoopFlags |= DISCL_FOREGROUND;
    else
        dwCoopFlags |= DISCL_BACKGROUND;

    // Obtain an interface to the joystick device.
    Error = s_Input.pDInput->CreateDevice( pInstance->guidInstance, &Device.pDevice, NULL );
    if( FAILED( Error ) )
        return Error;

    // Set the data format to "simple joystick" - a predefined data format 
    //
    // A data format specifies which controls on a device we are interested in,
    // and how they should be reported. This tells DInput that we will be
    // passing a DIJOYSTATE2 structure to IDirectInputDevice::GetDeviceState().
    Error = Device.pDevice->SetDataFormat( &c_dfDIJoystick2 );
    if( FAILED( Error ) )
        return Error;

    // Set the cooperative level to let DInput know how this device should
    // interact with the system and with other DInput applications.
    Error = Device.pDevice->SetCooperativeLevel( s_Input.Window, dwCoopFlags );
    if( Error == DIERR_UNSUPPORTED && !s_Input.bForeground && s_Input.bExclusive )
    {
        d3deng_KillInput();
        MessageBox( s_Input.Window,"SetCooperativeLevel() returned DIERR_UNSUPPORTED.\n"
                                   "For security reasons, background exclusive joystick\n"
                                   "access is not allowed.", "Keyboard", MB_OK );
        return Error;
    }
    if( Error == E_INVALIDARG )
    {
        d3deng_KillInput();
        MessageBox( s_Input.Window,"SetCooperativeLevel() returned DIERR_INVALIDPARAM.\n"
                                   "Invalid parameter", "Joystick", MB_OK );
        return Error;
    }
    if( Error == E_HANDLE )
    {
        d3deng_KillInput();
        MessageBox( s_Input.Window,"SetCooperativeLevel() returned E_HANDLE.\n"
                                   "Invalid window handle", "Joystick", MB_OK );
        return Error;
    }
    
    if( FAILED(Error) )
        return Error;

    if( !s_Input.bImmediate )
    {
        // IMPORTANT STEP TO USE BUFFERED DEVICE DATA!
        //
        // DirectInput uses unbuffered I/O (buffer size = 0) by default.
        // If you want to read buffered data, you need to set a nonzero
        // buffer size.
        //
        // Set the buffer size to DINPUT_BUFFERSIZE (defined above) elements_Input.
        //
        // The buffer size is a DWORD property associated with the device.
        DIPROPDWORD dipdw;

        dipdw.diph.dwSize       = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj        = 0;
        dipdw.diph.dwHow        = DIPH_DEVICE;
        dipdw.dwData            = SampleBufferSize; // Arbitary buffer size

        Error = Device.pDevice->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph );
        if( FAILED( Error ) )
            return Error;
    }

    // Determine how many axis the joystick has (so we don't error out setting
    // properties for unavailable axis)
    DIDEVCAPS               DeviceCaps;
    DeviceCaps.dwSize = sizeof(DIDEVCAPS);
    Error = Device.pDevice->GetCapabilities( &DeviceCaps );
    if( FAILED( Error ) )
        return Error;

    // Enumerate the axes of the joyctick and set the range of each axis. Note:
    // we could just use the defaults, but we're just trying to show an example
    // of enumerating device objects (axes, buttons, etc.).
    Error = Device.pDevice->EnumObjects( EnumAxesCallback, (VOID*)Device.pDevice, DIDFT_AXIS );
    if( FAILED( Error ) )
        return Error;

    //
    // Set the device for the type of joystick
    //

    if( x_strcmp( "4 axis 16 button joystick", pInstance->tszProductName ) == 0 )
    {
        Device.Flags |= DEVICE_FLG_PS2_PAD;

        s_Input.PS2PadDevice[ s_Input.nPS2Pads ] = s_Input.nJoysticks;
        s_Input.nPS2Pads++;
    }
    else if( (x_strcmp( "PSX for USB Converter (4-axis, 12-button, POV,effects)", pInstance->tszProductName ) == 0) ||
             (x_strcmp( "PSX/USB Pad Adaptor (4-axis, 12-button, POV,effects)", pInstance->tszProductName ) == 0) )
    {
        Device.Flags |= DEVICE_FLG_PS2_PAD;
		Device.Flags |= DEVICE_FLG_SOYO_PS2_PAD;

        s_Input.PS2PadDevice[ s_Input.nPS2Pads ] = s_Input.nJoysticks;
        s_Input.nPS2Pads++;
    }
    else if( (x_strcmp( "PS Vibration Feedback Converter ", pInstance->tszProductName ) == 0) ||
             (x_strcmp( "Logitech RumblePad 2 USB", pInstance->tszProductName ) == 0) )
    {
        Device.Flags |= DEVICE_FLG_PS2_PAD;
		Device.Flags |= DEVICE_FLG_DUALSHOCK_CONVERTER_PS2_PAD;

        s_Input.PS2PadDevice[ s_Input.nPS2Pads ] = s_Input.nJoysticks;
        s_Input.nPS2Pads++;
    }
    else if(  ( x_strcmp( "PSX/USB Pad", pInstance->tszProductName ) == 0 ) ||
             (x_strcmp( "4 axis 12 button joystick with hat switch", pInstance->tszProductName ) == 0) )
    {
        Device.Flags |= DEVICE_FLG_PS2_PAD;
        Device.Flags |= DEVICE_FLG_PSX_USB_PAD;

        s_Input.PS2PadDevice[ s_Input.nPS2Pads ] = s_Input.nJoysticks;
        s_Input.nPS2Pads++;
    }
    else if( x_strcmp( "Logitech Dual Action USB", pInstance->tszProductName ) == 0 )
    {
        Device.Flags |= DEVICE_FLG_PS2_PAD;

        s_Input.PS2PadDevice[ s_Input.nPS2Pads ] = s_Input.nJoysticks;
        s_Input.nPS2Pads++;
    }
    else
    {

        s_Input.PCPadDevice[ s_Input.nPCPads ] = s_Input.nJoysticks;
        s_Input.nPCPads++;
    
	}  



    return Error;
}

//=========================================================================
static 
dxerr ReadKeyboadBufferedData( device& Device, s32 ID )
{
    DIDEVICEOBJECTDATA  didod[ MAX_EVENTS ];  // Receives buffered data 
    DWORD               dwElements;
    dxerr               Error;

    dwElements = MAX_EVENTS;
    Error = Device.pDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0 );
    if( Error != DI_OK ) 
    {
        // We got an error or we got DI_BUFFEROVERFLOW.
        //
        // Either way, it means that continuous contact with the
        // device has been lost, either due to an external
        // interruption, or because the buffer overflowed
        // and some events were lost.
        //
        // Consequently, if a button was pressed at the time
        // the buffer overflowed or the connection was broken,
        // the corresponding "up" message might have been lost.
        //
        // But since our simple sample doesn't actually have
        // any state associated with button up or down events,
        // there is no state to reset.  (In a real game, ignoring
        // the buffer overflow would result in the game thinking
        // a key was held down when in fact it isn't; it's just
        // that the "up" event got lost because the buffer
        // overflowed.)
        //
        // If we want to be cleverer, we could do a
        // GetDeviceState() and compare the current state
        // against the state we think the device is in,
        // and process all the states that are currently
        // different from our private state.
        Error = Device.pDevice->Acquire();
        while( Error == DIERR_INPUTLOST ) 
            Error = Device.pDevice->Acquire();

        // Update the dialog text 
        if( Error == DIERR_OTHERAPPHASPRIO || 
            Error == DIERR_NOTACQUIRED ) 
        {
            // We lost the device some how 
            //    SetDlgItemText( s_Input.Window, IDC_DATA, "Unacquired" );
        }

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return Error; 
    }

    if( FAILED(Error) )  
        return Error;

    //
    // Study each of the buffer elements and process them.
    //
    for( u32 i = 0; i < dwElements; i++ ) 
    {
        state&          State  = GetState( didod[ i ].dwTimeStamp );
        input_gadget    Gadget = (input_gadget)( didod[ i ].dwOfs );

        ASSERT( Gadget >= 0 );
        ASSERT( Gadget < DIGITAL_COUNT_KBD );

        if( (didod[ i ].dwData & 0x80) )
        {
            State.Keyboard[ ID ].Digital[ Gadget ] |= DIGITAL_ON | DIGITAL_DEBAUNCE;
        }
        else
        {
            State.Keyboard[ ID ].Digital[ Gadget ] &= (~DIGITAL_ON);
        }
    }

    return Error;
}

//=========================================================================
static 
dxerr ReadMouseBufferedData( device& Device, s32 ID )
{
    DIDEVICEOBJECTDATA didod[ MAX_EVENTS ];  // Receives buffered data 
    DWORD              dwElements;
    dxerr              Error;

    dwElements = MAX_EVENTS;
    Error = Device.pDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0 );
    if( Error != DI_OK ) 
    {
        // We got an error or we got DI_BUFFEROVERFLOW.
        //
        // Either way, it means that continuous contact with the
        // device has been lost, either due to an external
        // interruption, or because the buffer overflowed
        // and some events were lost.
        //
        // Consequently, if a button was pressed at the time
        // the buffer overflowed or the connection was broken,
        // the corresponding "up" message might have been lost.
        //
        // But since our simple sample doesn't actually have
        // any state associated with button up or down events,
        // there is no state to reset.  (In a real game, ignoring
        // the buffer overflow would result in the game thinking
        // a key was held down when in fact it isn't; it's just
        // that the "up" event got lost because the buffer
        // overflowed.)
        //
        // If we want to be cleverer, we could do a
        // GetDeviceState() and compare the current state
        // against the state we think the device is in,
        // and process all the states that are currently
        // different from our private state.
        Error = Device.pDevice->Acquire();
        while( Error == DIERR_INPUTLOST ) 
            Error = Device.pDevice->Acquire();

        // Update the dialog text 
        if( Error == DIERR_OTHERAPPHASPRIO || 
            Error == DIERR_NOTACQUIRED ) 
        {
        //    SetDlgItemText( hDlg, IDC_DATA, "Unacquired" );
        }

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return Error; 
    }

    if( FAILED(Error) )  
        return Error;

    //
    // Study each of the buffer elements and process them.
    //
    for( u32 i = 0; i < dwElements; i++ ) 
    {
        state& State  = GetState( didod[ i ].dwTimeStamp );

        if( didod[ i ].dwOfs >= DIMOFS_BUTTON0 && didod[ i ].dwOfs <= DIMOFS_BUTTON7 )
        {
            s32 Index = didod[ i ].dwOfs - DIMOFS_BUTTON0;
            ASSERT( Index >= 0 );
            ASSERT( Index < DIGITAL_COUNT_MOUSE );

            if( (didod[ i ].dwData & 0x80) )
            {
                State.Mouse[ID].Digital[ Index ] |= DIGITAL_ON | DIGITAL_DEBAUNCE;
            }
            else
            {
                State.Mouse[ID].Digital[ Index ] &= (~DIGITAL_ON);
            }
        }
        else if( didod[ i ].dwOfs >= DIMOFS_X && didod[ i ].dwOfs <= DIMOFS_Z )
        {
            s32 Index = (didod[ i ].dwOfs - DIMOFS_X)>>2;
            ASSERT( Index >= 0 );
            ASSERT( Index < ANALOG_COUNT_MOUSE );
            //DebugMessage( "%d\n", Index );
            State.Mouse[ID].Anolog[ Index ] = (f32)((s32)didod[ i ].dwData);
        }
    }

    return Error;
}

//=========================================================================
static 
dxerr ReadMouseImmediateData( device& Device, s32 ID )
{
    dxerr           Error;
    DIMOUSESTATE2   dims2;      // DirectInput mouse state structure

    // Get the input's device state, and put the state in dims
    ZeroMemory( &dims2, sizeof(dims2) );

    Error = Device.pDevice->GetDeviceState( sizeof(DIMOUSESTATE2), &dims2 );
    if( FAILED(Error) ) 
    {
        // DirectInput may be telling us that the input stream has been
        // interrupted.  We aren't tracking any state between polls, so
        // we don't have any special reset that needs to be done.
        // We just re-acquire and try again.
        
        // If input is lost then acquire and keep trying 
        Error = Device.pDevice->Acquire();
        while( Error == DIERR_INPUTLOST ) 
            Error = Device.pDevice->Acquire();

        // Update the dialog text 
        if( Error == DIERR_OTHERAPPHASPRIO || 
            Error == DIERR_NOTACQUIRED ) 
        {
            // Lost device
        //    SetDlgItemText( hDlg, IDC_DATA, "Unacquired" );
        }

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return Error; 
    }
    
    //
    // The dims structure now has the state of the mouse.
    //        
    for( s32 i=0; i<8; i++ )
    {
        s_Input.State[0].Mouse[ID].Digital[i] = (dims2.rgbButtons[i]& 0x80) != 0;
    }

    s_Input.State[0].Mouse[ID].Anolog[0] = (f32)dims2.lX;
    s_Input.State[0].Mouse[ID].Anolog[1] = (f32)dims2.lY; 
    s_Input.State[0].Mouse[ID].Anolog[2] = (f32)dims2.lZ;

    return Error;
}

//=========================================================================
static 
dxerr ReadKeyboardImmediateData( device& Device, s32 ID )
{
    dxerr   Error;
    byte    diks[256];   // DirectInput keyboard state buffer 
    
    // Get the input's device state, and put the state in dims
    ZeroMemory( &diks, sizeof(diks) );

    Error = Device.pDevice->GetDeviceState( sizeof(diks), &diks );
    if( FAILED(Error) ) 
    {
        // DirectInput may be telling us that the input stream has been
        // interrupted.  We aren't tracking any state between polls, so
        // we don't have any special reset that needs to be done.
        // We just re-acquire and try again.
        
        // If input is lost then acquire and keep trying 
        Error = Device.pDevice->Acquire();
        while( Error == DIERR_INPUTLOST ) 
            Error = Device.pDevice->Acquire();

        // Update the dialog text 
        if( Error == DIERR_OTHERAPPHASPRIO || 
            Error == DIERR_NOTACQUIRED ) 
        {
            // Lost Device
        //    SetDlgItemText( hDlg, IDC_DATA, "Unacquired" );
        }

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return Error; 
    }
    
    // Make a string of the index values of the keys that are down
    for( s32 i = 0; i < ( INPUT_KBD__END - INPUT_KBD__BEGIN ) ; i++ ) 
    {
        xbool Pressed = ( diks[i] & 0x80 ) != 0;

        if( Pressed )
        {
            if( s_Input.State[0].Keyboard[ID].Digital[i] & DIGITAL_ON )
            {
                s_Input.State[0].Keyboard[ID].Digital[i] &= ~DIGITAL_DEBAUNCE;
            }
            else
            {
                s_Input.State[0].Keyboard[ID].Digital[i] |= DIGITAL_ON | DIGITAL_DEBAUNCE;
            }
        }
        else
        {
            s_Input.State[0].Keyboard[ID].Digital[i] &= (~(DIGITAL_ON|DIGITAL_DEBAUNCE));
        }                    

    }

    //ASSERT( s_Input.State[0].Keyboard[0].Digital[2] == 0 );        

    return Error;
}


//=========================================================================
static 
dxerr ReadJoystickImmediateData( device& Device, s32 ID )
{
    dxerr           Error;
    DIJOYSTATE2     dipad2;      // DirectInput Joystick state structure

    // Get the input's device state, and put the state in dims
    ZeroMemory( &dipad2, sizeof(dipad2) );

    Error = Device.pDevice->GetDeviceState( sizeof(DIJOYSTATE2), &dipad2 );
    if( FAILED(Error) ) 
    {
        // DirectInput may be telling us that the input stream has been
        // interrupted.  We aren't tracking any state between polls, so
        // we don't have any special reset that needs to be done.
        // We just re-acquire and try again.
        
        // If input is lost then acquire and keep trying 
        Error = Device.pDevice->Acquire();
        while( Error == DIERR_INPUTLOST ) 
            Error = Device.pDevice->Acquire();

        // Update the dialog text 
        if( Error == DIERR_OTHERAPPHASPRIO || 
            Error == DIERR_NOTACQUIRED ) 
        {
            // Lost device
        //    SetDlgItemText( hDlg, IDC_DATA, "Unacquired" );
        }

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return Error; 
    }
    
    // The dipad2 structure now has the state of the pad.
    {
        if( Device.Flags & DEVICE_FLG_PS2_PAD )
        {
        }
        else
        {
            for( s32 i=0; i<(INPUT_PC_BTN_31 - INPUT_PC_BTN_0); i++ )
            {
                s_Input.State[0].PCPad[ID].Digital[ i ] = ( dipad2.rgbButtons[ i ] & 0x80) !=0;
            }

            s_Input.State[0].PCPad[ID].Anolog[0] = (f32)dipad2.lX;
            s_Input.State[0].PCPad[ID].Anolog[1] = (f32)dipad2.lY;
            s_Input.State[0].PCPad[ID].Anolog[2] = (f32)dipad2.lZ;

            s_Input.State[0].PCPad[ID].Anolog[3] = (f32)dipad2.lRx;
            s_Input.State[0].PCPad[ID].Anolog[4] = (f32)dipad2.lRy;
            s_Input.State[0].PCPad[ID].Anolog[5] = (f32)dipad2.lRz;
        }
    }
    
    return Error;
}

//=========================================================================
static 
dxerr ReadJoystickBufferedData( device& Device, s32 ID )
{
    DIDEVICEOBJECTDATA didod[ MAX_EVENTS ];  // Receives buffered data 
    DWORD              dwElements;
    dxerr              Error;

    dwElements = MAX_EVENTS;
    Error = Device.pDevice->GetDeviceData( sizeof(DIDEVICEOBJECTDATA), didod, &dwElements, 0 );
    if( Error != DI_OK ) 
    {
        // We got an error or we got DI_BUFFEROVERFLOW.
        //
        // Either way, it means that continuous contact with the
        // device has been lost, either due to an external
        // interruption, or because the buffer overflowed
        // and some events were lost.
        //
        // Consequently, if a button was pressed at the time
        // the buffer overflowed or the connection was broken,
        // the corresponding "up" message might have been lost.
        //
        // But since our simple sample doesn't actually have
        // any state associated with button up or down events,
        // there is no state to reset.  (In a real game, ignoring
        // the buffer overflow would result in the game thinking
        // a key was held down when in fact it isn't; it's just
        // that the "up" event got lost because the buffer
        // overflowed.)
        //
        // If we want to be cleverer, we could do a
        // GetDeviceState() and compare the current state
        // against the state we think the device is in,
        // and process all the states that are currently
        // different from our private state.
        Error = Device.pDevice->Acquire();
        while( Error == DIERR_INPUTLOST ) 
            Error = Device.pDevice->Acquire();

        // Update the dialog text 
        if( Error == DIERR_OTHERAPPHASPRIO || 
            Error == DIERR_NOTACQUIRED ) 
        {
        //    SetDlgItemText( hDlg, IDC_DATA, "Unacquired" );
        }

        // hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
        // may occur when the app is minimized or in the process of 
        // switching, so just try again later 
        return Error; 
    }

    if( FAILED(Error) )  
        return Error;

    // Study each of the buffer elements and process them.
    //
    // Since we really don't do anything, our "processing"
    // consists merely of squirting the name into our
    // local buffer.
/*
    //---------------------------------------------------------------------
    // PS2 DIGITAL GADGETS
    //---------------------------------------------------------------------
    INPUT_PS2__DIGITAL,

    INPUT_PS2_BTN_ANALOG_MODE,                

    INPUT_PS2_BTN_SELECT,                 
    INPUT_PS2_BTN_START,                  
                      
    INPUT_PS2_BTN_L_STICK,                
    INPUT_PS2_BTN_R_STICK,                

    //---------------------------------------------------------------------
    // PS2 ANOLOG GADGETS
    //---------------------------------------------------------------------
    INPUT_PS2__ANOLOG,

    INPUT_PS2_BTN_D_UP,                   
    INPUT_PS2_BTN_D_LEFT,                 
    INPUT_PS2_BTN_D_RIGHT,                
    INPUT_PS2_BTN_D_DOWN,                 
                                
    INPUT_PS2_BTN_L1,                     
    INPUT_PS2_BTN_L2,                     
    INPUT_PS2_BTN_R1,                     
    INPUT_PS2_BTN_R2,                     
                                
    INPUT_PS2_BTN_TRIANGLE,               
    INPUT_PS2_BTN_SQUARE,                 
    INPUT_PS2_BTN_CIRCLE,                 
    INPUT_PS2_BTN_CROSS,                  

    INPUT_PS2_STICK_LEFT_X,               
    INPUT_PS2_STICK_LEFT_Y,               
    INPUT_PS2_STICK_RIGHT_X,              
    INPUT_PS2_STICK_RIGHT_Y,              
    
    INPUT_PS2__END,              
*/
    for( u32 i = 0; i < dwElements; i++ ) 
    {
        state& State = GetState( didod[ i ].dwTimeStamp );

        if( Device.Flags & DEVICE_FLG_PS2_PAD )
        {            
            static s32 BtnMap[]=
            {
                INPUT_PS2_BTN_TRIANGLE  - INPUT_PS2_BTN_L2,   //  0             
                INPUT_PS2_BTN_CIRCLE    - INPUT_PS2_BTN_L2,   //  1           
                INPUT_PS2_BTN_CROSS     - INPUT_PS2_BTN_L2,   //  2                  
                INPUT_PS2_BTN_SQUARE    - INPUT_PS2_BTN_L2,   //  3                 

                INPUT_PS2_BTN_L2        - INPUT_PS2_BTN_L2,   //  4                     
                INPUT_PS2_BTN_R2        - INPUT_PS2_BTN_L2,   //  5                     
  
                INPUT_PS2_BTN_L1        - INPUT_PS2_BTN_L2,   //  6                     
                INPUT_PS2_BTN_R1        - INPUT_PS2_BTN_L2,   //  7                     
  
                INPUT_PS2_BTN_SELECT    - INPUT_PS2_BTN_SELECT,  //  8
                INPUT_PS2_BTN_START     - INPUT_PS2_BTN_SELECT,  //  9

                INPUT_PS2_BTN_R_STICK   - INPUT_PS2_BTN_SELECT,  // 10
                INPUT_PS2_BTN_L_STICK   - INPUT_PS2_BTN_SELECT,  // 11
  
                INPUT_PS2_BTN_L_UP      - INPUT_PS2_BTN_L2,   // 12                  
                INPUT_PS2_BTN_L_RIGHT   - INPUT_PS2_BTN_L2,   // 13              
                INPUT_PS2_BTN_L_DOWN    - INPUT_PS2_BTN_L2,   // 14               
                INPUT_PS2_BTN_L_LEFT    - INPUT_PS2_BTN_L2,   // 15               
            };  

			if( (Device.Flags & DEVICE_FLG_SOYO_PS2_PAD) ||
                (Device.Flags & DEVICE_FLG_DUALSHOCK_CONVERTER_PS2_PAD) ) // SOYO Uses POV Hat switch, so we must interpret results for buttons 12-15
			{
				if( didod[ i ].dwOfs >= DIJOFS_POV(0) && didod[ i ].dwOfs <= DIJOFS_POV(3) ) 			
				{
					if( didod[ i ].dwData == 0xFFFFFFFF ) // POV Hat Switch released
					{
						if( State.PS2Pad[ID].Anolog[ 12 ] > 0.01f ) 
                            State.PS2Pad[ID].Anolog[ 12 ] = State.PS2Pad[ID].Anolog[ 12 ] - 1;
						if( State.PS2Pad[ID].Anolog[ 13 ] > 0.01f ) 
                            State.PS2Pad[ID].Anolog[ 13 ] = State.PS2Pad[ID].Anolog[ 13 ] - 1;
						if( State.PS2Pad[ID].Anolog[ 14 ] > 0.01f ) 
                            State.PS2Pad[ID].Anolog[ 14 ] = State.PS2Pad[ID].Anolog[ 14 ] - 1;
						if( State.PS2Pad[ID].Anolog[ 15 ] > 0.01f ) 
                            State.PS2Pad[ID].Anolog[ 15 ] = State.PS2Pad[ID].Anolog[ 15 ] - 1;
					}
					else // Hat Switch Pressed.
					{
						// SOYO USB Converter gives the following values in dwData 
						// INPUT_PS2_BTN_L_UP		= 0
						// INPUT_PS2_BTN_L_RIGHT	= 9000,
						// INPUT_PS2_BTN_L_DOWN		= 18000
						// INPUT_PS2_BTN_L_LEFT 	= 27000

						s32 Index = didod[ i ].dwData/9000 + 12; // Mapped to buttons 12-15 
                        State.PS2Pad[ID].Anolog[ Index ] = 1.0001f;
					}
				}

                if( Device.Flags & DEVICE_FLG_DUALSHOCK_CONVERTER_PS2_PAD )
                {
                    f32 Temp;

                    Temp                        = State.PS2Pad[ID].Anolog[18];
                    State.PS2Pad[ID].Anolog[18] = State.PS2Pad[ID].Anolog[19];
                    State.PS2Pad[ID].Anolog[19] = Temp;
                }
			}

			// OLD
            //if( didod[ i ].dwOfs >= DIJOFS_BUTTON0 && didod[ i ].dwOfs <= DIJOFS_BUTTON31 ) 

			if( didod[ i ].dwOfs >= DIJOFS_BUTTON0 && didod[ i ].dwOfs <= DIJOFS_BUTTON31 ) 			
            {
                s32 Index = didod[ i ].dwOfs - DIJOFS_BUTTON0;

                ASSERT( Index >= 0 );
                ASSERT( Index < ANALOG_COUNT_PS2PAD );

                if( Index >= 8 && Index <= 11 )
                {
                    Index = BtnMap[ Index ];

                    ASSERT( Index >= 0 );
                    ASSERT( Index < ANALOG_COUNT_PS2PAD );

                    if( (didod[ i ].dwData & 0x80) )
                    {
                        State.PS2Pad[ID].Digital[ Index ] |= DIGITAL_ON | DIGITAL_DEBAUNCE;
                    }
                    else
                    {
                        State.PS2Pad[ID].Digital[ Index ] &= (~DIGITAL_ON);
                    }                    
                }
                else
                {
                    Index = BtnMap[ Index ];

                    //DebugMessage( "%d \n", Index );

                    ASSERT( Index >= 0 );
                    ASSERT( Index < ANALOG_COUNT_PS2PAD );

                    if( didod[ i ].dwData & 0x80 )
                    {
                        State.PS2Pad[ID].Anolog[ Index ] = 1.0001f;
                    }
                    else
                    {
                        if( State.PS2Pad[ID].Anolog[ Index ] > 0.01f ) 
                            State.PS2Pad[ID].Anolog[ Index ] = State.PS2Pad[ID].Anolog[ Index ] - 1;
                    }                    
                }
            }
            else if( didod[ i ].dwOfs >= DIJOFS_X && didod[ i ].dwOfs <= DIJOFS_RZ ) 
            {
                s32 Index = (didod[ i ].dwOfs - DIJOFS_X)>>2;

                ASSERT( Index >= 0 );
                ASSERT( Index < ANALOG_COUNT_PS2PAD );

                if( Index == 0 )
                {
                    Index = INPUT_PS2_STICK_LEFT_X  - INPUT_PS2_BTN_L2 ;
                    State.PS2Pad[ID].Anolog[ Index ] = (f32)((s32)didod[ i ].dwData)/ 1000.0f;    
                }
                else if( Index == 1 )
                {
                    Index = INPUT_PS2_STICK_LEFT_Y  - INPUT_PS2_BTN_L2 ;
                    State.PS2Pad[ID].Anolog[ Index ] = (f32)((s32)didod[ i ].dwData)/-1000.0f;    
                }
                else if( Index == 2 )
                {
                    //Index = INPUT_PS2_STICK_RIGHT_X  - INPUT_PS2_BTN_L2 ;
                    //State.PS2Pad[ID].Anolog[ Index ] = (f32)((s32)didod[ i ].dwData)/ 1000.0f;    
                    Index = INPUT_PS2_STICK_RIGHT_Y  - INPUT_PS2_BTN_L2;
                    State.PS2Pad[ID].Anolog[ Index ] = (f32)((s32)didod[ i ].dwData)/ -1000.0f;    
                }
                else if( Index == 5 )
                {
                    //Index = INPUT_PS2_STICK_RIGHT_Y  - INPUT_PS2_BTN_L2;
                    //State.PS2Pad[ID].Anolog[ Index ] = (f32)((s32)didod[ i ].dwData)/-1000.0f;    
                    Index = INPUT_PS2_STICK_RIGHT_X  - INPUT_PS2_BTN_L2 ;
                    State.PS2Pad[ID].Anolog[ Index ] = (f32)((s32)didod[ i ].dwData)/ 1000.0f;    
                }
            }
        }
        else
        {
            //
            // Set the button presses
            //
            if( didod[ i ].dwOfs >= DIJOFS_BUTTON0 && didod[ i ].dwOfs <= DIJOFS_BUTTON31 ) 
            {
                s32 Index = didod[ i ].dwOfs - DIJOFS_BUTTON0;

                ASSERT( Index >= 0 );
                ASSERT( Index < DIGITAL_COUNT_PCPAD );

                if( (didod[ i ].dwData & 0x80) )
                {
                    State.PCPad[ID].Digital[ Index ] |= DIGITAL_ON | DIGITAL_DEBAUNCE;
                }
                else
                {
                    State.PCPad[ID].Digital[ Index ] &= (~DIGITAL_ON);
                }
            }
            else if( didod[ i ].dwOfs >= DIJOFS_X && didod[ i ].dwOfs <= DIJOFS_RZ ) 
            {
                s32 Index = (didod[ i ].dwOfs - DIJOFS_X)>>2;

                ASSERT( Index >= 0 );
                ASSERT( Index < ANALOG_COUNT_PCPAD );

                //DebugMessage( "%d %d\n", Index, didod[ i ].dwData );

                State.PCPad[ID].Anolog[ Index ] = (f32)((s32)didod[ i ].dwData)/1000.0f;
            }
        }
    }
/*

    for(  i=0; i<16; i++ )
    {
        x_DebugMsg( "%2d ", i );
    }
    x_DebugMsg( "\n");

    for(  i=0; i<16; i++ )
    {
        x_DebugMsg( "%2d ", (s32)s_Input.State[ s_Input.iState ].PS2Pad[ 0 ].Anolog[ i ] );
    }
    x_DebugMsg( "\n");

    //x_DebugMsg("\n");
*/
    return Error;
}


//=========================================================================
static
xbool ProcessEvents( void )
{ 
    //
    // Check whether we have more events to porcess
    //
    if( s_Input.iStateNext >= s_Input.nStates )
        return FALSE;

    //
    // Advance the process queue
    //
    s_Input.iState = s_Input.iStateNext;
    ASSERT( s_Input.iState < MAX_STATES );

    //
    // Check whether we have more events to porcess
    //
    s_Input.iStateNext++;
   // DebugMessage( "%d\n", s_Input.nStates );

    return TRUE;
}

//========================================================================
static
s64 TIME_GetClock( void )
{
    // use same clock than direct input
    return GetTickCount();
}

//========================================================================
static
s64 TIME_GetTicksPerSecond( void )
{
    return 1000;
}

//=========================================================================
// This is more like a hack trying to get the stupid joystics to work. 
// For editors and such. DInput is just a pain to put up with.
//=========================================================================
static 
void ReadWindowsJoy( device& Device, s32 ID )
{
    JOYINFOEX ExInfo;

    state& State = GetState( 0 ); //GetTickCount() );

    x_memset( &ExInfo, 0, sizeof(ExInfo) );
    ExInfo.dwSize = sizeof(ExInfo);
    ExInfo.dwFlags = JOY_RETURNALL ;
    joyGetPosEx( 0, &ExInfo );

    // SB - This currently works for the EMS USB convertor!
    //      If other convertors are returning incorrect
    //      values, then create a new mapping table for it
    //      and check the Device.Flags to see which convertor
    //      is being used. DON'T JUST MODIFY THIS TABLE!!!!
    static s32 MapPC2PS2[] = 
    { 
        INPUT_PS2_BTN_TRIANGLE,         // 0
        INPUT_PS2_BTN_CIRCLE,           // 1
        INPUT_PS2_BTN_CROSS,            // 2
        INPUT_PS2_BTN_SQUARE,           // 3
        INPUT_PS2_BTN_L2,               // 4
        INPUT_PS2_BTN_R2,               // 5
        INPUT_PS2_BTN_L1,               // 6
        INPUT_PS2_BTN_R1,               // 7
        INPUT_PS2_BTN_SELECT,           // 8
        INPUT_PS2_BTN_START,            // 9
        INPUT_PS2_BTN_L_STICK,          // 10
        INPUT_PS2_BTN_R_STICK,          // 11
        INPUT_PS2_BTN_L_UP,             // 12
        INPUT_PS2_BTN_L_RIGHT,          // 13
        INPUT_PS2_BTN_L_DOWN,           // 14
        INPUT_PS2_BTN_L_LEFT,           // 15
    };

    //
    // Handle buttons
    // 
    for( s32 i=0; i<16; i++ )
    {
        s32 Gadget = MapPC2PS2[i];

        if( Gadget < 0 )
            continue;

        //
        // Digital Buttons
        //
        s32 Index = Gadget - INPUT_PS2_BTN_L2;
        if( ExInfo.dwButtons&(1<<i) )
        {
            if( State.PS2Pad[ID].Digital[ Index ] & DIGITAL_ON )
            {
                State.PS2Pad[ID].Digital[ Index ] &= ~DIGITAL_DEBAUNCE;
            }
            else
            {
                State.PS2Pad[ID].Digital[ Index ] |= DIGITAL_ON | DIGITAL_DEBAUNCE;
            }
        }
        else
        {
            State.PS2Pad[ID].Digital[ Index ] &= (~(DIGITAL_ON|DIGITAL_DEBAUNCE));
        }                    
    }

    if ( Device.Flags & DEVICE_FLG_PSX_USB_PAD )
    {
        // bah. stupid inconsistent PC crap. this particular model returns dpad buttons
        // as a hat POV angle. wtf?
        if ( ExInfo.dwFlags & JOY_RETURNPOV )
        {
            // we have a POV angle, figure out where it is
            xbool IsPressed[4]; // up, right, down, left
            if ( ExInfo.dwPOV != 0xffff )
            {
                IsPressed[0] = (ExInfo.dwPOV >= 29250) || (ExInfo.dwPOV <  6750);
                IsPressed[1] = (ExInfo.dwPOV >=  2250) && (ExInfo.dwPOV < 15750);
                IsPressed[2] = (ExInfo.dwPOV >= 11250) && (ExInfo.dwPOV < 24750);
                IsPressed[3] = (ExInfo.dwPOV >= 20250) && (ExInfo.dwPOV < 33750);
            }
            else
            {
                IsPressed[0] = FALSE;
                IsPressed[1] = FALSE;
                IsPressed[2] = FALSE;
                IsPressed[3] = FALSE;
            }

            for ( s32 iPressed = 0; iPressed < 4; iPressed++ )
            {
                s32 Index = MapPC2PS2[iPressed+12] - INPUT_PS2_BTN_L2;
                if ( IsPressed[iPressed] )
                {
                    if( State.PS2Pad[ID].Digital[ Index ] & DIGITAL_ON )
                    {
                        State.PS2Pad[ID].Digital[ Index ] &= ~DIGITAL_DEBAUNCE;
                    }
                    else
                    {
                        State.PS2Pad[ID].Digital[ Index ] |= DIGITAL_ON | DIGITAL_DEBAUNCE;
                    }
                }
                else
                {   
                    State.PS2Pad[ID].Digital[ Index ] &= (~(DIGITAL_ON|DIGITAL_DEBAUNCE));
                }
            }
        }
    }

    const f32 Max_S16    = 0xffff/2;
    const f32 InvMax_S16 = 1.0f/Max_S16;
    State.PS2Pad[ID].Anolog[INPUT_PS2_STICK_LEFT_X -INPUT_PS2_BTN_L2] = (((f32)ExInfo.dwXpos) - Max_S16)* InvMax_S16;
    State.PS2Pad[ID].Anolog[INPUT_PS2_STICK_LEFT_Y -INPUT_PS2_BTN_L2] = (((f32)ExInfo.dwYpos) - Max_S16)*-InvMax_S16;
    State.PS2Pad[ID].Anolog[INPUT_PS2_STICK_RIGHT_X-INPUT_PS2_BTN_L2] = (((f32)ExInfo.dwZpos) - Max_S16)* InvMax_S16;
    State.PS2Pad[ID].Anolog[INPUT_PS2_STICK_RIGHT_Y-INPUT_PS2_BTN_L2] = (((f32)ExInfo.dwRpos) - Max_S16)*-InvMax_S16;
}

//=========================================================================

xbool input_UpdateState2( s32 Depth = 0 )
{
    //x_DebugMsg("Entering iUs\n");
    //
    // check whether we are getting events from the queue or we are collecting more events
    //
    if( s_Input.bProcessEvents )
    {
        // 
        // Process the next set of events
        //
        if( ProcessEvents() )
        {
            //x_DebugMsg("Leaving iUs\n");
            return TRUE;
        }
        
        // 
        // We are done processing events for this time interval
        //
        s_Input.bProcessEvents = FALSE;
        //x_DebugMsg("Leaving iUs\n");
        return FALSE;
    }

    //
    // Read all messages for the system
    //
    if(!s_DoNotProcessWindowsMessages)
    {
        while( PeekMessage( &s_Input.Msg, NULL, 0, 0, PM_NOREMOVE) )
        {
            if ( !GetMessage( &s_Input.Msg, NULL, 0, 0 ) )
            {
                s_Input.ExitApp = TRUE;
                return FALSE;
            }

            TranslateMessage( &s_Input.Msg );
            DispatchMessage ( &s_Input.Msg );
        }
        d3deng_ComputeMousePos();
    }
    //
    // Update the timer as well as the type of input query
    //
    s_Input.LastTimeFrame    = s_Input.CurrentTimeFrame;
    s_Input.CurrentTimeFrame = TIME_GetClock(); 
    if( (s_Input.TicksPerRefresh) && ((( s_Input.CurrentTimeFrame - s_Input.LastTimeFrame)/s_Input.TicksPerRefresh) > MAX_STATES) )
    {
        //
        // The user waited to long to ask the input system for events. Now
        // we have too many states to fit in the queue. So lets jump to 
        // Immediate mode
        //
        s_Input.bImmediate = TRUE;
        //DebugMessage( "s_Input.bImmediate = TRUE;\n");

    }
    else
    {
        //DebugMessage( "s_Input.bImmediate = FALSE;\n");
        //s_Input.bImmediate = FALSE;
    }

    //
    // Tell the queue to prepare since we are about to start collecting events
    //
    if( Depth == 0 )
        PageFlipQueue();

    //
    // Read input for all the mouses
    //
    if(!s_DoNotProcessWindowsMessages)
    {
        for( s32 m=0; m<s_Input.nMouses; m++ )
        {
            if( s_Input.bImmediate )
            {
                ReadMouseImmediateData( s_Input.Mouse[m], m );
            }
            else
            {
                ReadMouseBufferedData( s_Input.Mouse[m], m );
            }
        }

    //
    // Read input for all the keyboards
    //
        for( s32 k=0; k<s_Input.nKeyboards; k++ )
        {
            if( s_Input.bImmediate )
            {
                ReadKeyboardImmediateData( s_Input.Keyboard[k], k );
            }
            else
            {
                ReadKeyboadBufferedData( s_Input.Keyboard[k], k );
            }
        }
    }

    //
    // Read input for all the joysticks
    //
    for( s32 j=0; j<s_Input.nJoysticks; j++ )
    {
        //if( s_Input.bImmediate )
        if( 1 )
        {
            // HACKED VERSION
            if( Depth == 0 )
                ReadWindowsJoy( s_Input.Joystick[j], j );
        }
        else
        {
            if(0)
            {
                ReadJoystickImmediateData( s_Input.Joystick[j], j );
            }
            else
            {
                ReadJoystickBufferedData( s_Input.Joystick[j], j );
            }
        }
    }

    //
    // Set the next stage of the input system
    //
    s_Input.bProcessEvents = TRUE;

    //x_DebugMsg("Leaving iUs\n");
    
    return input_UpdateState2( Depth++ );
}

xbool input_UpdateState( void )
{
    return input_UpdateState2( 0 );
}

//=========================================================================

void d3deng_KillInput( void )
{
}

//=========================================================================

void d3deng_DoNotProcessWindowsMessages()
{
    s_DoNotProcessWindowsMessages = TRUE;
}

xbool d3deng_InitInput( HWND Window )
{
    dxerr   Error;

    //
    // Set our input in contex of a window
    //
    s_Input.Window             = Window;

    //
    // Set our defauls
    //
    s_Input.bExclusive         = FALSE;
    s_Input.bForeground        = TRUE;
    s_Input.bImmediate         = FALSE;
    s_Input.bDisableWindowsKey = TRUE;

#ifndef X_EDITOR
    s_Input.bExclusive = TRUE;
    s_Input.bImmediate = TRUE;
#endif

#ifdef sbroumley
    s_Input.bExclusive         = FALSE;
    s_Input.bForeground        = TRUE;
    s_Input.bImmediate         = FALSE;
    s_Input.bDisableWindowsKey = TRUE;
#endif

    s_Input.TicksPerRefresh    = TIME_GetTicksPerSecond() / REFRESH_RATE;

    //
    // Initialize all the devices indirections to -1
    //
    for( s32 i=0; i<MAX_DEVICES; i++ )
    {
        s_Input.PCPadDevice[i]  = -1;
        s_Input.PS2PadDevice[i] = -1;
        s_Input.KeybdDevice[i]  = -1;
        s_Input.MouseDevice[i]  = -1;
    }

    //
    // Create a DInput object
    //
    Error = DirectInput8Create( GetModuleHandle(NULL), DIRECTINPUT_VERSION, 
                                IID_IDirectInput8, (VOID**)&s_Input.pDInput, NULL );
    if( FAILED( Error ) )
    {
        return FALSE;
    }

    //
    // Create all the keyboards
    //
    Error = s_Input.pDInput->EnumDevices( DI8DEVCLASS_KEYBOARD, 
                                          EnumKeyboardCallback,
                                          NULL, DIEDFL_ATTACHEDONLY );
    if( FAILED( Error ) )
    {
        d3deng_KillInput();
        return FALSE;
    }

    //
    // Create all the Mouses
    //
    if( 1 )
    {
        Error = s_Input.pDInput->EnumDevices( DI8DEVCLASS_POINTER, 
                                              EnumMouseCallback,
                                              NULL, DIEDFL_ATTACHEDONLY );
        if( FAILED( Error ) )
        {
            d3deng_KillInput();
            return FALSE;
        }
    }

    //
    // Create all the Joysticks
    //
    Error = s_Input.pDInput->EnumDevices( DI8DEVCLASS_GAMECTRL, 
                                          EnumJoysticksCallback,
                                          NULL, DIEDFL_ATTACHEDONLY );
    if( FAILED( Error ) )
    {
        d3deng_KillInput();
        return FALSE;
    }


    return TRUE;
}

//=========================================================================
static
f32 GetValue( s32 ControllerID, input_gadget GadgetID, digital_type DigitalType )
{
    ASSERT( ControllerID >= 0 );
    ASSERT( ControllerID < MAX_DEVICES );

    if( GadgetID < INPUT_KBD__END && GadgetID > INPUT_KBD__BEGIN )
    {
        s32 DeviceID = s_Input.KeybdDevice[ ControllerID ];

        if( DeviceID >= 0 )
        {
            s32 Index = GadgetID - INPUT_KBD__DIGITAL + 1;
            return (f32)( s_Input.State[ s_Input.iState ].Keyboard[ ControllerID ].Digital[ Index ] & DigitalType );
        }
    }

    if( GadgetID < INPUT_MOUSE__END && GadgetID > INPUT_MOUSE__BEGIN )
    {
        switch( GadgetID )
        {
        case INPUT_MOUSE_X_REL:     return s_Input.State[0].Mouse[ControllerID].Anolog[0]; //return d3deng_GetMouseX();
        case INPUT_MOUSE_Y_REL:     return s_Input.State[0].Mouse[ControllerID].Anolog[1]; //return d3deng_GetMouseY();
        case INPUT_MOUSE_WHEEL_REL: return s_Input.State[0].Mouse[ControllerID].Anolog[2]; //return d3deng_GetMouseWheel();
        case INPUT_MOUSE_BTN_L:     return s_Input.State[0].Mouse[ControllerID].Digital[0]; //return (f32)d3deng_MouseGetLButton();
        case INPUT_MOUSE_BTN_R:     return s_Input.State[0].Mouse[ControllerID].Digital[1]; //return (f32)d3deng_MouseGetRButton();
        case INPUT_MOUSE_BTN_C:     return s_Input.State[0].Mouse[ControllerID].Digital[2]; //return (f32)d3deng_MouseGetMButton();
        } 
/*
        s32 DeviceID = s_Input.MouseDevice[ ControllerID ];

        if( DeviceID >= 0 )
        {
            if( GadgetID < INPUT_MOUSE__ANALOG )
            {
                s32 Index = GadgetID - INPUT_MOUSE__DIGITAL -1;
                return (f32)(s_Input.State[ s_Input.iState ].Mouse[ ControllerID ].Digital[ Index ] & DigitalType );
            }
            else
            {
                s32 Index = GadgetID - INPUT_MOUSE__ANALOG -1;
                return s_Input.State[ s_Input.iState ].Mouse[ ControllerID ].Anolog[ Index ];
            }
        }
*/
    }

    if( GadgetID < INPUT_PC__END && GadgetID > INPUT_PC__BEGIN )
    {
        s32 DeviceID = s_Input.PCPadDevice[ ControllerID ];

        if( DeviceID >= 0 )
        {
            if( GadgetID < INPUT_PC__ANALOG )
            {
                s32 Index = GadgetID - INPUT_PC__DIGITAL -1;
                return (f32)(s_Input.State[ s_Input.iState ].PCPad[ ControllerID ].Digital[ Index ] & DigitalType );
            }
            else
            {
                s32 Index  = GadgetID - INPUT_PC__ANALOG -1;
                return s_Input.State[ s_Input.iState ].PCPad[ ControllerID ].Anolog[ Index ];
            }
        }
    }

    if( GadgetID < INPUT_PS2__END && GadgetID > INPUT_PS2__BEGIN )
    {
        s32 DeviceID = s_Input.PS2PadDevice[ ControllerID ];

        if( DeviceID >= 0 )
        {
            // Check if it is a digital button
            if( (GadgetID>=INPUT_PS2_BTN_L2) && (GadgetID<INPUT_PS2_STICK_LEFT_X) )
            {
                s32 Index = GadgetID - INPUT_PS2_BTN_L2;
                return (f32)(s_Input.State[ s_Input.iState ].PS2Pad[ ControllerID ].Digital[ Index ] & DigitalType );
            }
            else
            // Must be an analog
            {
                s32 Index = GadgetID - INPUT_PS2_BTN_L2;

                if( Index >= (INPUT_PS2_STICK_LEFT_X-INPUT_PS2_BTN_L2) && Index <= (INPUT_PS2_STICK_RIGHT_Y-INPUT_PS2_BTN_L2) )
                {
                    const f32 DEADZONE = 0.25f;
                    f32 V = s_Input.State[ s_Input.iState ].PS2Pad[ ControllerID ].Anolog[ Index ];

                    // Remove dead zone
                    if( V>=0.0f )
                    {
                        V -= DEADZONE ;
                        if( V<0.0f ) V = 0.0f;
                    }
                    else
                    {
                        V += DEADZONE ;
                        if( V>0.0f ) V = 0.0f;
                    }

                    // Scale back to max range
                    V *= 1.0f / (1.0f - DEADZONE) ;
    
                    // Clamp
                    if( V>1.0f ) V = 1.0f;
                    if( V<-1.0f) V = -1.0f;

                    return V;
                }
/*
                else
                if( DigitalType == DIGITAL_DEBAUNCE )
                {
                    if( (s_Input.State[ s_Input.iState ].PS2Pad[ ControllerID ].Anolog[ Index ] - 1) > 0 )
                    {
                        return 1;
                    }
                    else
                    {
                        if( s_Input.State[ s_Input.iState ].PS2Pad[ ControllerID ].Anolog[ Index ] == 0 )
                            return 0;

                        return 1;
                    }
                }
*/
                return s_Input.State[ s_Input.iState ].PS2Pad[ ControllerID ].Anolog[ Index ];
            }
        }
    }

    if( GadgetID == INPUT_MSG_EXIT )
    {
        return (f32)(s_Input.ExitApp);
    }

    return 0;
}

//=========================================================================

xbool input_WasPressed( input_gadget GadgetID, s32 ControllerID )
{
    return GetValue( ControllerID, GadgetID, DIGITAL_DEBAUNCE ) != 0;
}

//=========================================================================

xbool input_IsPressed( input_gadget GadgetID, s32 ControllerID )
{
    return GetValue( ControllerID, GadgetID, DIGITAL_ON ) != 0;
}

//=========================================================================

f32 input_GetValue( input_gadget GadgetID, s32 ControllerID )
{
    return GetValue( ControllerID, GadgetID, DIGITAL_ON );
}

//==============================================================================

void input_Feedback( f32 Duration, f32 Intensity, s32 ControllerID )
{
    // PC, this does nothing right now but should control any force-feedback device
    (void)Duration;
    (void)Intensity;
    (void)ControllerID;
}

//==============================================================================

void input_Feedback( s32 Count, feedback_envelope* pEnvelope, s32 ControllerID )
{
    (void)Count;
    (void)pEnvelope;
    (void)ControllerID;
}

//=============================================================================

void input_EnableFeedback( xbool state, s32 ControllerID )
{
    (void)state;
    (void)ControllerID;
}

//==============================================================================

void input_SuppressFeedback( xbool Suppress )
{
    (void)Suppress;
}

//==============================================================================

void input_Kill( void )
{
}

//==============================================================================

xbool input_IsPresent( input_gadget GadgetID, s32 ControllerID )
{
    (void) GadgetID;
    (void) ControllerID;
    return TRUE;
}

//==============================================================================

void input_ClearFeedback( void )
{
}

//==============================================================================

