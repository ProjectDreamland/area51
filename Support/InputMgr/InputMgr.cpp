//==============================================================================
//  InputMgr.cpp
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "InputMgr.hpp"
#include "Monkey.hpp"
#ifdef CONFIG_VIEWER
#include "../../Apps/ArtistViewer/Config.hpp"
#else
#include "../../Apps/GameApp/Config.hpp"	
#endif
#include "StateMgr\StateMgr.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

input_mgr    g_InputMgr;
input_pad*   input_mgr::s_pHead = NULL;

#ifndef X_RETAIL
xbool        g_InputText        = FALSE;
s32          g_InputTextLine;
#endif

extern xbool SuppressFeedback;

static const s32 FIRST_INPUT_TEXT_LINE  = 10;
static const s32 INPUT_TEXT_COLUMN      = 10;

#if CONFIG_IS_DEMO && !defined( X_EDITOR )
extern xtimer g_DemoIdleTimer;
#endif
//==============================================================================
//  FUNCTIONS
//==============================================================================

input_pad::input_pad( void )
{
    for( s32 i = 0; i < MAX_INPUT_PLATFORMS; i++ )
    {
        m_nMaps[i] = 0;        
        x_memset( m_Map[i], 0, sizeof(map)*MAX_MAPPINGS );
    }

    x_memset( m_Logical, 0, sizeof(logical)*MAX_LOGICAL );
    m_ControllerID   = -1;
    m_ActiveContext  =  0;
}

//==============================================================================

void input_pad::ClearMapping( void )
{
    for( s32 i = 0; i < MAX_INPUT_PLATFORMS; i++ )
    {
        x_memset( m_Map[i], 0, sizeof(map)*MAX_MAPPINGS );
        m_nMaps[i] = 0;
    }
}

//==============================================================================

void input_pad::SetLogical( s32 ID, const char* pName )
{
    (void)pName;

#ifdef TARGET_PC
    x_strncpy( m_Logical[ ID ].ActionName, pName, 48 );
#endif
    m_Logical[ ID ].IsValue  = 0;
    m_Logical[ ID ].WasValue = 0;
}

//==============================================================================
// This method is for those rare instances where a specific button is used, 
// and we need to clear the logical button associated with it. 
// Example - the ScreenShot button enabled from the debug menu.
#if !defined(X_RETAIL) && !defined(TARGET_PC)
void input_pad::SetLogical( s32 iPlatform, input_gadget GadgetID )
{
    ASSERT( iPlatform <= MAX_INPUT_PLATFORMS );

    input_gadget ID = INPUT_UNDEFINED;
    xbool IsButton = FALSE;
    f32 Scale = 0.0f;

    // find the logical button
    for( s32 LogID = 0; LogID < MAX_LOGICAL; LogID++ )
    {
        GetMapping( iPlatform, LogID, ID, IsButton, Scale );
        if( ID == GadgetID )
        {
            // found it. this uses our gadget.
            m_Logical[ LogID ].IsValue  = 0;
            m_Logical[ LogID ].WasValue = 0;
            break;
        }
    }
}
#endif
//==============================================================================

void input_pad::ClearAllLogical( void )
{
    for( s32 i=0; i<MAX_LOGICAL; i++ )
    {
        m_Logical[ i ].IsValue  = 0;
        m_Logical[ i ].WasValue = 0;
    }
}

//==============================================================================

void input_pad::AddMapping( s32             iPlatform, 
                            s32             ID, 
                            input_gadget    GadgetID, 
                            xbool           IsButton, 
                            f32             Scale )
{
    ASSERT( m_nMaps[iPlatform] <= MAX_MAPPINGS );
    ASSERT( ID                 <= MAX_LOGICAL  );

    m_Map[iPlatform][ m_nMaps[iPlatform] ].bButton         = IsButton;
    m_Map[iPlatform][ m_nMaps[iPlatform] ].GadgetID        = GadgetID;
    m_Map[iPlatform][ m_nMaps[iPlatform] ].Scale           = Scale;
    m_Map[iPlatform][ m_nMaps[iPlatform] ].iLogicalMapping = ID;

    m_nMaps[iPlatform]++;
}

//==============================================================================

void input_pad::GetMapping( s32             iPlatform, 
                            s32             ID, 
                            input_gadget&   GadgetID, 
                            xbool&          IsButton, 
                            f32&            Scale )
{
    for( s32 i = 0; i < m_nMaps[iPlatform]; i++ )
    {
        if( m_Map[iPlatform][ i ].iLogicalMapping == ID )
        {
            IsButton = m_Map[iPlatform][ i ].bButton;
            GadgetID = m_Map[iPlatform][ i ].GadgetID;
            Scale    = m_Map[iPlatform][ i ].Scale;
            break;
        }
    }
}

//==============================================================================

void input_pad::DelMapping( s32 iPlatform, s32 iMapping )
{
    ASSERT( iMapping <= MAX_MAPPINGS );
    ASSERT( iMapping <= m_nMaps[iPlatform] );

    for( s32 i=iMapping; i<m_nMaps[iPlatform]; i++ )
    {
        m_Map[iPlatform][ i ] = m_Map[iPlatform][ i+1 ];
    }

    m_nMaps[iPlatform]--;
}

//==============================================================================

void input_pad::EnableContext( u32 Context )
{
    m_ActiveContext |= Context;
}

//==============================================================================

void input_pad::DisableContext( u32 Context )
{
    m_ActiveContext = (m_ActiveContext & ~Context);
}

//==============================================================================

void input_pad::OnUpdate( s32 iPlatform, f32 DeltaTime )
{
    (void)DeltaTime;
    
    if( m_ControllerID == -1 )
        return;

    s32 ControllerID = m_ControllerID;

    //----------------------------------------------    
    if ( g_MonkeyOptions.Enabled )
    {
        g_Monkey.Update(DeltaTime);
        input_SuppressFeedback( g_MonkeyOptions.Enabled );
    }
    //----------------------------------------------

    s32 i;
    for( i=0; i<m_nMaps[iPlatform]; i++ )
    {
        map&        Map = m_Map[iPlatform][ i ];
        logical&    Log = m_Logical[ Map.iLogicalMapping ];
        
        //if( m_ActiveContext & Map.MapContext )
        {
            if( Map.bButton )
            {
                //
                // If the user wants to treat some of the analog buttons of the PS2 
                // as digitals, we will handle it for him.
                //

                f32 IsPress = (f32)input_IsPressed( Map.GadgetID, ControllerID );

#if CONFIG_IS_DEMO && !defined( X_EDITOR )
                if( IsPress )
                {
                    g_DemoIdleTimer.Trip();
                }
#endif
                //----------------------------------------------
                if( g_MonkeyOptions.Enabled )
                    IsPress = g_Monkey.GetValue( Map.iLogicalMapping );                    
                //----------------------------------------------

                if ( Map.bIsTap || Map.bIsHold )
                {
                    //
                    // Handle tap and hold by setting up IsPress accordingly
                    //
                    xbool ClearTime = !IsPress;

                    if ( IsPress )
                    {
                        Log.TimePressed += DeltaTime;
                    }

                    static f32 s_TapTime = 0.3f;
                    if ( Map.bIsTap )
                    {
                        // if the button was just released, and we're under the 
                        // time threshold, then we just tapped
                        if (   !IsPress 
                            && (Log.TimePressed > 0.0f) 
                            && (Log.TimePressed < s_TapTime ) )
                        {
                            IsPress = 1.0f; // have to force a value since the button isn't actually pressed
                        }
                        else if ( IsPress )
                        {
                            // We haven't let go of the button yet, no tap here
                            IsPress = 0.0f;
                        }
                    }
                    else
                    {
                        ASSERT( Map.bIsHold );

                        // if the button is pressed, and we're over the time
                        // threshold, then we are officially holding, so leave IsPress alone
                        // If not, then cancel the IsPress
                        if ( IsPress && (Log.TimePressed < s_TapTime) )
                        {
                            IsPress = 0.0f; // not holding
                        }
                        else if ( !IsPress )
                        {
                            // We aren't holding the button down any more, clear the timer
                            ClearTime = TRUE;
                        }
                    }

                    if ( ClearTime )
                    {
                        Log.TimePressed = 0.0f;
                    }

                }

                if( IsPress != Log.IsValue )
                {
                    if( IsPress > Log.MapsWasValue )
                        Log.MapsWasValue = IsPress;
                }

                if( IsPress > Log.MapsIsValue )
                    Log.MapsIsValue = IsPress;
            }
            else
            {
                f32 Value = 0.0f;
                
                if( Map.GadgetID != INPUT_UNDEFINED )
                    Value = input_GetValue( Map.GadgetID, ControllerID );

                //----------------------------------------------
                if( g_MonkeyOptions.Enabled )
                    Value = g_Monkey.GetValue( Map.iLogicalMapping );                    
                //----------------------------------------------
                Log.MapsWasValue = Log.IsValue;
                Log.MapsIsValue  = Map.Scale * Value;
#if CONFIG_IS_DEMO && !defined( X_EDITOR )
                if( x_abs( Log.IsValue ) > 0.2f )
                {
                    g_DemoIdleTimer.Trip();
                }
#endif
            }
        }

#if TARGET_PC && !defined( X_RETAIL )
        if ( Log.IsValue && g_InputText )
        {
            x_printfxy( INPUT_TEXT_COLUMN, g_InputTextLine++, 
                        xfs( "%i:%s", ControllerID, Log.ActionName ) );
        }
#endif
    }

    // Store the logical values.
    for( i=0; i < MAX_LOGICAL; i++ )
    {
        m_Logical[i].IsValue        = m_Logical[i].MapsIsValue;
        m_Logical[i].WasValue       = m_Logical[i].MapsWasValue;
        
        m_Logical[i].MapsIsValue    = 0.0f;
        m_Logical[i].MapsWasValue   = 0.0f;
    }
}

//==============================================================================

void input_pad::OnInitialize( void )
{
}

//==============================================================================

void input_pad::OnEnumProp( prop_enum& List )
{
    for( s32 iPlatform = 0; iPlatform < MAX_INPUT_PLATFORMS; iPlatform++ )
    {
        s32 iHeader = 0;

        switch( iPlatform )
        {
            default:
                ASSERT( FALSE );
                break;
#if defined(TARGET_XBOX)
            case INPUT_PLATFORM_XBOX:
                List.PropEnumHeader( "XBOX", "", 0 );
                iHeader = List.PushPath( "XBOX\\" );
                break;
#elif defined(TARGET_PS2)
            case INPUT_PLATFORM_PS2:
                List.PropEnumHeader( "PS2", "", 0 );
                iHeader = List.PushPath( "PS2\\" );
                break;
#elif defined(TARGET_PC)
            case INPUT_PLATFORM_XBOX:
                List.PropEnumHeader( "XBOX", "", 0 );
                iHeader = List.PushPath( "XBOX\\" );
                break;
            case INPUT_PLATFORM_PS2:
                List.PropEnumHeader( "PS2", "", 0 );
                iHeader = List.PushPath( "PS2\\" );
                break;    
            case INPUT_PLATFORM_PC:
                List.PropEnumHeader( "", "", 0 );
                iHeader = List.PushPath( "" );
                break;    
#endif
        }

        List.PropEnumHeader( "InputPad", "This is a manager used to attack physical devices to logical actions", PROP_TYPE_HEADER );

        List.PropEnumInt   ( "InputPad\\NMaps", "This is for loading and saving this should not be expouse to users", PROP_TYPE_DONT_SHOW );

        for( s32 i=0; i<MAX_LOGICAL; i++ )
        {
#ifdef TARGET_PC
            if( m_Logical[i].ActionName[0] == 0 )
                continue;
#endif

            List.PropEnumString( xfs("InputPad\\LogicalMap[%d]",i), "A logical map is like an specific command for the player such Jump. Where multiple keys could be maped to it in order to activate that command", PROP_TYPE_HEADER );
            List.PropEnumButton( xfs("InputPad\\LogicalMap[%d]\\AddMapping",i), "Creates a new mapping", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
            
            for( s32 j=0; j<m_nMaps[iPlatform]; j++ )
            {
                if( m_Map[iPlatform][j].iLogicalMapping == i )
                {
                    List.PropEnumString ( xfs("InputPad\\LogicalMap[%d]\\Map[%d]", i,j ), "A map is use to attach a physical device 'key' to a logical action", PROP_TYPE_HEADER );
                    List.PropEnumInt    ( xfs("InputPad\\LogicalMap[%d]\\Map[%d]\\iLogicalMapping",i,j),    "This is use for save and load.", PROP_TYPE_DONT_SHOW );
                    List.PropEnumButton ( xfs("InputPad\\LogicalMap[%d]\\Map[%d]\\DelMapping",i,j),    "Deletes this mapping", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
                    List.PropEnumEnum   ( xfs("InputPad\\LogicalMap[%d]\\Map[%d]\\GadgetID", i,j ), GetGadgetIDNames( iPlatform ),
                                         "This is the physical device which the logical action is map to", 0 );

                    List.PropEnumBool   ( xfs("InputPad\\LogicalMap[%d]\\Map[%d]\\IsButton",i,j), "Tells whether the gadget in question is setup as a button", PROP_TYPE_MUST_ENUM );
                    List.PropEnumBool   ( xfs("InputPad\\LogicalMap[%d]\\Map[%d]\\IsTap",i,j), "Tells whether the gadget in question is setup as a button tap", PROP_TYPE_MUST_ENUM );
                    List.PropEnumBool   ( xfs("InputPad\\LogicalMap[%d]\\Map[%d]\\IsHold",i,j), "Tells whether the gadget in question is setup as a button hold", PROP_TYPE_MUST_ENUM );
                    List.PropEnumFloat  ( xfs("InputPad\\LogicalMap[%d]\\Map[%d]\\Scale",i,j),    "Tells Whether it needs any scale for the input. Usually is just (1 or -1).", 0 );
                
                    //List.PropEnumButton ( xfs("InputPad\\LogicalMap[%d]\\Map[%d]\\AddContext",i,j),    "Add a input context", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
            
                    List.PropEnumBool ( xfs("InputPad\\LogicalMap[%d]\\Map[%d]\\InGame Context",i,j),        "This map has In game context", 0 );
                    List.PropEnumBool ( xfs("InputPad\\LogicalMap[%d]\\Map[%d]\\Inventory Context",i,j),     "This map has inventory context", 0 );
                    List.PropEnumBool ( xfs("InputPad\\LogicalMap[%d]\\Map[%d]\\Pause Context",i,j),         "This map has pause context", 0 );
                    List.PropEnumBool ( xfs("InputPad\\LogicalMap[%d]\\Map[%d]\\FrontEnd Context",i,j),      "This map has front end context", 0 );

                    // TO_ADD_CONTEXT
                    List.PropEnumBool ( xfs( "InputPad\\LogicalMap[%d]\\Map[%d]\\Ladder Context",i,j),       "This map has ladder context", 0 );

                }
            }
        }

        List.PopPath( iHeader );
    }
}

//==============================================================================

xbool input_pad::OnProperty( prop_query& I )
{
#ifdef TARGET_PC
    xbool Boolean;
#endif

    for( s32 i = 0; i < MAX_INPUT_PLATFORMS; i++ )
    {
        s32 iHeader = 0;
        s32 iIndex0 = I.GetIndex(0);
        s32 iIndex1 = I.GetIndex(1);

#ifdef TARGET_XBOX
#endif
            
        switch( i )
        {
            default:
                ASSERT( FALSE );
                break;
#if defined(TARGET_XBOX)
            case INPUT_PLATFORM_XBOX:
                iHeader = I.PushPath( "XBOX\\" );
                break;
#elif defined(TARGET_PS2)
            case INPUT_PLATFORM_PS2:
                iHeader = I.PushPath( "PS2\\" );
                break;
#elif defined(TARGET_PC)
            case INPUT_PLATFORM_XBOX:
                iHeader = I.PushPath( "XBOX\\" );
                break;
            case INPUT_PLATFORM_PS2:
                iHeader = I.PushPath( "PS2\\" );
                break;
            case INPUT_PLATFORM_PC:
                iHeader = I.PushPath( "" );
                break;
#endif
        }

        if( I.VarInt( "InputPad\\NMaps", m_nMaps[i] ) )
        {
            return TRUE;
        }

#ifdef TARGET_PC
        if( I.VarString( "InputPad\\LogicalMap[]", m_Logical[iIndex0].ActionName, 48 ) )
        {
            return TRUE;
        }
#endif

        if( I.IsVar( "InputPad\\LogicalMap[]\\AddMapping" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarButton( "AddMapping" );
            }
            else
            {
#if defined(TARGET_XBOX)
                AddMapping( i, iIndex0, INPUT_XBOX_STICK_LEFT_X, FALSE );
#elif defined(TARGET_PS2)
                AddMapping( i, iIndex0, INPUT_PS2_STICK_LEFT_X, FALSE );
#elif defined(TARGET_PC)
                if( i == INPUT_PLATFORM_PS2 )
                {
                    AddMapping( i, iIndex0, INPUT_PS2_STICK_LEFT_X, FALSE );
                }
                else if( i == INPUT_PLATFORM_XBOX ) 
                {
                    AddMapping( i, iIndex0, INPUT_XBOX_STICK_LEFT_X, FALSE );
                }
                else
                {
                }
#endif
            }        

            return TRUE;
        }
    
        if( I.IsVar( "InputPad\\LogicalMap[]\\Map[]\\DelMapping") )
        {
            if( I.IsRead() )
            {
                I.SetVarButton( "Delete" );
            }
            else
            {
                DelMapping( i, iIndex1 );
            }

            return TRUE;
        }    
    
        if( I.VarInt( "InputPad\\LogicalMap[]\\Map[]\\iLogicalMapping", m_Map[i][iIndex1].iLogicalMapping ) )
        {
            return TRUE;
        }    
    
        if( I.VarString( "InputPad\\LogicalMap[]\\Map[]", (char*)GetNameFromGadgetID( i, m_Map[i][iIndex1].GadgetID ), 256 ) )
        {
            // You can only read this guy
            ASSERT( I.IsRead() == TRUE );

            return TRUE;
        }
    
        if( I.IsVar( "InputPad\\LogicalMap[]\\Map[]\\GadgetID" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarEnum( GetNameFromGadgetID( i, m_Map[i][iIndex1].GadgetID ) );
            }
            else
            {
                {
                    {
                        m_Map[i][iIndex1].GadgetID = GetGadgetIDFromName( i, I.GetVarEnum() );
                        return TRUE;
                    }
                }
            }

            return TRUE;
        }

        {
            xbool bButton = m_Map[i][iIndex1].bButton;
            xbool Result = I.VarBool( "InputPad\\LogicalMap[]\\Map[]\\IsButton", bButton );
            m_Map[i][iIndex1].bButton = bButton;
            if( Result )
            {
                // ensure that related properties are valid
                if ( !m_Map[i][iIndex1].bButton )
                {
                    m_Map[i][iIndex1].bIsTap      = FALSE;
                    m_Map[i][iIndex1].bIsHold     = FALSE;
                }
                return TRUE;
            }
        }

        {
            xbool bIsTap = m_Map[i][iIndex1].bIsTap;
            xbool Result = I.VarBool( "InputPad\\LogicalMap[]\\Map[]\\IsTap", bIsTap );
            m_Map[i][iIndex1].bIsTap = bIsTap;
            if( Result )
            {
                // ensure that related properties are valid
                if ( m_Map[i][iIndex1].bIsTap )
                {
                    m_Map[i][iIndex1].bIsHold     = FALSE;
                    m_Map[i][iIndex1].bButton   = TRUE;
                }
                return TRUE;
            }
        }

        {
            xbool bIsHold = m_Map[i][iIndex1].bIsHold;
            xbool Result = I.VarBool( "InputPad\\LogicalMap[]\\Map[]\\IsHold", bIsHold );
            m_Map[i][iIndex1].bIsHold = bIsHold;
            if( Result )
            {
                // ensure that related properties are valid
                if ( m_Map[i][iIndex1].bIsHold )
                {
                    m_Map[i][iIndex1].bIsTap      = FALSE;
                    m_Map[i][iIndex1].bButton   = TRUE;
                }
                return TRUE;
            }
        }
    
        if( I.VarFloat( "InputPad\\LogicalMap[]\\Map[]\\Scale", m_Map[i][iIndex1].Scale ) )
        {
            return TRUE;
        }

#ifdef TARGET_PC
        if( I.IsVar( "InputPad\\LogicalMap[]\\Map[]\\InGame Context" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarBool( (m_Map[i][iIndex1].MapContext & INGAME_CONTEXT) );
            }
            else
            {
                if( I.GetVarBool() )
                    m_Map[i][iIndex1].MapContext |= INGAME_CONTEXT;
                else
                    m_Map[i][iIndex1].MapContext = (m_Map[i][iIndex1].MapContext & ~INGAME_CONTEXT);
            }

            return TRUE;
        }

        if( I.VarBool( "InputPad\\LogicalMap[]\\Map[]\\Inventory Context", Boolean ) )
        {
            if( I.IsRead() )
            {
                I.SetVarBool( (m_Map[i][iIndex1].MapContext & INVENTORY_CONTEXT) );
            }
            else
            {
                if( I.GetVarBool() )
                    m_Map[i][iIndex1].MapContext |= INVENTORY_CONTEXT;
                else
                    m_Map[i][iIndex1].MapContext = (m_Map[i][iIndex1].MapContext & ~INVENTORY_CONTEXT);
            }

            return TRUE;
        }

        if( I.VarBool( "InputPad\\LogicalMap[]\\Map[]\\Pause Context", Boolean ) )
        {
            if( I.IsRead() )
            {
                I.SetVarBool( (m_Map[i][iIndex1].MapContext & PAUSE_CONTEXT) );
            }
            else
            {
                if( I.GetVarBool() )
                    m_Map[i][iIndex1].MapContext |= PAUSE_CONTEXT;
                else
                    m_Map[i][iIndex1].MapContext = (m_Map[i][iIndex1].MapContext & ~PAUSE_CONTEXT);
            }

            return TRUE;
        }

        if( I.VarBool( "InputPad\\LogicalMap[]\\Map[]\\FrontEnd Context", Boolean ) )
        {
            if( I.IsRead() )
            {
                I.SetVarBool( (m_Map[i][iIndex1].MapContext & FRONTEND_CONTEXT) );
            }
            else
            {
                if( I.GetVarBool() )
                    m_Map[i][iIndex1].MapContext |= FRONTEND_CONTEXT;
                else
                    m_Map[i][iIndex1].MapContext = (m_Map[i][iIndex1].MapContext & ~FRONTEND_CONTEXT);
            }

            return TRUE;
        }

        // TO_ADD_CONTEXT
        if( I.VarBool( "InputPad\\LogicalMap[]\\Map[]\\Ladder Context", Boolean ) )
        {
            if( I.IsRead() )
            {
                I.SetVarBool( (m_Map[i][iIndex1].MapContext & LADDER_CONTEXT) );
            }
            else
            {
                if( I.GetVarBool() )
                    m_Map[i][iIndex1].MapContext |= LADDER_CONTEXT;
                else
                    m_Map[i][iIndex1].MapContext = (m_Map[i][iIndex1].MapContext & ~LADDER_CONTEXT);
            }

            return TRUE;
        }
#endif // TARGET_PC

        I.PopPath( iHeader );
    }

    return FALSE;
}


//==============================================================================

#ifndef X_RETAIL
void input_pad::OnDebugRender( void )
{
#ifdef TARGET_PC
    for( s32 i=0; i<MAX_LOGICAL; i++ )
    {
        if( m_Logical[i].ActionName[0] == 0 )
            continue;

        x_printfxy( 30, i, "%d %s Is(%f) Was(%f)\n", 
                        i, m_Logical[i].ActionName, 
                           m_Logical[i].IsValue, 
                           m_Logical[i].WasValue );
    }
#endif
}
#endif // X_RETAIL

//==============================================================================

void input_pad::SetAllLogical( logical* pLogical )
{
    ASSERT( pLogical );
    s32 i;

    for ( i = 0; i < MAX_LOGICAL; ++i )
    {
        m_Logical[i] = pLogical[i];
    }
}

//==============================================================================

void input_pad::SetAllMap( s32 iPlatform, map* pMap, s32 nMaps )
{
    ASSERT( pMap );
    s32 i;
    for ( i = 0; i < nMaps; ++i )
    {
        m_Map[iPlatform][i] = pMap[i];
    }
    m_nMaps[iPlatform] = nMaps;
}

//==============================================================================

const char* input_pad::GetGadgetIDNames( s32 iPlatform )
{
    if( iPlatform == INPUT_PLATFORM_PS2 )
    {
        return  "PS2_L2\0"
                "PS2_R2\0"
                "PS2_L1\0"
                "PS2_R1\0"
                "PS2_TRIANGLE\0"
                "PS2_CIRCLE\0"
                "PS2_CROSS\0"
                "PS2_SQUARE\0"
                "PS2_SELECT\0"
                "PS2_L_STICK\0"
                "PS2_R_STICK\0"
                "PS2_START\0"
                "PS2_KEY_UP\0"
                "PS2_KEY_RIGHT\0"
                "PS2_KEY_DOWN\0"
                "PS2_KEY_LEFT\0"
                "PS2_LEFT_STICK_X\0"
                "PS2_LEFT_STICK_Y\0"
                "PS2_RIGHT_STICK_X\0"
                "PS2_RIGHT_STICK_Y\0";
    }
    else if (iPlatform == INPUT_PLATFORM_XBOX)
    {
        return  "XBOX_START\0"
                "XBOX_BACK\0"
                "XBOX_LEFT\0"
                "XBOX_RIGHT\0"
                "XBOX_UP\0"
                "XBOX_DOWN\0"
                "XBOX_BTN_L_STICK\0"
                "XBOX_BTN_R_STICK\0"
                "XBOX_WHITE\0"
                "XBOX_BLACK\0"
                "XBOX_A\0"
                "XBOX_B\0"
                "XBOX_X\0"
                "XBOX_Y\0"
                "XBOX_L_TRIGGER\0"
                "XBOX_R_TRIGGER\0"
                "XBOX_STICK_LEFT_X\0"
                "XBOX_STICK_LEFT_Y\0"
                "XBOX_STICK_RIGHT_X\0"
                "XBOX_STICK_RIGHT_Y\0";

    }
    else if (iPlatform == INPUT_PLATFORM_PC)
    {
        return  "MOUSE_BTN_L\0"
                "MOUSE_BTN_C\0"
                "MOUSE_BTN_R\0"
                "ESCAPE\0"
                "MINUS\0"
                "EQUALS\0"
                "LBRACKET\0"
                "RBRACKET\0"
                "RETURN\0"
                "LCONTROL\0"
                "SEMICOLON\0"
                "APOSTROPHE\0"
                "GRAVE\0"
                "BACKSLASH\0"
                "COMMA\0"
                "PERIOD\0"
                "SLASH\0"
                "RSHIFT\0"
                "MULTIPLY\0"
                "SPACE\0"
                "CAPITAL\0"
                "NUMLOCK\0"
                "SCROLL\0"
                "SUBTRACT\0"
                "NUMPAD1\0"
                "DECIMAL\0"
                "ABNT_C1\0"
                "CONVERT\0"
                "NOCONVERT\0"
                "NUMPADEQUALS\0"
                "PREVTRACK\0"
                "COLON\0"
                "UNDERLINE\0"
                "UNLABELED\0"
                "NEXTTRACK\0"
                "NUMPADENTER\0"
                "RCONTROL\0"
                "CALCULATOR\0"
                "PLAYPAUSE\0"
                "MEDIASTOP\0"
                "VOLUMEDOWN\0"
                "VOLUMEUP\0"
                "NUMPADCOMMA\0"
                "DIVIDE\0"
                "PAUSE\0"
                "PRIOR\0"
                "RIGHT\0"
                "INSERT\0"
                "DELETE\0"
                "POWER\0"
                "WEBSEARCH\0"
                "WEBFAVORITES\0"
                "WEBREFRESH\0"
                "WEBSTOP\0"
                "WEBFORWARD\0"
                "WEBBACK\0"
                "MYCOMPUTER\0"
                "MEDIASELECT\0";
    }
    x_throw ("Alert! Unexpected platfrom!!!" );
    //return "";
}

//==============================================================================
// Table to map input_gadget to string.

struct gadget_name_map
{
    input_gadget GadgetID;
    const char*  pName;
};

static const s32 ACTUAL_GADGET_COUNT[3] = { 20, 20, 64 }; // PS2, XBOX, PC

#define MAX_GADGET_MAPS 64

static gadget_name_map GadgetNameMap[3][MAX_GADGET_MAPS] =
{
    {
        {  INPUT_PS2_BTN_L2          ,   "PS2_L2"               },        
        {  INPUT_PS2_BTN_R2          ,   "PS2_R2"               },
        {  INPUT_PS2_BTN_L1          ,   "PS2_L1"               },
        {  INPUT_PS2_BTN_R1          ,   "PS2_R1"               },
        {  INPUT_PS2_BTN_TRIANGLE    ,   "PS2_TRIANGLE"         },
        {  INPUT_PS2_BTN_CIRCLE      ,   "PS2_CIRCLE"           },
        {  INPUT_PS2_BTN_CROSS       ,   "PS2_CROSS"            },
        {  INPUT_PS2_BTN_SQUARE      ,   "PS2_SQUARE"           },
        {  INPUT_PS2_BTN_SELECT      ,   "PS2_SELECT"           },
        {  INPUT_PS2_BTN_L_STICK     ,   "PS2_L_STICK"          },
        {  INPUT_PS2_BTN_R_STICK     ,   "PS2_R_STICK"          },
        {  INPUT_PS2_BTN_START       ,   "PS2_START"            },
        {  INPUT_PS2_BTN_L_UP        ,   "PS2_KEY_UP"           },
        {  INPUT_PS2_BTN_L_RIGHT     ,   "PS2_KEY_RIGHT"        },
        {  INPUT_PS2_BTN_L_DOWN      ,   "PS2_KEY_DOWN"         },
        {  INPUT_PS2_BTN_L_LEFT      ,   "PS2_KEY_LEFT"         },
        {  INPUT_PS2_STICK_LEFT_X    ,   "PS2_LEFT_STICK_X"     },
        {  INPUT_PS2_STICK_LEFT_Y    ,   "PS2_LEFT_STICK_Y"     },
        {  INPUT_PS2_STICK_RIGHT_X   ,   "PS2_RIGHT_STICK_X"    },
        {  INPUT_PS2_STICK_RIGHT_Y   ,   "PS2_RIGHT_STICK_Y"    },
    },
    {
        {  INPUT_XBOX_BTN_START      ,   "XBOX_START"           },
        {  INPUT_XBOX_BTN_BACK       ,   "XBOX_BACK"            },
        {  INPUT_XBOX_BTN_LEFT       ,   "XBOX_LEFT"            },
        {  INPUT_XBOX_BTN_RIGHT      ,   "XBOX_RIGHT"           },
        {  INPUT_XBOX_BTN_UP         ,   "XBOX_UP"              },
        {  INPUT_XBOX_BTN_DOWN       ,   "XBOX_DOWN"            },
        {  INPUT_XBOX_BTN_L_STICK    ,   "XBOX_BTN_L_STICK"     },
        {  INPUT_XBOX_BTN_R_STICK    ,   "XBOX_BTN_R_STICK"     },
        {  INPUT_XBOX_BTN_WHITE      ,   "XBOX_WHITE"           },
        {  INPUT_XBOX_BTN_BLACK      ,   "XBOX_BLACK"           },
        {  INPUT_XBOX_BTN_A          ,   "XBOX_A"               },
        {  INPUT_XBOX_BTN_B          ,   "XBOX_B"               },
        {  INPUT_XBOX_BTN_X          ,   "XBOX_X"               },
        {  INPUT_XBOX_BTN_Y          ,   "XBOX_Y"               },
        {  INPUT_XBOX_L_TRIGGER      ,   "XBOX_L_TRIGGER"       },
        {  INPUT_XBOX_R_TRIGGER      ,   "XBOX_R_TRIGGER"       },
        {  INPUT_XBOX_STICK_LEFT_X   ,   "XBOX_STICK_LEFT_X"    },
        {  INPUT_XBOX_STICK_LEFT_Y   ,   "XBOX_STICK_LEFT_Y"    },
        {  INPUT_XBOX_STICK_RIGHT_X  ,   "XBOX_STICK_RIGHT_X"   },
        {  INPUT_XBOX_STICK_RIGHT_Y  ,   "XBOX_STICK_RIGHT_Y"   },
    },
    {
        { INPUT_MOUSE_BTN_L          , "MOUSE_BTN_L"            },
        { INPUT_MOUSE_BTN_C          , "MOUSE_BTN_C"            },
        { INPUT_MOUSE_BTN_R          , "MOUSE_BTN_R"            },
        { INPUT_KBD_ESCAPE           , "ESCAPE"                 },
        { INPUT_KBD_MINUS            , "MINUS"                  },
        { INPUT_KBD_EQUALS           , "EQUALS"                 },
        { INPUT_KBD_LBRACKET         , "LBRACKET"               },
        { INPUT_KBD_RBRACKET         , "RBRACKET"               },
        { INPUT_KBD_RETURN           , "RETURN"                 },
        { INPUT_KBD_LCONTROL         , "LCONTROL"               },
        { INPUT_KBD_SEMICOLON        , "SEMICOLON"              },
        { INPUT_KBD_APOSTROPHE       , "APOSTROPHE"             },
        { INPUT_KBD_GRAVE            , "GRAVE"                  },
        { INPUT_KBD_BACKSLASH        , "BACKSLASH"              },
        { INPUT_KBD_COMMA            , "COMMA"                  },
        { INPUT_KBD_PERIOD           , "PERIOD"                 },
        { INPUT_KBD_SLASH            , "SLASH"                  },
        { INPUT_KBD_RSHIFT           , "RSHIFT"                 },
        { INPUT_KBD_MULTIPLY         , "MULTIPLY"               },
        { INPUT_KBD_SPACE            , "SPACE"                  },
        { INPUT_KBD_CAPITAL          , "CAPITAL"                },
        { INPUT_KBD_NUMLOCK          , "NUMLOCK"                },
        { INPUT_KBD_SCROLL           , "SCROLL"                 },
        { INPUT_KBD_SUBTRACT         , "SUBTRACT"               },
        { INPUT_KBD_NUMPAD1          , "NUMPAD1"                },
        { INPUT_KBD_DECIMAL          , "DECIMAL"                },
        { INPUT_KBD_ABNT_C1          , "ABNT_C1"                },
        { INPUT_KBD_CONVERT          , "CONVERT"                },
        { INPUT_KBD_NOCONVERT        , "NOCONVERT"              },
        { INPUT_KBD_NUMPADEQUALS     , "NUMPADEQUALS"           },
        { INPUT_KBD_PREVTRACK        , "PREVTRACK"              },
        { INPUT_KBD_COLON            , "COLON"                  },
        { INPUT_KBD_UNDERLINE        , "UNDERLINE"              },
        { INPUT_KBD_UNLABELED        , "UNLABELED"              },
        { INPUT_KBD_NEXTTRACK        , "NEXTTRACK"              },
        { INPUT_KBD_NUMPADENTER      , "NUMPADENTER"            },
        { INPUT_KBD_RCONTROL         , "RCONTROL"               },
        { INPUT_KBD_CALCULATOR       , "CALCULATOR"             },
        { INPUT_KBD_PLAYPAUSE        , "PLAYPAUSE"              },
        { INPUT_KBD_MEDIASTOP        , "MEDIASTOP"              },
        { INPUT_KBD_VOLUMEDOWN       , "VOLUMEDOWN"             },
        { INPUT_KBD_VOLUMEUP         , "VOLUMEUP"               },
        { INPUT_KBD_NUMPADCOMMA      , "NUMPADCOMMA"            },
        { INPUT_KBD_DIVIDE           , "DIVIDE"                 },
        { INPUT_KBD_PAUSE            , "PAUSE"                  },
        { INPUT_KBD_PRIOR            , "PRIOR"                  },
        { INPUT_KBD_RIGHT            , "RIGHT"                  },
        { INPUT_KBD_INSERT           , "INSERT"                 },
        { INPUT_KBD_DELETE           , "DELETE"                 },
        { INPUT_KBD_POWER            , "POWER"                  },
        { INPUT_KBD_WEBSEARCH        , "WEBSEARCH"              },
        { INPUT_KBD_WEBFAVORITES     , "WEBFAVORITES"           },
        { INPUT_KBD_WEBREFRESH       , "WEBREFRESH"             },
        { INPUT_KBD_WEBSTOP          , "WEBSTOP"                },
        { INPUT_KBD_WEBFORWARD       , "WEBFORWARD"             },
        { INPUT_KBD_WEBBACK          , "WEBBACK"                },
        { INPUT_KBD_MYCOMPUTER       , "MYCOMPUTER"             },
        { INPUT_KBD_MEDIASELECT      , "MEDIASELECT"            },
    }
};

//==============================================================================

const char* input_pad::GetNameFromGadgetID( s32 iPlatform, input_gadget GadgetID )
{    
    for( s32 i = 0; i < ACTUAL_GADGET_COUNT[iPlatform]; i++ )
    {
        if( GadgetNameMap[iPlatform][i].GadgetID == GadgetID )
            return( GadgetNameMap[iPlatform][i].pName );
    }

    return( "" );
}

//==============================================================================

input_gadget input_pad::GetGadgetIDFromName( s32 iPlatform, const char* pGadgetName )
{
    for( s32 i = 0; i < ACTUAL_GADGET_COUNT[iPlatform]; i++ )
    {
        if( x_strcmp( GadgetNameMap[iPlatform][i].pName, pGadgetName ) == 0 )
            return( GadgetNameMap[iPlatform][i].GadgetID );
    }

    return( INPUT_UNDEFINED );
}

//==============================================================================

input_mgr::input_mgr( void )
{   
}

//==============================================================================

void input_mgr::RegisterPad( input_pad& Pad )
{
    Pad.m_pNext = s_pHead;
    s_pHead = &Pad;

    Pad.OnInitialize();
}

//==============================================================================

xbool input_mgr::Update( f32 DeltaTime )
{
    //
    // First lets go throw all the queue.
    //
#ifndef TARGET_PS2
    while( input_UpdateState() )
    {
        if( input_IsPressed( INPUT_MSG_EXIT ) )
            return TRUE;
    };
#else
    input_UpdateState();
#endif
    
#if defined(TARGET_XBOX)
    s32 iPlatform = INPUT_PLATFORM_XBOX;
#elif defined(TARGET_PS2)
    s32 iPlatform = INPUT_PLATFORM_PS2;
#elif defined(TARGET_PC)
    s32 iPlatform = INPUT_PLATFORM_PC;
#endif

    //
    // Now lets read our input.
    //
#ifndef X_RETAIL
    g_InputTextLine = FIRST_INPUT_TEXT_LINE;
#endif
    for( input_pad* pPad = s_pHead; pPad != NULL; pPad = pPad->m_pNext )
    {
        pPad->OnUpdate( iPlatform, DeltaTime );
    }

    return( FALSE );
}

//==============================================================================
s32 input_mgr::WasPausePressed( void )
{
    for( input_pad* pPad = s_pHead; pPad != NULL; pPad = pPad->m_pNext )
    {
        s32 ControllerID = pPad->m_ControllerID;

        if( ControllerID != -1 )
        {

#if !defined(X_EDITOR) && !defined(CONFIG_RETAIL)            
            if( g_MonkeyOptions.Enabled )
            {
                if ( !g_StateMgr.IsPaused() )
                {
                    // pause the game if the monkey says it is appropriate to do so
                    if ( g_Monkey.ShouldPause() )
                        return ControllerID;
                }
                else
                {
                    // unpause if monkey is exiting menumonkey mode
                    if ( g_Monkey.ShouldUnpause() )
                        return ControllerID;
                }
            }
#endif


#if defined(TARGET_XBOX)
            if( input_WasPressed( INPUT_XBOX_BTN_START, ControllerID ) )
            {
                return ControllerID;
            }
#elif defined(TARGET_PS2)
            if( input_WasPressed(INPUT_PS2_BTN_START, ControllerID ) )
            {
                return ControllerID;
            }
#elif defined(TARGET_PC)
            if( input_WasPressed( INPUT_KBD_ESCAPE, ControllerID ) )
            {
                return ControllerID;
            }
#endif
        }
    }
    return -1;
}
