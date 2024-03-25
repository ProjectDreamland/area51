//==============================================================================
//
//  MP_Settings.cpp
//
//==============================================================================
/*
MP Settings\...
EXPORT  SAVE    SHOW
Active Game Types       = header "DM, Tag, INF"     
Active Game Types\DM    = bool                      no      no      yes 
Active Game Types\TDM   = bool                      no      no      yes 

Circuit                 = header "DM, Tag, INF"
Circuit\04              = enum of 4 choices         no      no      yes

Circuit CTF\Values      = s32, all 16 2bit values   yes     no      no  
Circuit CTF\04\Value    = enum of 4 choices         no      yes     no 

*/
//==============================================================================
//  INCLUDES
//==============================================================================

#include "MiscUtils\SimpleUtils.hpp"
#include "MP_Settings.hpp"

//==============================================================================
//  LOCAL STORAGE
//==============================================================================
#ifdef X_EDITOR
//------------------------------------------------------------------------------

const   char*   pEnum = "None\0Team 0 (Alpha)\0Team 1 (Omega)\0All\0";
const   char*   pHelp = "By default, which team owns this circuit?";
xbool   mp_settings::s_DirtyEnum           = TRUE;
xbool   mp_settings::s_DirtyAbbr           = TRUE;
xbool   mp_settings::s_Selected            = FALSE;
f32     mp_settings::s_HighY               = -100000000;
char    mp_settings::s_CircuitEnum[512]    = { '\0' };
char    mp_settings::s_ActiveAbbrList[128] = { '\0' };
u32     mp_settings::s_GameTypeBits        = 0;

const   char*   mp_settings::s_ValueName[] = 
{
    "None", "Team 0 (Alpha)", "Team 1 (Omega)", "All", "<values differ>",
};

//------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------

const   char*   mp_settings::s_GameTypeAbbr[] =
{
    "DM", "TDM", "CTF", "Hunt", "INF", "CNH",
};

s32     mp_settings::s_Circuit[MAX_CIRCUIT_GAMES];      // [GameType]
s32     mp_settings::s_NGameTypes = sizeof(s_GameTypeAbbr) / 4;

//==============================================================================
//  OBJECT DESCRIPTION
//==============================================================================

static struct mp_settings_desc : public object_desc
{
    mp_settings_desc( void ) 
        :   object_desc( object::TYPE_MP_SETTINGS, 
                         "MP Settings",
                         "Multiplayer",
                         object::ATTR_NULL + object::ATTR_NO_RUNTIME_SAVE,
                         FLAGS_GENERIC_EDITOR_CREATE | FLAGS_IS_DYNAMIC )
    {
        // Empty function body.
    }

    //--------------------------------------------------------------------------

    virtual object* Create( void ) 
    { 
        return( new mp_settings ); 
    }

    //--------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32 OnEditorRender( object& Object ) const
    {
        mp_settings::s_Selected = (Object.GetAttrBits() & object::ATTR_EDITOR_SELECTED);
        return( EDITOR_ICON_MP_SETTINGS );
    }

#endif // X_EDITOR

} s_mp_setings_Desc;

//==============================================================================
//  FUNCTIONS
//==============================================================================

const object_desc& mp_settings::GetTypeDesc( void ) const
{
    return( s_mp_setings_Desc );
}

//==============================================================================

const object_desc& mp_settings::GetObjectType( void )
{
    return( s_mp_setings_Desc );
}

//==============================================================================

mp_settings::mp_settings( void )
{
    // The startup values for all circuits are stored in s_Circuit per each
    // game type.  Since there are 4 possible circuit values, only 2 bits are
    // needed per circuit.  Make sure one entry in s_Circuit can hold all
    // MAX_CIRCUITS circuit values.

    ASSERT( (sizeof(s_Circuit[0]) * 8) >= (MAX_CIRCUITS*2) );

    // The first four circuits are constant.  When we construct, go ahead and
    // establish the values for the first four circuits for each game type.
    //  Circuit[0] = 0x00000000 = NONE or HOSTILE TO ALL
    //  Circuit[1] = 0x00000001 = FRIENDLY TO TEAM ALPHA
    //  Circuit[2] = 0x00000002 = FRIENDLY TO TEAM OMEGA
    //  Circuit[3] = 0xFFFFFFFF = (or 3 in 2 bit binary) ALL or FRIENDLY TO ALL

    for( s32 t = 0; t < MAX_CIRCUIT_GAMES; t++ )
    {
        s_Circuit[t] = (0x00 << 0)      // Circuit[0] =  0 // NONE
                     | (0x01 << 2)      // Circuit[1] =  1 // TEAM ALPHA
                     | (0x02 << 4)      // Circuit[2] =  2 // TEAM OMEGA
                     | (0x03 << 6);     // Circuit[3] = -1 // ALL
    }        
}

//==============================================================================

mp_settings::~mp_settings( void )
{
}

//==============================================================================

bbox mp_settings::GetLocalBBox( void ) const 
{ 
    return( bbox( vector3(0,0,0), 50.0f ) );
}

//==============================================================================
#ifdef X_EDITOR
//------------------------------------------------------------------------------

u32 mp_settings::GetTeamBits( s32 Circuit )
{
    u32 TeamBits[5] = { 0x00000000, 0x00000001, 0x00000002, 0xFFFFFFFF, 0xDEADC0DE };

    if( Circuit == MAX_CIRCUITS )
        return( TeamBits[4] );

    ASSERT( IN_RANGE( 0, Circuit, MAX_CIRCUITS ) );

    s32 V = -1;
    for( s32 t = 0; t < MAX_CIRCUIT_GAMES; t++ )
    {
        if( s_GameTypeBits & (1<<t) )
        {
            s32 R = (s_Circuit[t] >> (Circuit*2)) & 0x03;
            if( V == -1 )   V = R;
            if( V !=  R )   V = 4;
        }
    }
    if( V == -1 )   V = 4;

    return( TeamBits[V] );
}

//==============================================================================

const char* mp_settings::GetValueName( s32 Circuit )
{
    ASSERT( IN_RANGE( 0, Circuit, MAX_CIRCUITS ) );

    s32 V = -1;
    for( s32 t = 0; t < MAX_CIRCUIT_GAMES; t++ )
    {
        if( s_GameTypeBits & (1<<t) )
        {
            s32 R = (s_Circuit[t] >> (Circuit*2)) & 0x03;
            if( V == -1 )   V = R;
            if( V !=  R )   V = 4;
        }
    }
    if( V == -1 )   V = 4;

    return( s_ValueName[V] );
}

//==============================================================================

const char* mp_settings::GetCircuitEnum( void )
{
    if( s_DirtyEnum )
    {
        s_DirtyEnum = FALSE;

        // Start off with an empty string.  Then concatenate from there.
        s_CircuitEnum[0] = '\0';

        // For each circuit...
        for( s32 c = 0; c < MAX_CIRCUITS; c++ )
        {
            // Extract the circuit value (2 bits in s_Circuit per game type).
            s32 V = (s_Circuit[s_GameTypeBits] >> (c*2)) & 0x03;

            // Append an entry for the editor enumeration.
            // Example: 05 - Team 0 (Alpha)
            // For the editor enum, each entry needs to be null terminated.
            // Since strcat doesn't leave embedded nulls, just drop '~'s on
            // the end of each entry and then replace them later.
            x_strcat( s_CircuitEnum, 
                xfs( "%02d - %s~", c, GetValueName(c) ) );
        }

        // Now, zip through the string and change all '~' to '\0'.
        char* p = s_CircuitEnum;
        while( *p )
        {
            if( *p == '~' )
                *p = '\0';
            p++;
        }
    }

    return( s_CircuitEnum );
}

//==============================================================================

const char* mp_settings::GetActiveAbbr( void )
{
    if( s_DirtyAbbr )
    {
        s_DirtyAbbr = FALSE;

        s_ActiveAbbrList[0] = '\0';
        for( s32 t = 0; t < s_NGameTypes; t++ )
        {
            if( s_GameTypeBits & (1 << t) )
            {
                if( s_ActiveAbbrList[0] )
                {
                    x_strcat( s_ActiveAbbrList, ", " );
                }
                x_strcat( s_ActiveAbbrList, s_GameTypeAbbr[t] );
            }
        }
    }

    return( s_ActiveAbbrList );
}

//------------------------------------------------------------------------------
#else
//------------------------------------------------------------------------------

u32 mp_settings::GetTeamBits( s32 Circuit, s32 GameType )
{
    u32 TeamBits[4] = { 0x00000000, 0x00000001, 0x00000002, 0xFFFFFFFF };

    ASSERT( IN_RANGE( 0, Circuit, MAX_CIRCUITS ) );
    s32 Value = (s_Circuit[GameType] >> (Circuit*2)) & 0x03;

    return( TeamBits[Value] );
}

//------------------------------------------------------------------------------
#endif
//==============================================================================

void mp_settings::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader( "MP Settings", "Provides data for MP maps.", 0 );

#ifdef X_EDITOR
    // While editing circuits, the user must select which game type the initial
    // circuit values apply to.

    //                                                      EXPORT  SAVE    SHOW
    //  Active Game Types       = header "DM, Tag, INF"         
    //  Active Game Types\DM    = bool                      no      no      yes 
    //  Active Game Types\TDM   = bool                      no      no      yes 

    u32 Flags = PROP_TYPE_MUST_ENUM | 
        PROP_TYPE_DONT_SAVE | 
        PROP_TYPE_DONT_EXPORT | 
        PROP_TYPE_DONT_SAVE_MEMCARD;
    List.PropEnumString( "MP Settings\\Active Game Types", "Select game types to edit.", Flags | PROP_TYPE_HEADER );
    List.PropEnumBool  ( "MP Settings\\Active Game Types\\DM",   "Death Match",          Flags );
    List.PropEnumBool  ( "MP Settings\\Active Game Types\\TDM",  "Team Death Match",     Flags );
    List.PropEnumBool  ( "MP Settings\\Active Game Types\\CTF",  "Capture the Flag",     Flags );
    List.PropEnumBool  ( "MP Settings\\Active Game Types\\Hunt", "Tag",                  Flags );
    List.PropEnumBool  ( "MP Settings\\Active Game Types\\INF",  "Infection",            Flags );
    List.PropEnumBool  ( "MP Settings\\Active Game Types\\CNH",  "Capture and Hold",     Flags );

    //                                                      EXPORT  SAVE    SHOW
    //  Circuit                 = header "DM, Tag, INF"
    //  Circuit\04              = enum of 4 choices         no      no      yes

    List.PropEnumString( "MP Settings\\Circuit", "", 
        PROP_TYPE_HEADER | 
        PROP_TYPE_DONT_SAVE | 
        PROP_TYPE_DONT_EXPORT | 
        PROP_TYPE_DONT_SAVE_MEMCARD );
    if( s_GameTypeBits )
    {
        Flags = PROP_TYPE_DONT_SAVE | 
            PROP_TYPE_DONT_EXPORT | 
            PROP_TYPE_DONT_SAVE_MEMCARD;

        for( s32 c = 0; c < MAX_CIRCUITS; c++ )
        {
            xfs Path( "MP Settings\\Circuit\\%02d", c );

            // The first four circuits are constant.  The rest can be edited.
            if( c >= 4 )
            {
                // MP Settings\Circuit\00
                List.PropEnumEnum( Path,
                    "None\0Team 0 (Alpha)\0Team 1 (Omega)\0All\0",
                    "Initial value.", Flags );
            }
            else
            {
                List.PropEnumString( Path, 
                    "Locked value.", PROP_TYPE_HEADER | Flags );
            }
        }
    }
    else
    {
        for( s32 c = 0; c < MAX_CIRCUITS; c++ )
        {
            xfs Path( "MP Settings\\Circuit\\%02d", c );
            List.PropEnumString( Path, "No game types selected.", 
                PROP_TYPE_HEADER | 
                PROP_TYPE_DONT_SAVE | 
                PROP_TYPE_DONT_EXPORT | 
                PROP_TYPE_DONT_SAVE_MEMCARD );
        }
    }

#endif // #ifdef X_EDITOR

    //                                                      EXPORT  SAVE    SHOW
    //  Circuit CTF\Values      = s32, all 16 2bit values   yes     no      no  
    //  Circuit CTF\04\Value    = enum of 4 choices         no      yes     no 

    // For each game type...
    for( s32 t = 0; t < s_NGameTypes; t++ )
    {
        // This field is not saved or visible, but it IS exported.
        List.PropEnumInt( xfs( "MP Settings\\Circuit %s\\Values", s_GameTypeAbbr[t] ),
            "", 
            PROP_TYPE_DONT_SAVE_MEMCARD | 
            PROP_TYPE_DONT_SHOW | 
            PROP_TYPE_DONT_SAVE );

#ifdef X_EDITOR
        // If we are in the editor, we need the individual circuit values.  
        // These are not visible, not exported, and not saved to memory card.
        // For every circuit (within the game type)...
        for( s32 c = 0; c < MAX_CIRCUITS; c++ )
        {
            xfs Path( "MP Settings\\Circuit %s\\%02d\\Value", 
                s_GameTypeAbbr[t], c );

            // The first four circuits are constant.  The rest can be edited
            // indirectly.
            if( c >= 4 )
            {
                // MP Settings\Circuit DM\00\Value
                List.PropEnumEnum  ( Path,
                    "None\0Team 0 (Alpha)\0Team 1 (Omega)\0All\0",
                    "",
                    PROP_TYPE_DONT_SAVE_MEMCARD | 
                    PROP_TYPE_DONT_SHOW | 
                    PROP_TYPE_DONT_EXPORT );
            }
            else
            {
                // MP Settings\Circuit DM\04\Value
                List.PropEnumString( Path,
                    "",
                    PROP_TYPE_DONT_SAVE_MEMCARD | 
                    PROP_TYPE_DONT_SHOW | 
                    PROP_TYPE_DONT_SAVE | 
                    PROP_TYPE_DONT_EXPORT | 
                    PROP_TYPE_HEADER );
            }
        }
#endif // #ifdef X_EDITOR
    }

    //
    // For loading MP resources.
    //
    {
        // These are needed for a HACK which gets the MP avatar resources exported.
        // Since only MP levels have spawn points, the hack landed here.
        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\AnimFile",  SMP_ANIM      );
        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\SkinFile",  SMP_SKINGEOM  );

        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\CapBase",   SMP_RIGIDGEOM );
        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\CapBall",   SMP_RIGIDGEOM );
        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\CapBeam",   SMP_FXO       );
        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\CapTether", SMP_FXO       );
        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\CapAnim",   SMP_ANIM      );

        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\FlagBase",  SMP_RIGIDGEOM );

        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\MP Common Sounds",   SMP_AUDIOPKG );
        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\MP Intercom Sounds", SMP_AUDIOPKG );

        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\FlagSpawn",      SMP_FXO       );
        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\FlagCapture",    SMP_FXO       );
        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\FlagDisappear",  SMP_FXO       );

        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\ContagiousHud",  SMP_FXO       );
        SMP_UTIL_EnumHiddenManualResource( List, "MP Settings\\ContagionTether",  SMP_FXO       );
    }

    object::OnEnumProp( List );
}

//------------------------------------------------------------------------------

xbool mp_settings::OnProperty( prop_query& Query )
{
    if( object::OnProperty( Query ) )
    {
        return( TRUE );
    }

#ifdef X_EDITOR

    // Yes, this could probably be done a little more gracefully.  But since 
    // this code is only present in the editor (and NOT in the game), I'm not
    // too worried about it.  Got bigger fish to fry.

    /*
    List.PropEnumBool  ( "MP Settings\\Active Game Types\\DM",   "Death Match",      Flags );
    List.PropEnumBool  ( "MP Settings\\Active Game Types\\TDM",  "Team Death Match", Flags );
    List.PropEnumBool  ( "MP Settings\\Active Game Types\\CTF",  "Capture the Flag", Flags );
    List.PropEnumBool  ( "MP Settings\\Active Game Types\\Hunt", "Tag",              Flags );
    List.PropEnumBool  ( "MP Settings\\Active Game Types\\INF",  "Infection",        Flags );
    List.PropEnumBool  ( "MP Settings\\Active Game Types\\CNH",  "Capture and Hold", Flags );
    */

    //                                                      EXPORT  SAVE    SHOW
    //  Active Game Types       = header "DM, Tag, INF"         
    //  Active Game Types\DM    = bool                      no      no      yes 
    //  Active Game Types\TDM   = bool                      no      no      yes 

    if( Query.IsRead() && 
        Query.IsVar( "MP Settings\\Active Game Types" ) )
    {
        Query.SetVarString( GetActiveAbbr(), 128 );
        return( TRUE );
    }

    s32 Index = -1;
    for( s32 t = 0; t < s_NGameTypes; t++ )
    {
        if( Query.IsVar( xfs( "MP Settings\\Active Game Types\\%s", s_GameTypeAbbr[t] ) ) )
        {
            Index = t;
            break;
        }
    }
    if( Index != -1 )
    {
        if( Query.IsRead() )
        {
            Query.SetVarBool( s_GameTypeBits & (1<<Index) ? 1 : 0 );
        }
        else
        {
            s_DirtyAbbr = TRUE;
            s_DirtyEnum = TRUE;
            s_GameTypeBits &= ~(1<<Index);
            if( Query.GetVarBool() )
                s_GameTypeBits |= (1<<Index);
        }
        return( TRUE );
    }

    //                                                      EXPORT  SAVE    SHOW
    //  Circuit                 = header "DM, Tag, INF"
    //  Circuit\04              = enum of 4 choices         no      no      yes

    if( Query.IsRead() && 
        Query.IsVar( "MP Settings\\Circuit" ) )
    {
        Query.SetVarString( GetActiveAbbr(), 128 );
        return( TRUE );
    }

    for( s32 c = 0; c < MAX_CIRCUITS; c++ )
    {
        if( s_GameTypeBits &&
            Query.IsVar( xfs( "MP Settings\\Circuit\\%02d", c ) ) )
        {
            if( c < 4 )
            {
                // First 4 circuits are locked.
                if( Query.IsRead() )
                {
                    Query.SetVarString( s_ValueName[c], 32 );
                    return( TRUE );
                }
            }
            else
            {
                if( Query.IsRead() )
                {
                    if( s_GameTypeBits )
                    {
                        s32 V = -1;
                        for( s32 t = 0; t < MAX_CIRCUIT_GAMES; t++ )
                        {
                            if( s_GameTypeBits & (1<<t) )
                            {
                                s32 R = (s_Circuit[t] >> (c*2)) & 0x03;
                                if( V == -1 )   V = R;
                                if( V !=  R )   V = 4;
                            }
                        }
                        Query.SetVarEnum( s_ValueName[V] );
                    }
                    return( TRUE );
                }
                else
                {   
                    for( s32 t = 0; t < MAX_CIRCUIT_GAMES; t++ )
                    {
                        const char* pString = Query.GetVarEnum();
                        for( s32 V = 0; V < 4; V++ )
                        {
                            if( x_strcmp( s_ValueName[V], pString ) == 0 )
                                break;
                        }
                        ASSERT( V < 4 );
                        if( s_GameTypeBits & (1<<t) )
                        {
                            s_Circuit[t] &= ~(3 << (c*2));
                            s_Circuit[t] |=  (V << (c*2));
                        }
                    }
                    return( TRUE );
                }
            }
        }
    }

#endif // #ifdef X_EDITOR

    //                                                      EXPORT  SAVE    SHOW
    //  Circuit CTF\Values      = s32, all 16 2bit values   yes     no      no  
    //  Circuit CTF\04\Value    = enum of 4 choices         no      yes     no 

    // For each game type...
    for( s32 t = 0; t < s_NGameTypes; t++ )
    {
        // Take care of the single value which holds all of the initial circuit
        // settings for this game type.
        if( Query.VarInt( xfs( "MP Settings\\Circuit %s\\Values", s_GameTypeAbbr[t] ),
            s_Circuit[t] ) )
        {
            return( TRUE );
        }

#ifdef X_EDITOR

        // For each circuit (within the game type)...
        for( s32 c = 0; c < MAX_CIRCUITS; c++ )
        {
            // The first four circuits are locked, so only provide edit ability
            // for c >= 4.
            // MP Settings\Circuit DM\00\Value
            if( (c >= 4) &&
                Query.IsVar( xfs( "MP Settings\\Circuit %s\\%02d\\Value", 
                s_GameTypeAbbr[t], c ) ) )
            {
                // When ever there is a change, we need to rebuild the string
                // that "circuit consumers" use for their property enumeration.
                s_DirtyEnum = TRUE;

                if( Query.IsRead() )
                {
                    // Extract the circuit's value from s_Circuit, then populate
                    // the property enumeration with the appropriate string.
                    u32 Value = (s_Circuit[t] >> (c*2)) & 0x03;
                    switch( Value )
                    {
                    case 0x00:    Query.SetVarEnum( "None" );           break;
                    case 0x01:    Query.SetVarEnum( "Team 0 (Alpha)" ); break;
                    case 0x02:    Query.SetVarEnum( "Team 1 (Omega)" ); break;
                    case 0x03:    Query.SetVarEnum( "All" );            break;
                    default:      ASSERT( FALSE );                      break;
                    }    
                    return( TRUE );
                }
                else
                {
                    // Based on the string in the enumerated property, determine
                    // the circuit value and then encode that into s_Circuit for
                    // safe keeping.
                    u32 Value = 0;
                    const char* pString = Query.GetVarEnum();
                    if( x_stricmp( pString, "None"           ) == 0 )    { Value = 0x00; }
                    if( x_stricmp( pString, "Team 0 (Alpha)" ) == 0 )    { Value = 0x01; }
                    if( x_stricmp( pString, "Team 1 (Omega)" ) == 0 )    { Value = 0x02; }
                    if( x_stricmp( pString, "All"            ) == 0 )    { Value = 0x03; }
                    s_Circuit[t] &= ~(0x03  << (c*2));
                    s_Circuit[t] |=  (Value << (c*2));
                    return( TRUE );
                }
            }
        }

#endif // #ifdef X_EDITOR
    }

    //
    // For loading MP resources.
    //
    {
        // See HACK comments in OnEnumProp() above.
        if( Query.IsVar( "MP Settings\\AnimFile" ) && Query.IsRead() )
        {
            Query.SetVarExternal( PRELOAD_MP_FILE( "MP_AVATAR.anim" ), RESOURCE_NAME_SIZE );
            return( TRUE );
        }

        // See HACK comments in OnEnumProp() above.
        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\SkinFile", "MP_AVATAR_BIND.skingeom" ))
            return( TRUE );

        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\CapBase", "MP_Cap_Point_Base001.rigidgeom" ))
            return( TRUE );

        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\CapBall", "MP_Cap_Point_001.rigidgeom" ))
            return( TRUE );

        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\CapBeam", "MP_CapturePtBeam.fxo" ))
            return( TRUE );

        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\CapTether", "MP_ElecTeather_000.fxo" ))
            return( TRUE );

        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\CapAnim", "MP_Cap_Point_002.anim" ))
            return TRUE;

        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\FlagBase", "MP_team_flag_base_000.rigidgeom" ))
            return TRUE;

        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\MP Common Sounds",   "SFX_MP_COMMON.AUDIOPKG" ))
            return TRUE;

        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\MP Intercom Sounds", "DX_MP_INTERCOMFEMALE.AUDIOPKG" ))
            return TRUE;

        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\CapBall",         "MP_Cap_Point_001.rigidgeom" ))
            return( TRUE );

        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\FlagSpawn",       "MP_FlagReturned.fxo" ))
            return( TRUE );
        
        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\FlagCapture",     "MP_FlagCaptured.fxo" ))
            return( TRUE );
        
        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\FlagDisappear",   "MP_ctf_flag_disappear.fxo"  ))
            return( TRUE );

        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\ContagiousHud",   "HUD_ContagionBorder.fxo"  ))
            return( TRUE );

        if( SMP_UTIL_IsHiddenManualResource( Query, "MP Settings\\ContagionTether",   "ContagionTether.fxo"  ))
            return( TRUE );
    }

    return( FALSE );
}

//==============================================================================