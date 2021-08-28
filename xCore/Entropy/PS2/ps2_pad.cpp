#include "e_engine.hpp"
#include "x_threads.hpp"

#if !defined(TARGET_PS2)
#error This is for a PS2 target build. Please exclude from build rules.
#endif

#include "ps2_misc.hpp"

#define NUM_PORTS   2

struct pad
{
    u32             prev_buttons;
    u32             buttons;
    byte            Raw[32];
    f32             LX,LY;
    f32             RX,RY;
    s32             state;
    s32             phase;

    f32             GF;
    xbool           Connected;
};

struct pads
{
    pad             Pad[2];
};

enum feedback_state
{
    FS_OFF,
    FS_ON,
    FS_SWITCH_OFF,
    FS_SWITCH_ON,
    FS_DELAY,
};

struct feedback
{
    xbool   Enabled;
    f32     Duration;
    f32     Intensity;
    s32     Count;
    feedback_envelope *pEnvelope;
    feedback_envelope StaticEnvelope;
    feedback_state State;
    u8      Actuator[6];
};

#define PAD_QUEUE_SIZE 32
static pads*    s_pCurrentPad;
static pads*    s_pPreviousPad;
static xmesgq*  s_pqPadReady;
static xmesgq*  s_pqPadAvailable;
//static xthread* s_pUpdateThread;
static pads     s_PadQueue[ PAD_QUEUE_SIZE ];
static s32      s_Phase[2];
static xbool    s_IsAnalog[2];
static feedback s_Feedback[2];
static xbool    s_SuppressFeedback = FALSE;
static pads     s_ReadState;
static xbool    s_Initialized = FALSE;

static u_long128 dma_buf0[scePadDmaBufferMax] PS2_ALIGNMENT(64);
static u_long128 dma_buf1[scePadDmaBufferMax] PS2_ALIGNMENT(64);
//static pad pad_info[2]={0};
//static unsigned char act_direct[6];
static unsigned char act_align[6];

void UpdateFeedback(void);

int slot;
//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================
//=========================================================================
//static void input_PeriodicUpdate(void);

void input_Init(void)
{
    int ret; 
    s32 i;

    ASSERT( s_Initialized == FALSE );
    printf("PAD: Init begin\n");

    // load sio2man.irx 
    
    scePadInit(0);
    //port = 0;
    slot = 0;
    //term_id = 0;
    //rterm_id = 0;

    ret = scePadPortOpen( 0, slot, dma_buf0 );
    ASSERT( ret && "ERROR: scePadPortOpen\n" );
    scePadSetMainMode(0, slot, 1, 3);
    scePadInfoAct(0, slot, -1, 0);  

    ret = scePadPortOpen( 1, slot, dma_buf1 );
    ASSERT( ret && "ERROR: scePadPortOpen\n" );
    scePadSetMainMode(1, slot, 1, 3);
    scePadInfoAct(1, slot, -1, 0);  

    x_memset(s_PadQueue, 0, sizeof(s_PadQueue) );
    x_memset(&s_ReadState,0,sizeof(s_ReadState));
    s_pPreviousPad = s_PadQueue;
    s_pCurrentPad  = s_PadQueue;
    s_pqPadReady = new xmesgq(PAD_QUEUE_SIZE);
    s_pqPadAvailable = new xmesgq(PAD_QUEUE_SIZE);

    for( i=0;i<PAD_QUEUE_SIZE;i++ )
    {
        s_pqPadAvailable->Send(&s_PadQueue[i],MQ_BLOCK);
    }
    s_Feedback[0].State     = FS_OFF;
    s_Feedback[0].Duration  = 0.0f;
    s_Feedback[0].Intensity = 0.0f;
    s_Feedback[0].Enabled   = TRUE;
    s_Feedback[1].State     = FS_OFF;
    s_Feedback[1].Duration  = 0.0f;
    s_Feedback[1].Intensity = 0.0f;
    s_Feedback[1].Enabled   = TRUE;
    s_Phase[0]=0;
    s_Phase[1]=0;
    s_IsAnalog[0] = FALSE;
    s_IsAnalog[1] = FALSE;
    //s_pUpdateThread = new xthread(input_PeriodicUpdate,"Controller reader",8192,2);

    printf("PAD: Init end\n");
    s_Initialized = TRUE;
}

//=========================================================================

void input_Kill( void )
{
    ASSERT( s_Initialized );
    s_Initialized = FALSE;
    scePadPortClose( 0, 0 );
    scePadPortClose( 1, 0 );
    scePadEnd();
    //delete s_pUpdateThread;
    delete s_pqPadReady;
    delete s_pqPadAvailable;
}

//=========================================================================

static float AxisValue(unsigned char Input)
{
    static f32 s_DEADZONE = 0.3f;

    // Move range to -1<->1
    // Make range from 0->2
    f32 V = ((f32)Input*(2.0f / 255.0f)) - 1.0f;

    // Remove dead zone
    if( V >= 0.0f )
    {
        V -= s_DEADZONE;
        if( V < 0.0f ) V = 0.0f;
    }
    else
    {
        V += s_DEADZONE;
        if( V > 0.0f ) V = 0.0f;
    }

    // Scale back to max range
    V *= 1.0f / (1.0f - s_DEADZONE);
    
    // Clamp
    if( V >  1.0f) V =  1.0f;
    if( V < -1.0f) V = -1.0f;

    return V;
}

//=========================================================================

static int UpdateValues( int port )
{
    s32 status;
    // Don't overwrite unread data
    ASSERT( s_Initialized );

    pad* pPad = &s_pCurrentPad->Pad[port];    
    ASSERT(pPad);

    status = scePadRead( port, slot, pPad->Raw );
    if( status == 0 )
    {   
        //x_printfxy(0,0,"*");
        return 0;
    }

    //for( s32 i=0; i<16; i++ )
//        x_printfxy(10,i,"%3d %3d %3d %3d",i,(s32)pPad->Raw[i],(s32)pPad->Raw[i+16],i+16);
    
    if( pPad->Raw[0] == 0 )
    {
        if( s_IsAnalog[port] )
        {
            pPad->RX        =  AxisValue(pPad->Raw[4]);
            pPad->RY        = -AxisValue(pPad->Raw[5]);
            pPad->LX        =  AxisValue(pPad->Raw[6]);
            pPad->LY        = -AxisValue(pPad->Raw[7]);
        }
        else
        {
            pPad->RX = 0;
            pPad->RY = 0;
            pPad->LX = 0;
            pPad->LY = 0;

            x_memset( pPad->Raw, 0, sizeof(pPad->Raw) );
            pPad->Raw[2] = 0xff;
            pPad->Raw[3] = 0xff;
        }

        pPad->buttons   = 0xffff ^ ((pPad->Raw[2]<<8)|pPad->Raw[3]);

        pPad->Connected = TRUE;

        if( s_pPreviousPad )
            pPad->prev_buttons = s_pPreviousPad->Pad[port].buttons;
    }

    return 1;
}

//=========================================================================

static void SolveStates( void )
{
    int port;
//    int state;
    int id;
    int exid;
    int i;

    ASSERT( s_Initialized );


    // Loop through each controller
    for( port=0; port<2; port++ )
    {
        int done=0;
        int count=0;

        pad* pPad = &s_pCurrentPad->Pad[port];


        // Go through phases until we reach a known state
        while( (!done) && (count<32) )
        {
            count++;

            pPad->state = scePadGetState(port,slot);

            if( pPad->state==scePadStateDiscon)
            {
                s_Phase[port]     = 0;
                pPad->prev_buttons = 0;
                pPad->buttons    = 0;
                pPad->LX         = 0;
                pPad->LY         = 0;
                pPad->RX         = 0;
                pPad->RY         = 0;
                pPad->Connected  = FALSE;
                s_IsAnalog[port] = FALSE;
            }
            
            switch(s_Phase[port])
            {
                //-----------------------------------------------------------
                // Starting from scratch.  Determine if connected or what
                // type of controller we have
                //-----------------------------------------------------------
                case 0:
   
                        // Check if disconnected
                        if( (pPad->state != scePadStateStable) &&
                            (pPad->state != scePadStateFindCTP1) ) 
                        {
                            break;
                        }

                        id = scePadInfoMode(port, slot, InfoModeCurID, 0 );
                        if( id==0 ) 
                        {
                            //printf("id==0\n");
                            break;
                        }
            
                        exid = scePadInfoMode(port, slot, InfoModeCurExID,0);
                        if( exid>0 ) 
                            id = exid;

                        switch(id)
                        {
                            case 4:// STANDARD 
                            //printf("%s", "STANDARD");
                            s_Phase[port] = 10;
                            break;

                            case 7:// ANALOG 
                            //printf("%s", "ANALOG");
                            s_IsAnalog[port] = TRUE;
                            s_Phase[port] = 20;
                            break;

                            case 2:// NEGI-COM 
                            //sprintf(pPad->name, "%s", "NeGi-CON");
                            s_Phase[port] = 99;
                            break;

                            case 3:// KONAMI-GUN 
                            //sprintf(pPad->name, "%s", "GunCON(Konami)");
                            s_Phase[port] = 99;
                            break;

                            case 5:// JOY STICK 
                            //sprintf(pPad->name, "%s", "JOYSTICK");
                            s_Phase[port] = 99;
                            break;
                            
                            case 6:// NAMCO-GUN 
                            //sprintf(pPad->name, "%s", "GunCON(NAMCO)");
                            s_Phase[port] = 99;
                            break;
                            
                            case 0x100:// TSURI-CON
                            //sprintf(pPad->name, "%s", "TSURI-CON");
                            s_Phase[port] = 99;
                            break;
                            
                            case 0x300:// JOG-CON
                            //sprintf(pPad->name, "%s", "JOG-CON");
                            s_Phase[port] = 99;
                            break;

                            default:
                            printf("UNKNOWN");
                            s_Phase[port] = 99;
                            break;
                        }
                        break;

                //-----------------------------------------------------------
                // STANDARD STATE MACHINE 
                //-----------------------------------------------------------
                case 10:
                if( scePadInfoMode(port, slot, InfoModeCurExID,0)==0 )
                {
                    s_Phase[port] = 99;
                    break;
                }
                s_Phase[port]++;
                //-----------------------------------------------------------
                case 11:
                // switch to Extend mode 
                if( scePadSetMainMode(port, slot, 1, 3)==1 )
                {
                    s_Phase[port]++;
                }
                break;
                //-----------------------------------------------------------
                case 12:
                if( scePadGetState(port,slot)==scePadStateExecCmd ) 
                    break;
                s_Phase[port] = 0;
                break;

                //-----------------------------------------------------------
                // ANALOG STATE MACHINE 
                //-----------------------------------------------------------
                case 20:
                if( scePadEnterPressMode(port,slot) == 0 )
                {
                    // Refuses command...skip to next stage
                    s_Phase[port] = 22;
                }
                s_Phase[port]++;
                break;
                //-----------------------------------------------------------
                case 21:
                if( scePadGetState(port,slot)==scePadStateExecCmd ) 
                    break;
                s_Phase[port]++;
                break;
                //-----------------------------------------------------------
                case 22:
                if( scePadInfoAct(port, slot, -1, 0)==0 )
                {
                    s_Phase[port] = 99; // SCPH-1150 
                }
                // ACTUATER SETTING (DUALSHOK) 
                act_align[ 0 ] = 0;
                act_align[ 1 ] = 1;
                for( i = 2; i < 6; i++ )
                    act_align[ i ] = 0xff;
                if( scePadSetActAlign( port, slot, act_align ) )
                {
                    s_Phase[port]++;
                }
                break;
                //-----------------------------------------------------------
                case 23:
                if( scePadGetState(port,slot)==scePadStateExecCmd ) 
                    break;
                s_Phase[port] = 99;
                break;
                //-----------------------------------------------------------

                //-----------------------------------------------------------
                // case 99: SETUP IS OVER... READ DATA
                //-----------------------------------------------------------
                default:
                done = 1;
                if( pPad->state==scePadStateStable || pPad->state==scePadStateFindCTP1 )
                {
                    if( UpdateValues(port) != 1 )
                    {
                        s_Phase[port] = 0;
                    }
                }
                break;
            }
        }
    }

}

//=========================================================================

static void input_CheckDevices( void )
{
    // We're gonna keep a copy of the old pad structure as long as we can
    // It may get overwritten if all the queues become used but that's an
    // exceptional circumstance (hopefully).
    if( s_pCurrentPad )
    {
        s_pPreviousPad = s_pCurrentPad;
    }
    //
    // If we can't get a new pad block, let's just steal one from the ready queue
    //
    s_pCurrentPad = (pads *)s_pqPadAvailable->Recv(MQ_NOBLOCK);
    if( !s_pCurrentPad )
    {
        s_pCurrentPad = (pads *)s_pqPadReady->Recv(MQ_NOBLOCK);
        ASSERT(s_pCurrentPad);
    }

    SolveStates();
    UpdateFeedback();
    VERIFY(s_pqPadReady->Send(s_pCurrentPad,MQ_NOBLOCK));
}

//=========================================================================

xbool input_UpdateState( void )
{
    pads *pPads;

    // The updatestate function *may* be called in a background thread while the IOP
    // is being rebooted. So prevent it from being updated.
    if( !s_Initialized )
        return FALSE;

    input_CheckDevices();

    pPads = (pads *)s_pqPadReady->Recv(MQ_NOBLOCK);
    if( !pPads )
        return FALSE;
    s_ReadState = *pPads;
    VERIFY(s_pqPadAvailable->Send(pPads,MQ_NOBLOCK));
    return TRUE;
}

//=========================================================================

xbool input_IsPressed( input_gadget GadgetID, s32 ControllerID )
{   
    if( (GadgetID >= INPUT_KBD__BEGIN) && (GadgetID < INPUT_KBD__END))
    {
        return 0;
    }

    if( (GadgetID >= INPUT_MOUSE__DIGITAL) && (GadgetID < INPUT_MOUSE__ANALOG) )
    {
        return 0;
    }

    if( GadgetID == INPUT_MSG_EXIT )
        return( FALSE );

    ASSERT( (ControllerID>=0) && (ControllerID<2) );

    // Handle buttons
    ASSERT( ((GadgetID>=INPUT_PS2_BTN_L2) && (GadgetID<=INPUT_PS2_BTN_L_LEFT)) || 
            (GadgetID == INPUT_UNDEFINED) );
    s32 Bit = GadgetID - INPUT_PS2_BTN_L2;
    s32 Buttons = s_ReadState.Pad[ControllerID].buttons;
    return (Buttons&(1<<Bit))?(TRUE):(FALSE);
}

//=========================================================================

xbool input_WasPressed( input_gadget GadgetID, s32 ControllerID )
{
    if( (GadgetID >= INPUT_KBD__BEGIN) && (GadgetID < INPUT_KBD__END) )
    {

        return FALSE;
    }

    if( (GadgetID >= INPUT_MOUSE__DIGITAL) && (GadgetID < INPUT_MOUSE__ANALOG)  )
    {
        return FALSE;
    }

    if( GadgetID == INPUT_MSG_EXIT )
        return( FALSE );

    ASSERT( (ControllerID>=0) && (ControllerID<2) );
    ASSERT( (GadgetID>=INPUT_PS2_BTN_L2) && (GadgetID<=INPUT_PS2_BTN_L_LEFT) );
    s32 Bit = GadgetID - INPUT_PS2_BTN_L2;
    s32 Buttons = s_ReadState.Pad[ControllerID].buttons;
    s32 PrevButtons = s_ReadState.Pad[ControllerID].prev_buttons;

    return ((Buttons & (~PrevButtons))&(1<<Bit))?(TRUE):(FALSE);
}

//=========================================================================

f32 input_GetValue( input_gadget GadgetID, s32 ControllerID )
{
    ASSERT( (ControllerID>=0) && (ControllerID<2) );

    pad* pPad = &s_ReadState.Pad[ControllerID];
    s32 B=0;
    switch( GadgetID )
    {
        case INPUT_MOUSE_X_REL:         return 0;
        case INPUT_MOUSE_Y_REL:         return 0;
        case INPUT_MOUSE_WHEEL_REL:     return 0;

        case INPUT_MOUSE_X_ABS:         return 0;
        case INPUT_MOUSE_Y_ABS:         return 0;
        case INPUT_MOUSE_WHEEL_ABS:     return 0;

        case INPUT_PS2_BTN_L2:          B = pPad->Raw[18]; break;
        case INPUT_PS2_BTN_R2:          B = pPad->Raw[19]; break;
        case INPUT_PS2_BTN_L1:          B = pPad->Raw[16]; break;
        case INPUT_PS2_BTN_R1:          B = pPad->Raw[17]; break;
        case INPUT_PS2_BTN_L_UP:        B = pPad->Raw[10]; break;
        case INPUT_PS2_BTN_L_LEFT:      B = pPad->Raw[ 9]; break;
        case INPUT_PS2_BTN_L_RIGHT:     B = pPad->Raw[ 8]; break;
        case INPUT_PS2_BTN_L_DOWN:      B = pPad->Raw[11]; break;
        case INPUT_PS2_BTN_TRIANGLE:    B = pPad->Raw[12]; break;
        case INPUT_PS2_BTN_SQUARE:      B = pPad->Raw[15]; break;
        case INPUT_PS2_BTN_CIRCLE:      B = pPad->Raw[13]; break;
        case INPUT_PS2_BTN_CROSS:       B = pPad->Raw[14]; break;
        case INPUT_PS2_STICK_LEFT_X:    return pPad->LX;
        case INPUT_PS2_STICK_LEFT_Y:    return pPad->LY;
        case INPUT_PS2_STICK_RIGHT_X:   return pPad->RX;
        case INPUT_PS2_STICK_RIGHT_Y:   return pPad->RY;
        default:
            ASSERTS( FALSE, "Unknown gadget!?!" );
            break;
    }

    return ((f32)B*(1/255.0f));
}

//==============================================================================

xbool input_IsPresent( input_gadget GadgetID, s32 ControllerID /* = -1  */)
{
    ASSERT(ControllerID >= -1);
    ASSERT(ControllerID < NUM_PORTS);

    switch( GadgetID )
    {
    case INPUT_PS2_QRY_MOUSE_PRESENT:
        break;

    case INPUT_PS2_QRY_KBD_PRESENT:
        break;

    case INPUT_PS2_QRY_PAD_PRESENT:
        if( ControllerID == -1 )
        {
            s32 i;
            for( i = 0; i < NUM_PORTS; i++ )
            {
                if (s_ReadState.Pad[i].Connected == FALSE)
                    continue;
                break;
            }
            return( i < NUM_PORTS );
        }

        return (s_ReadState.Pad[ControllerID].Connected);
        break;

    case INPUT_PS2_QRY_ANALOG_MODE:           
        if( ControllerID == -1 )
        {
            s32 i;
            for( i = 0; i < NUM_PORTS; i++ )
            {
                if (s_IsAnalog[i] == FALSE)
                    continue;
                break;
            }
            return( i < NUM_PORTS );
        }

        return (s_IsAnalog[ControllerID]);
        break;

    case INPUT_PS2_QRY_MEM_CARD_PRESENT:
        ASSERTS( FALSE, "Not implemented" );
        break;

    default:
        ASSERTS( FALSE, "Invalid input Query" );
        break;
    }

    return FALSE;
}

//=========================================================================

#if 0
static void input_PeriodicUpdate(void)
{
    xtimer time;
    xtimer  current;

    current.Reset();
    current.Start();
    while( 1 )
    {
        time.Reset();
        time.Start();
//        input_CheckDevices();
        x_DelayThread(30);
        time.Stop();
    }
}
#endif

//=========================================================================

void input_EnableFeedback(xbool state, s32 ControllerID)
{
    ASSERT( (ControllerID >=0) && (ControllerID < 2) );
    s_Feedback[ControllerID].Enabled = state;
}

//=========================================================================

void input_Feedback(s32 Count, feedback_envelope *pEnvelope, s32 ControllerID)
{
    ASSERT( (ControllerID>=0) && (ControllerID<2) );

    s_Feedback[ControllerID].pEnvelope = pEnvelope;
    s_Feedback[ControllerID].Count = Count;
    // This will implicitly force a reload of all envelope parameters
    s_Feedback[ControllerID].Duration = 0.0f;
}

//=========================================================================

void input_Feedback( f32 Duration, f32 Intensity, s32 ControllerID = 0)
{
#ifdef dstewart
    return;
#endif

    feedback *pFeedback;

    ASSERT( (ControllerID>=0) && (ControllerID<2) );
    pFeedback = &s_Feedback[ControllerID];

    if( !pFeedback->Enabled || s_SuppressFeedback )
    {
        Duration = 0.0f;
        Intensity = 0.0f;
    }

    if( (pFeedback->State == FS_SWITCH_ON ) || 
         (pFeedback->State == FS_ON ) )
    {
        // if we try to start a new feedback event when a pending event is of greater
        // intensity OR the current effect is of greater intensity, then we ignore
        // the modification.
        if( (pFeedback->Intensity > Intensity ) ||
             ((pFeedback->Count) && (pFeedback->pEnvelope->Intensity > Intensity) )  )
        {
            return;
        }
    }
#if 0
    x_DebugMsg("[%2.4f] New state set %2.2f, %2.2f, (%2.2f,%2.2f) %d\n",
                x_GetTimeSec(),
                Duration,Intensity,
                pFeedback->Duration,
                pFeedback->Intensity,
                pFeedback->State);
#endif
    s_Feedback[ControllerID].StaticEnvelope.Duration = Duration;
    s_Feedback[ControllerID].StaticEnvelope.Intensity = Intensity;
    input_Feedback(1, &s_Feedback[ControllerID].StaticEnvelope, ControllerID);
}

//=========================================================================

void input_SuppressFeedback( xbool Suppress )
{
    s_SuppressFeedback = Suppress;
}

//=========================================================================

#define ACTUATOR_2_THRESHOLD 0.8f

void UpdateFeedback( void )
{
    s32         i;
    feedback*   pFeedback;
    f32         intensity;
    pad*        pPad;
    
    pPad = s_pCurrentPad->Pad;

    pFeedback = s_Feedback;

    for( i=0;i<2;i++ )
    {
        //
        // Check for a state change which happens when we expire the duration count
        //
        if( pFeedback->Duration <= 0.0f )
        {
            pFeedback->Actuator[2] = 0xff;
            pFeedback->Actuator[3] = 0xff;
            pFeedback->Actuator[4] = 0xff;
            pFeedback->Actuator[5] = 0xff;

            if( pFeedback->Count )
            {
                pFeedback->Duration = pFeedback->pEnvelope->Duration;
                pFeedback->Intensity = pFeedback->pEnvelope->Intensity;

                if( pFeedback->pEnvelope->Intensity > 0.05f )
                {
                    pFeedback->State = FS_SWITCH_ON;
                }
                else
                {
                    pFeedback->State = FS_SWITCH_OFF;
                }
                pFeedback->Count--;
                pFeedback->pEnvelope++;
            }
            else
            {
                // Make sure we've stopped the controller actuator
                if( (pFeedback->State == FS_SWITCH_ON) || (pFeedback->State == FS_ON )  )
                {
                    pFeedback->State = FS_SWITCH_OFF;
                }
            }
        }
        else
        {
            pFeedback->Duration -= 0.030f;
        }

        switch (pFeedback->State)
        {
        //---------------------------------------------------------------------
        case FS_OFF:
            break;
        //---------------------------------------------------------------------
        case FS_SWITCH_OFF:
            pFeedback->Actuator[0] = 0;
            pFeedback->Actuator[1] = 0;
            if( pPad->state==scePadStateStable || pPad->state==scePadStateFindCTP1 )
            {
                if( scePadSetActDirect(i,0,pFeedback->Actuator) )
                {
                    pFeedback->State = FS_OFF;
                }
            }
            break;
        //---------------------------------------------------------------------
        case FS_SWITCH_ON:
            intensity = pFeedback->Intensity;
            // If we have an intensity of > 0.5, then we switch on the 2nd actuator
            // and reduce the intensity of the first

            if( intensity < 0.0f )
                intensity = 0.0f;
            if( intensity > 1.0f )
                intensity = 1.0f;

            if( pFeedback->Intensity > ACTUATOR_2_THRESHOLD )
            {
                pFeedback->Actuator[0] = 1;
                intensity = (intensity - (1.0f - ACTUATOR_2_THRESHOLD));
            }
            else
            {
                pFeedback->Actuator[0] = 0;
            }
            intensity *= 1.0f/ACTUATOR_2_THRESHOLD;

            pFeedback->Actuator[1] = (u8)(intensity * 255.0f);

//          x_DebugMsg( "Intensity: %2.2f, actuator 0: %d, actuator 1: %d\n",
//                      pFeedback->Intensity,
//                      pFeedback->Actuator[0],
//                      pFeedback->Actuator[1] );

            if( pPad->state==scePadStateStable || pPad->state==scePadStateFindCTP1 )
            {
                if( scePadSetActDirect(i,0,pFeedback->Actuator) )
                {
                    pFeedback->State = FS_ON;
                }
            }
            break;
        //---------------------------------------------------------------------
        case FS_ON:
            break;
        }

        pFeedback++;
        pPad++;
    }
}

//=========================================================================

void input_ClearFeedback( void )
{
    //SH: We do not mess with the 'enabled' flags
    //    The point here is just to clear the rumble data, not to
    //    change the state of the input system.

    s_Feedback[0].State     = FS_OFF;
    s_Feedback[0].Duration  = 0.0f;
    s_Feedback[0].Intensity = 0.0f;
    //s_Feedback[0].Enabled   = TRUE;
    s_Feedback[1].State     = FS_OFF;
    s_Feedback[1].Duration  = 0.0f;
    s_Feedback[1].Intensity = 0.0f;
    //s_Feedback[1].Enabled   = TRUE;
}

//=========================================================================