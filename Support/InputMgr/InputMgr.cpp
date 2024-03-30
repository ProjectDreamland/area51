//==============================================================================
//  InputMgr.cpp
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "InputMgr.hpp"
#include "Monkey.hpp"
#include "../../Apps/GameApp/Config.hpp"
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

#if defined( TARGET_PC ) && !defined(X_RETAIL)
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
#ifndef TARGET_XBOX
            case INPUT_PLATFORM_PS2:
                List.PropEnumHeader( "PS2", "", 0 );
                iHeader = List.PushPath( "PS2\\" );
                break;
#endif
#ifndef TARGET_PS2
            case INPUT_PLATFORM_XBOX:
                List.PropEnumHeader( "XBOX", "", 0 );
                iHeader = List.PushPath( "XBOX\\" );
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
#ifndef TARGET_XBOX
            case INPUT_PLATFORM_PS2:
                iHeader = I.PushPath( "PS2\\" );
                break;
#endif

#ifndef TARGET_PS2
            case INPUT_PLATFORM_XBOX:
                iHeader = I.PushPath( "XBOX\\" );
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
#ifdef TARGET_PC
			    if( i == INPUT_PLATFORM_PS2 )
                {
                    AddMapping( i, iIndex0, INPUT_PS2_STICK_LEFT_X, FALSE );
                }
                else
                {
                    AddMapping( i, iIndex0, INPUT_XBOX_STICK_LEFT_X, FALSE );
                }
#endif
#ifdef TARGET_PS2
                AddMapping( i, iIndex0, INPUT_PS2_STICK_LEFT_X, FALSE );
#endif
#ifdef TARGET_XBOX
                AddMapping( i, iIndex0, INPUT_XBOX_STICK_LEFT_X, FALSE );
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
                //s32 Index = m_Map[iIndex1].GadgetID - INPUT_PS2_BTN_L2;
                I.SetVarEnum( GetNameFromGadgetID( i, m_Map[i][iIndex1].GadgetID ) );
            }
            else
            {
                //for( s32 i=0; pTable[i]; i++ )
                {
                    //if( x_stricmp( I.GetVarEnum(), pTable[i] ) == 0 )
                    {
#ifdef TARGET_XBOX
                        m_Map[i][iIndex1].GadgetID = GetGadgetIDFromName( 1, I.GetVarEnum() );//(input_gadget)(i + INPUT_PS2_BTN_L2);
#else
                        m_Map[i][iIndex1].GadgetID = GetGadgetIDFromName( i, I.GetVarEnum() );//(input_gadget)(i + INPUT_PS2_BTN_L2);
#endif
                        return TRUE;
                    }
                }
                //return FALSE;
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
    else
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
}

//==============================================================================
// Table to map input_gadget to string.

struct gadget_name_map
{
    input_gadget GadgetID;
    const char*  pName;
};

#define MAX_GADGET_MAPS 20

static gadget_name_map GadgetNameMap[2][MAX_GADGET_MAPS] =
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
    }
};

//==============================================================================

const char* input_pad::GetNameFromGadgetID( s32 iPlatform, input_gadget GadgetID )
{    
    for( s32 i = 0; i < MAX_GADGET_MAPS; i++ )
    {
        if( GadgetNameMap[iPlatform][i].GadgetID == GadgetID )
            return( GadgetNameMap[iPlatform][i].pName );
    }

    return( "" );
}

//==============================================================================

input_gadget input_pad::GetGadgetIDFromName( s32 iPlatform, const char* pGadgetName )
{
    for( s32 i = 0; i < MAX_GADGET_MAPS; i++ )
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
    
#ifdef TARGET_XBOX
    s32 iPlatform = INPUT_PLATFORM_XBOX;
#else
    s32 iPlatform = INPUT_PLATFORM_PS2;
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

#if defined(TARGET_PS2) || defined(TARGET_PC)
            if( input_WasPressed(INPUT_PS2_BTN_START, ControllerID ) )
            {
                return ControllerID;
            }
#endif
#if defined(TARGET_XBOX)
            if( input_WasPressed( INPUT_XBOX_BTN_START, ControllerID ) )
            {
                return ControllerID;
            }
#endif
        }
    }
    return -1;
}
