//==============================================================================
//
//  Cinema.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "Cinema.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "ConversationMgr\ConversationMgr.hpp"
#include "Loco\LocoUtil.hpp"
#include "Characters\Character.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Animation\AnimPlayer.hpp"

//=========================================================================
// DATA
//=========================================================================

// End of cinema marker text
char* s_pEndOfCinema = "End of cinema!";

//=========================================================================

// Next AI state exposed for actors after playing lip sync
typedef enum_pair<s32> character_state_enum_pair;
static character_state_enum_pair s_CharacterStateEnumPair[] = 
{
    // Available states
    character_state_enum_pair( "NULL",      character_state::STATE_NULL     ),
    character_state_enum_pair( "IDLE",      character_state::STATE_IDLE     ),
    character_state_enum_pair( "HOLD",      character_state::STATE_HOLD     ),
    character_state_enum_pair( "ATTACK",    character_state::STATE_ATTACK   ),

    //**MUST BE LAST**//    
    character_state_enum_pair( k_EnumEndStringConst,   character_state::STATE_NULL      )
};
enum_table<s32>  s_CharacterStateList( s_CharacterStateEnumPair );              

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct cinema_object_desc : public object_desc
{
    cinema_object_desc( void ) : object_desc( object::TYPE_AUDIO_DRIVEN_CINEMA, 
        "Cinema",
        "SCRIPT",

        object::ATTR_NEEDS_LOGIC_TIME,

        FLAGS_GENERIC_EDITOR_CREATE | FLAGS_TARGETS_OBJS |
        FLAGS_IS_DYNAMIC ) {}

        //-------------------------------------------------------------------------

        virtual object* Create( void ) { return new cinema_object; }

        //-------------------------------------------------------------------------

#ifdef X_EDITOR

        virtual s32 OnEditorRender( object& Object ) const
        {
            (void)Object;
            return( EDITOR_ICON_CINEMA_OBJECT );
        }

#endif // X_EDITOR

} s_cinema_object_Desc;

//==============================================================================

const object_desc& cinema_object::GetTypeDesc( void ) const
{
    return s_cinema_object_Desc;
}

//==============================================================================

const object_desc& cinema_object::GetObjectType   ( void )
{
    return s_cinema_object_Desc;
}

//==============================================================================
// position_marker
//==============================================================================

cinema_object::position_marker::position_marker() 
{
    Init();
};

void cinema_object::position_marker::Init( void )
{
    Time     = 0.0f;
};

//==============================================================================
// action_marker
//==============================================================================

cinema_object::action_marker::action_marker()
{
    Init();
};

void cinema_object::action_marker::Init( void )
{
    MarkerType = BLOCKING;
    Time       = 0.0f;
    Name[0]    = 0;
};

//==============================================================================
// cinema_character
//==============================================================================

cinema_object::cinema_character::cinema_character()
{
    Init();
};

void cinema_object::cinema_character::Init( void )
{
    StartMarkerDistance = 0.0f;
    StartMarkerYaw      = R_0;
    hAnimGroup.SetName( "" );
    AnimGroupName = -1;
    AnimName      = -1;
    AnimFlags     = loco::ANIM_FLAG_TURN_OFF_AIMER | loco::ANIM_FLAG_MASK_TYPE_FULL_BODY;
    NextAiState   = character_state::STATE_NULL;
    hNextAnimGroup.SetName( "" );
    iNextAnim     = -1;
};

//==============================================================================
// cinema_object
//==============================================================================

cinema_object::cinema_object(void)
{
    m_Characters.Clear();
    m_PositionMarkers.Clear();
    m_ActionMarkers.Clear(); 

    m_bCinemaActive     = FALSE;
    m_bCinemaDone       = FALSE;
    m_bIs2D             = FALSE;
    m_AudioStatus       = AUDIO_STATUS_OFF;
    m_AnimStatus        = ANIM_STATUS_OFF;
    
    m_nPositionMarkers  = 0;
    m_nActionMarkers    = 0;
    m_nCharacters       = 0;
    m_PrevTime          = 0.0f;
    m_CurrTime          = 0.0f;
    m_AudioLength       = 0.0f;
    m_VoiceID           = 0;
    m_Descriptor[0]     = 0; 

    m_PositionMarkerIndex = -1;
#ifdef X_EDITOR
    m_pEnumString = NULL;
    UpdateEnumString();
#endif // X_EDITOR
}

//==============================================================================

cinema_object::~cinema_object(void)
{
#ifdef X_EDITOR
    if( m_pEnumString )
        x_free( m_pEnumString );
#endif // X_EDITOR
}

//==============================================================================

bbox cinema_object::GetLocalBBox( void ) const
{
    return( bbox( vector3(0,0,0), 50.0f ) );
}

//==============================================================================

void cinema_object::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader  ( "Cinema", "Cinema info", PROP_TYPE_DONT_EXPORT );

    List.PropEnumBool    ( "Cinema\\Active",        "Does this cinema start active?", 0 );
    List.PropEnumString  ( "Cinema\\Audio Package", "The audio package associated with this cinema.", PROP_TYPE_READ_ONLY );
    List.PropEnumExternal( "Cinema\\Audio Package External Resource", "Resource\0audiopkg\0", "", PROP_TYPE_DONT_SHOW );
    List.PropEnumExternal( "Cinema\\Audio Descriptor", "Sound\0soundexternal\0","Cinema Sound Descriptor (Label)", PROP_TYPE_MUST_ENUM  );    
    List.PropEnumFloat   ( "Cinema\\Audio Length", "Length of the audio in seconds.", PROP_TYPE_READ_ONLY|PROP_TYPE_DONT_EXPORT );

    m_PlayerEndMarker.OnEnumProp( List, "Cinema\\Player End Marker", PROP_TYPE_MUST_ENUM );

    // Characters
    List.PropEnumInt     ( "Cinema\\Character Count", "Number of characters", PROP_TYPE_DONT_SHOW );
    List.PropEnumButton  ( "Cinema\\Add Character",   "Adds a new character to the list.", PROP_TYPE_MUST_ENUM );
    for( s32 i=0 ; i<m_nCharacters ; i++ )
    {
        cinema_character& CinChar = m_Characters[i];
        
        List.PropEnumString( xfs( "Cinema\\Character[%d]", i ), "", PROP_TYPE_HEADER ); 
        s32 iHeader = List.PushPath( xfs("Cinema\\Character[%d]\\", i ) );

        CinChar.CharacterAffecter.OnEnumProp( List, "Character", PROP_TYPE_MUST_ENUM );
        CinChar.StartMarkerAffecter.OnEnumProp( List, "Start Marker",    PROP_TYPE_MUST_ENUM );
        
        // Hide marker info if not specified
        u32 Flags = 0;
        if( !CinChar.StartMarkerAffecter.GetObjectPtr() )
            Flags = PROP_TYPE_DONT_SHOW;
             
        List.PropEnumFloat ( "Start Marker Distance", "Distance threshold before character is popped to the marker position", Flags );
        List.PropEnumAngle ( "Start Marker Yaw",      "Yaw threshold before character is popped to the marker rotation", Flags );

        CinChar.EndMarkerAffecter.OnEnumProp( List, "End Marker",    PROP_TYPE_MUST_ENUM );

        LocoUtil_OnEnumPropAnimFlags( List, 
                                      loco::ANIM_FLAG_TURN_OFF_AIMER | loco::ANIM_FLAG_MASK_TYPE_FULL_BODY,
                                      CinChar.AnimFlags );

        List.PropEnumEnum    ( "Next State" ,    s_CharacterStateList.BuildString(), "State to transition the AI to after the cinema is complete.", 0 );
        List.PropEnumHeader  ( "Remove",         "Remove character", PROP_TYPE_DONT_EXPORT );
        List.PropEnumButton  ( "Remove\\Remove", "Remove Character", PROP_TYPE_MUST_ENUM );

        List.PopPath( iHeader );
    }

    // Position markers
    List.PropEnumInt     ( "Cinema\\Audio Positions",                  "Audio Positions", PROP_TYPE_HEADER );
    List.PropEnumInt     ( "Cinema\\Audio Positions\\Position Count",  "Number of positions", PROP_TYPE_DONT_SHOW );
    List.PropEnumButton  ( "Cinema\\Audio Positions\\Add Position",    "Adds a new audio position to the list.", PROP_TYPE_MUST_ENUM );
    for( s32 i=0 ; i<m_nPositionMarkers ; i++ )
    {
        List.PropEnumString( xfs( "Cinema\\Audio Positions\\Position[%d]", i ), "", PROP_TYPE_HEADER ); 
        s32 iHeader = List.PushPath( xfs("Cinema\\Audio Positions\\Position[%d]\\", i ) );        

        List.PropEnumFloat   ( "Time",           "Audio position time", PROP_TYPE_MUST_ENUM );
        m_PositionMarkers[i].ObjectAffecter.OnEnumProp( List, "Position", PROP_TYPE_MUST_ENUM );
        List.PropEnumHeader  ( "Remove",         "Remove position", PROP_TYPE_DONT_EXPORT );
        List.PropEnumButton  ( "Remove\\Remove", "Remove position", PROP_TYPE_MUST_ENUM );

        List.PopPath( iHeader );
    }

    // Action markers
    List.PropEnumInt   ( "Cinema\\Marker Count", "Number of markers", PROP_TYPE_DONT_SHOW );
    List.PropEnumButton( "Cinema\\Add Marker",   "Add a new marker to the list.", PROP_TYPE_MUST_ENUM );
    for( s32 i=0 ; i<m_nActionMarkers ; i++ )
    {
        List.PropEnumString( xfs( "Cinema\\Marker[%d]", i ), "", PROP_TYPE_HEADER ); 
        s32 iHeader = List.PushPath( xfs("Cinema\\Marker[%d]\\", i ) );        

        List.PropEnumFloat       ( "Time",           "Marker Time", PROP_TYPE_MUST_ENUM );
        List.PropEnumEnum        ( "Type",           "Blocking\0Activating\0", "Type of marker", PROP_TYPE_MUST_ENUM );
        if( m_ActionMarkers[i].MarkerType == BLOCKING )
        {
            List.PropEnumString  ( "Blocking Label", "Blocking marker label", PROP_TYPE_MUST_ENUM );
        } 
        else if ( m_ActionMarkers[i].MarkerType == ACTIVATING )
        {
            m_ActionMarkers[i].ObjectAffecter.OnEnumProp( List, "Marker", PROP_TYPE_MUST_ENUM );
        }
        List.PropEnumHeader      ( "Remove",         "Remove marker", PROP_TYPE_DONT_EXPORT );
        List.PropEnumButton      ( "Remove\\Remove", "Remove marker", PROP_TYPE_MUST_ENUM );
        List.PopPath( iHeader );    
    }
}

//==============================================================================

xbool cinema_object::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
    {
        return( TRUE );
    }

    // Active?
    if( I.IsVar( "Cinema\\Active" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarBool( (GetAttrBits() & object::ATTR_NEEDS_LOGIC_TIME) != 0 );
        }
        else
        {
            OnActivate( I.GetVarBool() );
        }

        return TRUE;
    }


    // External
    if( I.IsVar( "Cinema\\Audio Descriptor" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_Descriptor, 64 );
        }
        else
        {
            // Get the Descriptor
            xstring ExtString = I.GetVarExternal();
            if( !ExtString.IsEmpty() )
            {
                xstring String( ExtString );

                if( String == "<null>" )
                {
                    m_AudioPackage.SetName( "" );
                    m_Descriptor[0] = 0;
                    m_AudioLength   = 0.0f;
                }
                else
                {
                    s32 PkgIndex = String.Find( '\\', 0 );

                    if( PkgIndex != -1 )
                    {
                        xstring Pkg = String.Left( PkgIndex );
                        String.Delete( 0, PkgIndex+1 );

                        m_AudioPackage.SetName( Pkg );                

                        // Load the audio package.
                        if( m_AudioPackage.IsLoaded() == FALSE )
                            m_AudioPackage.GetPointer();
                    }

                    x_strncpy( m_Descriptor, String, 64 );

                    m_AudioLength = g_AudioMgr.GetLengthSeconds( m_Descriptor );
                }
            }
        }

        return( TRUE );
    }

    // External
    if( I.IsVar( "Cinema\\Audio Package External Resource" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_AudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                if( xstring(pString) == "<null>" )
                {
                    m_AudioPackage.SetName( "" );
                }
                else
                {
                    m_AudioPackage.SetName( pString );                

                    // Load the audio package.
                    if( m_AudioPackage.IsLoaded() == FALSE )
                        m_AudioPackage.GetPointer();
                }
            }
        }
        return( TRUE );        
    }

    if( I.IsVar( "Cinema\\Audio Package" ) )
    {
        // You can only read.
        if( I.IsRead() )
        {
            I.SetVarString( m_AudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        return TRUE;
    }

    if( I.IsVar( "Cinema\\Audio Length" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_AudioLength );
        }
        return TRUE;
    }

    // Add a character.
    if( I.IsVar( "Cinema\\Add Character" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add" );
        }
        else
        {
            cinema_character& CinChar = m_Characters.Append();
            CinChar.Init();
            
            m_nCharacters = m_Characters.GetCount();
        }
        return TRUE;
    }

    // Character count.
    if( I.IsVar("Cinema\\Character Count") )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nCharacters );
        }
        else
        {
            m_nCharacters = I.GetVarInt( 0 );
            m_Characters.SetCapacity( m_nCharacters );
            m_Characters.SetCount( m_nCharacters );
        }
        return TRUE;
    }

    // Player end marker
    if( m_PlayerEndMarker.OnProperty( I, "Cinema\\Player End Marker" ) )
    {
        return TRUE;
    }

    //=============================
    // Handle the characters.
    //=============================

    if( I.IsSimilarPath( "Cinema\\Character" ) && 
       !I.IsSimilarPath( "Cinema\\Characters" ) )
    {
        // Lookup cinema character
        s32                 i       = I.GetIndex(0);
        cinema_character&   CinChar = m_Characters[i];
        
        ASSERT( m_Characters.GetCount() );

        if ( I.IsVar( "Cinema\\Character[]" ) )
        {
#ifdef X_EDITOR
            if( I.IsRead() )
            {
                object* pObject = CinChar.CharacterAffecter.GetObjectPtr();
                if( pObject )
                    I.SetVarString( pObject->GetName(), 64 );
                else
                    I.SetVarString( "Unknown", 64 );
            }
#endif // X_EDITOR
            return TRUE;
        }

        s32 iHeader = I.PushPath( "Cinema\\Character[]\\" );        
        
        if( CinChar.CharacterAffecter.OnProperty( I, "Character" ) )
        {
            return TRUE;
        }

        if( CinChar.StartMarkerAffecter.OnProperty( I, "Start Marker" ) )
        {
            return TRUE;
        }

        if( CinChar.EndMarkerAffecter.OnProperty( I, "End Marker" ) )
        {
            return TRUE;
        }

        if( I.VarFloat( "Start Marker Distance", CinChar.StartMarkerDistance, 0.0f, 100.0f*100.0f ) )
        {
            return TRUE;
        }

        if( I.VarAngle( "Start Marker Yaw", CinChar.StartMarkerYaw, R_0, R_360 ) )
        {
            return TRUE;
        }

        // Next state
        if ( I.IsVar( "Next State" ) ) 
        {
            // Updating UI?
            if( I.IsRead() )
            {
                if ( s_CharacterStateList.DoesValueExist( CinChar.NextAiState ) )
                {
                    I.SetVarEnum( s_CharacterStateList.GetString( CinChar.NextAiState ) );
                }
                else
                {
                    I.SetVarEnum( "NULL" );
                } 
            }
            else
            {
                // Reading from UI/File
                const char* pValue = I.GetVarEnum();

                // Found?
                if( !s_CharacterStateList.GetValue( pValue, CinChar.NextAiState ) )
                    CinChar.NextAiState = character_state::STATE_NULL;
            }
            
            return TRUE;      
        }        

        if( I.IsVar( "Remove\\Remove" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarButton( "Remove" );
            }
            else
            {
                m_Characters.Delete( i );
                m_nCharacters = m_Characters.GetCount();
            }
            return TRUE;
        }

        // Backwards compatiblity START
        // Character guid.
        if( I.IsVar( "Static GUID" ) )
        {
            if( !I.IsRead() )
            {
                CinChar.CharacterAffecter.SetStaticGuid( I.GetVarGUID() );
            }
            return TRUE;
        }
        // Backwards compatiblity END

        // Animation properties?
        if( LocoUtil_OnPropertyAnimFlags( I,
                                          CinChar.AnimGroupName,
                                          CinChar.AnimName,
                                          CinChar.AnimFlags,
                                          CinChar.hAnimGroup ) )
        {                                          
            return TRUE;
        }
        
        I.PopPath( iHeader );
    }

    // ================================= Begin old character way =================================
    // Remove character.
    if( I.IsVar( "Cinema\\Characters\\Remove[]\\Remove" ) )
    {
        s32 i = I.GetIndex(0);

        if( I.IsRead() )
        {
            I.SetVarButton( xfs("Remove[%d]",i) );
        }
        else
        {
            m_Characters.Delete( i );
            m_nCharacters = m_Characters.GetCount();
        }
        return TRUE;
    }

    // Add a character.
    if( I.IsVar( "Cinema\\Characters\\Add Character" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add" );
        }
        else
        {
            cinema_character& CinChar = m_Characters.Append();
            CinChar.Init();
            
            m_nCharacters = m_Characters.GetCount();
        }
        return TRUE;
    }

    // Character count.
    if( I.IsVar("Cinema\\Characters\\Character Count") )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nCharacters );
        }
        else
        {
            m_nCharacters = I.GetVarInt( 0 );
            m_Characters.SetCapacity( m_nCharacters );
            m_Characters.SetCount( m_nCharacters );
        }
    }

    // Character guid.
    if( I.IsVar( "Cinema\\Characters\\Static GUID[]" ) )
    {
        s32 i = I.GetIndex(0);

        if( !I.IsRead() )
        {
            m_Characters[i].CharacterAffecter.SetStaticGuid( I.GetVarGUID() );
        }
        return TRUE;
    }

    // Anim group.
    if( I.IsVar( "Cinema\\Characters\\Anim Package[]" ) )
    {
        s32 i = I.GetIndex(0);

        if( I.IsRead() )
        {
            if( m_Characters[i].AnimGroupName >= 0 )
                I.SetVarExternal( g_StringMgr.GetString( m_Characters[i].AnimGroupName ), 256 );
            else
                I.SetVarExternal("", 256);
        }
        else
        {
            // Get the FileName
            xstring String = I.GetVarExternal();
            if( !String.IsEmpty() )
            {
                s32 PkgIndex = String.Find( '\\', 0 );
                if( PkgIndex != -1 )
                {
                    xstring Pkg = String.Left( PkgIndex );
                    Pkg += "\0\0";
                    m_Characters[i].AnimName = g_StringMgr.Add( String.Right( String.GetLength() - PkgIndex - 1) );
                    m_Characters[i].AnimGroupName = g_StringMgr.Add( Pkg );
                    m_Characters[i].hAnimGroup.SetName( Pkg );
                }
                else
                {
                    m_Characters[i].AnimGroupName = g_StringMgr.Add( String );
                    m_Characters[i].hAnimGroup.SetName( String );
                }
            }
        }
        return TRUE;
    }

    // Anim name.
    if( I.IsVar( "Cinema\\Characters\\Anim Name[]" ) )
    {
        s32 i = I.GetIndex(0);

        if( I.IsRead() )
        {
            if( m_Characters[i].AnimName >= 0 )
                I.SetVarString( g_StringMgr.GetString( m_Characters[i].AnimName ), 256 );
            else
                I.SetVarString( "", 256 );
        }
        else
        {
            if( x_strlen( I.GetVarString() ) > 0 )
                m_Characters[i].AnimName = g_StringMgr.Add( I.GetVarString() );
        }
        return TRUE;
    }
    // ================================= End old character way =================================

    //=============================
    // Handle the position markers.
    //=============================

    if( I.IsVar( "Cinema\\Audio Positions" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nPositionMarkers );
        }

        return TRUE;
    }

    // Add position.
    if( I.IsVar( "Cinema\\Audio Positions\\Add Position" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add" );
        }
        else
        {
            position_marker& PosMarker = m_PositionMarkers.Append();
            PosMarker.Init();
            
            m_nPositionMarkers = m_PositionMarkers.GetCount();
        }
        return TRUE;
    }

    // Position count.
    if( I.IsVar("Cinema\\Audio Positions\\Position Count") )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nPositionMarkers );
        }
        else
        {
            m_nPositionMarkers = I.GetVarInt( 0 );
            m_PositionMarkers.SetCapacity( m_nPositionMarkers );
            m_PositionMarkers.SetCount( m_nPositionMarkers );
        }
        return TRUE;
    }

    if( I.IsSimilarPath( "Cinema\\Audio Positions\\Position" ) )
    {
        s32 i = I.GetIndex(0);
        (void)i;

        if ( I.IsVar( "Cinema\\Audio Positions\\Position[]" ) )
        {
#ifdef X_EDITOR
            if( I.IsRead() )
            {
                xstring     Label;
                const char* pName;
                object* pObject = m_PositionMarkers[i].ObjectAffecter.GetObjectPtr();
                if( pObject )
                    pName = pObject->GetName();
                else
                    pName = "Unknown";
                Label.Format( "%06.3f [%s]", m_PositionMarkers[i].Time, pName );
                I.SetVarString( (const char*)Label, 64 );
            }
#endif // X_EDITOR
            return TRUE;
        }

        s32 iHeader = I.PushPath( "Cinema\\Audio Positions\\Position[]\\" ); 

        // GUID.
        if( m_PositionMarkers[i].ObjectAffecter.OnProperty( I, "Position" ) )
        {
            return TRUE;
        }

        // Remove position.
        if( I.IsVar( "Remove\\Remove" ) )
        {
            s32 i = I.GetIndex(0);

            if( I.IsRead() )
            {
                I.SetVarButton( "Remove" );
            }
            else
            {
                m_PositionMarkers.Delete( i );
                m_nPositionMarkers = m_PositionMarkers.GetCount();
            }
            return TRUE;
        }

        // Is it a time?
        if( I.IsVar( "Time" ) )
        {
            s32 i = I.GetIndex(0);

            if( I.IsRead() )
            {
                I.SetVarFloat( m_PositionMarkers[i].Time );
            }
            else
            {
                m_PositionMarkers[i].Time = I.GetVarFloat( 0.0f, m_AudioLength );
            }
            return TRUE;
        }

        // Is is a static guid?
        if( I.IsVar( "Static GUID" ) )
        {
            s32 i = I.GetIndex(0);

            if( !I.IsRead() )
            {
                m_PositionMarkers[i].ObjectAffecter.SetStaticGuid( I.GetVarGUID() );
            }
            return TRUE;
        }

        I.PopPath( iHeader );
    }

    // ================================= Begin old position way =================================
    // Remove position.
    // Marker count.
    if( I.IsVar("Cinema\\Audio Positioning\\Position Count") )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nPositionMarkers );
        }
        else
        {
            m_nPositionMarkers = I.GetVarInt( 0 );
            m_PositionMarkers.SetCapacity( m_nPositionMarkers );
            m_PositionMarkers.SetCount( m_nPositionMarkers );
        }
    }

    if( I.IsVar( "Cinema\\Audio Positioning\\Remove[]\\Remove" ) )
    {
        s32 i = I.GetIndex(0);

        if( I.IsRead() )
        {
            I.SetVarButton( xfs("Remove[%d]",i) );
        }
        else
        {
            m_PositionMarkers.Delete( i );
            m_nPositionMarkers = m_PositionMarkers.GetCount();
        }
        return TRUE;
    }

    // Is it a time?
    if( I.IsVar( "Cinema\\Audio Positioning\\Time[]" ) )
    {
        s32 i = I.GetIndex(0);

        if( I.IsRead() )
        {
            I.SetVarFloat( m_PositionMarkers[i].Time );
        }
        else
        {
            m_PositionMarkers[i].Time = I.GetVarFloat( 0.0f, m_AudioLength );
        }
        return TRUE;
    }

    // Is is a static guid?
    if( I.IsVar( "Cinema\\Audio Positioning\\Static GUID[]" ) )
    {
        s32 i = I.GetIndex(0);

        if( !I.IsRead() )
        {
            m_PositionMarkers[i].ObjectAffecter.SetStaticGuid( I.GetVarGUID() );
        }
        return TRUE;
    }

    // ================================= End old position way =================================

    //=============================
    // Handle the action markers.
    //=============================

    // Add marker button.
    if( I.IsVar( "Cinema\\Add Marker" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add" );
        }
        else
        {
            action_marker& ActionMarker = m_ActionMarkers.Append();
            ActionMarker.Init();
            
            m_nActionMarkers = m_ActionMarkers.GetCount();
#ifdef X_EDITOR
            UpdateEnumString();
#endif
        }
        return TRUE;
    }

    // Marker count.
    if( I.IsVar("Cinema\\Marker Count") )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nActionMarkers );
        }
        else
        {
            m_nActionMarkers = I.GetVarInt( 0 );
            m_ActionMarkers.SetCapacity( m_nActionMarkers );
            m_ActionMarkers.SetCount( m_nActionMarkers );
        }
        return TRUE;
    }

    if( I.IsSimilarPath( "Cinema\\Marker" ) && !I.IsSimilarPath( "Cinema\\Markers" ) )
    {
        s32 i = I.GetIndex(0);

        if ( I.IsVar( "Cinema\\Marker[]" ) )
        {
#ifdef X_EDITOR
            if( I.IsRead() )
            {
                xstring     Label;
                const char* pName;
                if( m_ActionMarkers[i].MarkerType == BLOCKING )
                {
                    Label.Format( "%06.3f Blocking [%s]", m_ActionMarkers[i].Time, m_ActionMarkers[i].Name );
                }
                else
                {
                    object* pObject = m_ActionMarkers[i].ObjectAffecter.GetObjectPtr();
                    if( pObject )
                        pName = pObject->GetName();
                    else
                        pName = "Unknown";
                    Label.Format( "%06.3f Activate [%s]", m_ActionMarkers[i].Time, pName );
                }
                I.SetVarString( (const char*)Label, 64 );
            }
#endif // X_EDITOR
            return TRUE;
        }

        s32 iHeader = I.PushPath( "Cinema\\Marker[]\\" );        
        (void)iHeader;

        // Remove action marker.
        if( I.IsVar( "Remove\\Remove" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarButton( "Remove" );
            }
            else
            {
                m_ActionMarkers.Delete( i );
                m_nActionMarkers = m_ActionMarkers.GetCount();
#ifdef X_EDITOR
                UpdateEnumString();
#endif
            }
            return TRUE;
        }

        // Time.
        if( I.IsVar( "Time" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarFloat( m_ActionMarkers[i].Time );
            }
            else
            {
                m_ActionMarkers[i].Time = I.GetVarFloat( 0.0f, m_AudioLength );
            }
            return TRUE;
        }

        // Marker type.
        if( I.IsVar( "Type" ) )
        {
            if( I.IsRead () )
            {
                switch( m_ActionMarkers[i].MarkerType )
                {
                case BLOCKING:    I.SetVarEnum( "Blocking" );          break;
                case ACTIVATING:  I.SetVarEnum( "Activating" );        break;
                default:          ASSERTS( 0, "Didn't set the type" ); break;
                } 
            }
            else
            {
                if( !x_stricmp( "Blocking", I.GetVarEnum()) )
                {
                    m_ActionMarkers[i].MarkerType = BLOCKING;
                }
                else if( !x_stricmp( "Activating", I.GetVarEnum() ) )
                {
                    m_ActionMarkers[i].MarkerType = ACTIVATING;
                    m_ActionMarkers[i].Name[0]    = 0;
                }
#ifdef X_EDITOR
                UpdateEnumString();
#endif
            }
            return TRUE;
        }

        // Label.
        if( I.IsVar( "Blocking Label" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarString( m_ActionMarkers[i].Name, 64 );
            }
            else
            {
                x_strncpy( m_ActionMarkers[i].Name, I.GetVarString(), 64 );
#ifdef X_EDITOR
                UpdateEnumString();
#endif
            }
            return TRUE;
        }

        // GUID
        if( m_ActionMarkers[i].ObjectAffecter.OnProperty( I, "Marker" ) )
        {
#ifdef X_EDITOR
            UpdateEnumString();
#endif
            return TRUE;
        }
    }

    // ================================= Begin old marker way =================================
    // Remove action marker.
    if( I.IsVar( "Cinema\\Markers\\Remove[]\\Remove" ) )
    {
        s32 i = I.GetIndex(0);

        if( I.IsRead() )
        {
            I.SetVarButton( xfs("Remove[%d]",i) );
        }
        else
        {
            m_ActionMarkers.Delete( i );
            m_nActionMarkers = m_ActionMarkers.GetCount();
#ifdef X_EDITOR
            UpdateEnumString();
#endif
        }
        return TRUE;
    }

    // Add marker button.
    if( I.IsVar( "Cinema\\Markers\\Add Marker" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add" );
        }
        else
        {
            action_marker& ActionMarker = m_ActionMarkers.Append();
            ActionMarker.Init();
            
            m_nActionMarkers = m_ActionMarkers.GetCount();
#ifdef X_EDITOR
            UpdateEnumString();
#endif
        }
        return TRUE;
    }

    // Marker count.
    if( I.IsVar("Cinema\\Markers\\Marker Count") )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nActionMarkers );
        }
        else
        {
            m_nActionMarkers = I.GetVarInt( 0 );
            m_ActionMarkers.SetCapacity( m_nActionMarkers );
            m_ActionMarkers.SetCount( m_nActionMarkers );
        }
    }

    // Time.
    if( I.IsVar( "Cinema\\Markers\\Time[]" ) )
    {
        s32 i = I.GetIndex(0);

        if( I.IsRead() )
        {
            I.SetVarFloat( m_ActionMarkers[i].Time );
        }
        else
        {
            m_ActionMarkers[i].Time = I.GetVarFloat( 0.0f, m_AudioLength );
        }
        return TRUE;
    }

    // Marker type.
    if( I.IsVar( "Cinema\\Markers\\Type[]" ) )
    {
        s32 i = I.GetIndex(0);

        if( I.IsRead () )
        {
            switch( m_ActionMarkers[i].MarkerType )
            {
                case BLOCKING:    I.SetVarEnum( "Blocking" );          break;
                case ACTIVATING:  I.SetVarEnum( "Activating" );        break;
                default:          ASSERTS( 0, "Didn't set the type" ); break;
            } 
        }
        else
        {
            if( !x_stricmp( "Blocking", I.GetVarEnum()) )
            {
                m_ActionMarkers[i].MarkerType = BLOCKING;
            }
            else if( !x_stricmp( "Activating", I.GetVarEnum() ) )
            {
                m_ActionMarkers[i].MarkerType = ACTIVATING;
                m_ActionMarkers[i].Name[0]    = 0;
            }
#ifdef X_EDITOR
            UpdateEnumString();
#endif
        }

        return TRUE;
    }

    // Label.
    if( I.IsVar( "Cinema\\Markers\\Blocking Label[]" ) )
    {
        s32 i = I.GetIndex(0);

        if( I.IsRead() )
        {
            I.SetVarString( m_ActionMarkers[i].Name, 64 );
        }
        else
        {
            x_strncpy( m_ActionMarkers[i].Name, I.GetVarString(), 64 );
#ifdef X_EDITOR
            UpdateEnumString();
#endif
        }
        return TRUE;
    }

    // Activating (via static guid)
    if( I.IsVar( "Cinema\\Markers\\Static GUID to activate[]" ) )
    {
        s32 i = I.GetIndex(0);

        if( !I.IsRead() )
        {
            m_ActionMarkers[i].ObjectAffecter.SetStaticGuid( I.GetVarGUID() );
#ifdef X_EDITOR
            UpdateEnumString();
#endif
        }
    }

    // ================================= End old marker way =================================
    return TRUE;
}

#ifdef X_EDITOR

//==============================================================================

char** cinema_object::GetEnumString( void )
{
    return &m_pEnumString;
}

//==============================================================================

void cinema_object::UpdateEnumString( void )
{
    s32 nBytes = 0;

    // Construct the enum list
    if( m_pEnumString )
    {
        x_free( m_pEnumString );
        m_pEnumString = NULL;
    }

    // Determine total length of string (blocking markers only!).
    for( s32 i=0 ; i< m_nActionMarkers ; i++ )
    {
        if( m_ActionMarkers[i].MarkerType == BLOCKING )
            nBytes += x_strlen( m_ActionMarkers[i].Name ) + 1;
    }

    // Always have end of cinema.
    nBytes += x_strlen( s_pEndOfCinema ) + 1;

    if( nBytes )
    {
        char* pStr;

        // Make space for extra terminating 0.
        nBytes++;

        // Allocate space for the enum string.
        m_pEnumString = pStr = (char*)x_malloc( nBytes );

        // For each marker...
        for( s32 i=0 ; i<m_nActionMarkers ; i++ )
        {
            // Blocking markers only!
            if( m_ActionMarkers[i].MarkerType == BLOCKING )
            {
                // Shove it in.
                x_sprintf( pStr, "%s", m_ActionMarkers[i].Name );

                // Bump past string and terminating 0.
                pStr += x_strlen( m_ActionMarkers[i].Name ) + 1;
            }
        }

        // Shove it in.
        x_sprintf( pStr, "%s", s_pEndOfCinema );

        // Bump past string and terminating 0.
        pStr += x_strlen( s_pEndOfCinema ) + 1;

        // Extra 0 needed.
        *pStr = 0;
    }
}
#endif // X_EDITOR

//==============================================================================

xbool cinema_object::AreAnimsDone( void )
{
    // Check all characters/surfaces to make sure anims have finished
    for( s32 i = 0; i < m_Characters.GetCount(); i++ )
    {
        // Lookup character guid
        guid CharGuid = m_Characters[i].CharacterAffecter.GetGuid();

        // Lookup the object
        object* pObject = g_ObjMgr.GetObjectByGuid( CharGuid );
        if( !pObject )
            continue;
    
        // Lookup actor?
        object_ptr<actor> pActor( CharGuid );
        if( ( pActor ) && ( pActor->IsActive() ) )           
        {
            // Get loco
            loco* pLoco = pActor->GetLocoPointer();
            if( pLoco )
            {
                // Get lip sync animation controller
                loco_lip_sync_controller& LipSyncCont = pLoco->GetLipSyncController();

                // If anim is still playing (which includes blending out), then we are not yet done...
                if( LipSyncCont.IsPlaying() )                
                    return FALSE;
            }
        }
        
        // Does object have a simple anim player?
        simple_anim_player* pAnimPlayer = pObject->GetSimpleAnimPlayer();
        if( pAnimPlayer )
        {
            // At end yet?
            if( pAnimPlayer->IsAtEnd() == FALSE )
                return FALSE;
        }
    }
    
    // All done
    return TRUE;
}

//==============================================================================

void cinema_object::PopCharactersToEndMarkers( void )
{
    // Pop player to end marker?
    object_ptr<object> pMarker( m_PlayerEndMarker.GetGuid() );
    if( pMarker )
    {
        // Lookup marker info
        vector3 MarkerPos = pMarker->GetPosition();
        radian  MarkerYaw = pMarker->GetL2W().GetRotation().Yaw;

        // Lookup the player
        actor* pActor = (actor*)SMP_UTIL_GetActivePlayer();
        if( pActor )
        {
            // Move the actor
            pActor->OnMove( MarkerPos );
            pActor->SetYaw( MarkerYaw );

            // Update actor zone to be markers and reset zone tracking
            pActor->SetZone1( pMarker->GetZone1() );
            pActor->SetZone2( pMarker->GetZone2() );
            pActor->InitZoneTracking();
            
            LOG_MESSAGE( "cinema_object::PopCharactersToEndMarkers", 
                         "Player %s put at end marker %s", 
                         (const char*)guid_ToString( pActor->GetGuid() ),
                         (const char*)guid_ToString( pMarker->GetGuid() ) );
        }
    }

    // Pop characters/anim prop/skin prop surface to end markers
    for( s32 i=0 ; i<m_nCharacters ; i++ )
    {
        cinema_character& CinChar = m_Characters[i];
        object_ptr<actor> pActor( CinChar.CharacterAffecter.GetGuid() ) ;

        // Is it a cinema object?
        if( pActor )
        {
            // Pop to end marker?
            object_ptr<object> pMarker( CinChar.EndMarkerAffecter.GetGuid() );
            if( pMarker )
            {
                // Lookup marker info
                vector3 MarkerPos = pMarker->GetPosition();
                radian  MarkerYaw = pMarker->GetL2W().GetRotation().Yaw;

                // Move the actor
                pActor->OnMove( MarkerPos );
                pActor->SetYaw( MarkerYaw );

                // Update actor zone to be markers and reset zone tracking
                pActor->SetZone1( pMarker->GetZone1() );
                pActor->SetZone2( pMarker->GetZone2() );
                pActor->InitZoneTracking();
                
                LOG_MESSAGE( "cinema_object::PopCharactersToEndMarkers", 
                             "Character %s put at end marker %s", 
                             (const char*)guid_ToString( pActor->GetGuid() ),
                             (const char*)guid_ToString( pMarker->GetGuid() ) );
            }
        }
    }
}

//==============================================================================

void cinema_object::StartAnimAudioTimer( void )
{
    // No camera by default
    guid CameraGuid  = NULL_GUID;
    s32  iCameraBone = -1;
    
    // Search all objects for camera bone that is used by the cinema
    for( s32 i = 0 ; i < m_nCharacters; i++ )
    {
        // Lookup object
        object* pObject = g_ObjMgr.GetObjectByGuid( m_Characters[i].CharacterAffecter.GetGuid() ) ;
        if( !pObject )
            continue;
             
        // Using a simple animation player?
        simple_anim_player* pAnimPlayer = pObject->GetSimpleAnimPlayer();
        if( pAnimPlayer )
        {
            // Search for a camera bone
            iCameraBone = pAnimPlayer->GetBoneIndex( "camera", TRUE );
            if( iCameraBone != -1 )
            {
                // Found!
                CameraGuid = pObject->GetGuid();
                break;
            }
        }            
    }
        
    // Finally, start the timer
    m_Timer.Start( m_VoiceID, CameraGuid, iCameraBone );
}

//==============================================================================

xbool cinema_object::IsPast( char* Marker )
{
    // Special check for end of cinema.
    if( x_strcmp( Marker, s_pEndOfCinema ) == 0 )
    {
        return m_bCinemaDone;
    }
    
    // Process the action markers.
    for( s32 i=0 ; i<m_nActionMarkers ; i++ )
    {
        // Activating marker?
        if( m_ActionMarkers[i].MarkerType == BLOCKING )
        {
            // Matching marker?
            if( x_strcmp( Marker, m_ActionMarkers[i].Name ) == 0 )
            {
                // Past this marker? (or done!)
                return m_bCinemaDone || (m_ActionMarkers[i].Time <= m_CurrTime);
            }
        }
    }

    return TRUE;
}

//==============================================================================

void cinema_object::ProcessMarkers( void )
{
    s32 ActiveMarker;
    f32 BestTime;

    // Process the position markers.
    ActiveMarker = -1;
    BestTime     = -1.0f;
    if( !m_bIs2D )
    {
        for( s32 i=0 ; i<m_nPositionMarkers ; i++ )
        {
            f32 Time = m_PositionMarkers[i].Time;

            // This marker activate this frame?
            if( (Time >= m_PrevTime) && (Time < m_CurrTime) )
            {
                // Most recent?
                if( Time >= BestTime )
                {
                    // Mark the most recent position marker.
                    ActiveMarker = i;
                    BestTime     = Time;
                }
            }
        }

        // Active marker found?
        if( ActiveMarker != -1 )
        {
            LOG_MESSAGE( "cinema_object::ProcessMarkers",
                        "ActiveMarker: %d", 
                        ActiveMarker );

            m_PositionMarkerIndex = ActiveMarker;
        }

        if( m_PositionMarkerIndex != -1 )
        {
            // Find the object.   
            object *pObject = m_PositionMarkers[m_PositionMarkerIndex].ObjectAffecter.GetObjectPtr();
            if( pObject )
            {
                // Get objects position.
                vector3 Pos = pObject->GetPosition();

                // Set the voices position.
                g_AudioMgr.SetPosition( m_VoiceID, Pos, pObject->GetZone1() );

                LOG_MESSAGE( "cinema_object::ProcessMarkers", 
                            "SetPosition (%f,%f,%f)", 
                            Pos.GetX(),
                            Pos.GetY(),
                            Pos.GetZ() );
            }
        }
    }

    // Process the action markers.
    for( s32 i=0 ; i<m_nActionMarkers ; i++ )
    {
        // This marker activate this tick?
        if( (m_ActionMarkers[i].Time > m_PrevTime) && (m_ActionMarkers[i].Time <= m_CurrTime) )
        {
            // Activating marker?
            if( m_ActionMarkers[i].MarkerType == ACTIVATING )
            {
                // Get the object to activate
                object *pObject = m_ActionMarkers[i].ObjectAffecter.GetObjectPtr();
                if( pObject )
                {
                    // If its not active, activate it.
                    if( !pObject->IsActive() )
                        pObject->OnActivate( TRUE );
                }
            }
        }
    }
}

//==============================================================================

void cinema_object::ResumeCharacterAi( cinema_character& CinChar )
{
    // Lookup actor and exit if already resumed
    object_ptr<actor> pActor( CinChar.CharacterAffecter.GetGuid() ) ;
    if( !pActor )
        return;

    // Lookup loco
    loco* pLoco = pActor->GetLocoPointer();
    if( !pLoco )
        return;

    // Was it a full body cinema anim?
    xbool bFullBody =     ( CinChar.AnimFlags & loco::ANIM_FLAG_MASK_TYPE_FULL_BODY )
                       || ( ( CinChar.AnimFlags & loco::ANIM_FLAG_MASK_TYPE_ALL ) == 0 );
                
    // Reset lip sync controller to continue
    loco_lip_sync_controller& LipSyncCont = pLoco->GetLipSyncController();
    LipSyncCont.SetRate( 1.0f );
    
    // Resume previous animation?
    if( CinChar.NextAiState == character_state::STATE_NULL )
    {
        // Only re-start base anim if cinema anim was full body
        if( bFullBody )
        {
            pLoco->m_Player.SetAnim( CinChar.hNextAnimGroup, CinChar.iNextAnim, 0.25f );
         
#if defined(X_LOGGING) && !defined(X_SUPPRESS_LOGS)
            // Log anim
            const anim_group* pAnimGroup = CinChar.hNextAnimGroup.GetPointer();
            if( pAnimGroup )
            {
                const anim_info& AnimInfo  = pAnimGroup->GetAnimInfo( CinChar.iNextAnim );
                const char*      pAnimName = AnimInfo.GetName();
                
                LOG_MESSAGE( "cinema_object::ResumeCharacterAi", 
                             "Character %s anim %s resumed", 
                             (const char*)guid_ToString( pActor->GetGuid() ),
                             pAnimName );
            }
#endif            
        }                        
    }

    // Just do this once
    CinChar.CharacterAffecter.SetStaticGuid( 0 );
}

//==============================================================================

// Checks for when anims end and resumes that characters Ai
void cinema_object::ProcessAnims ( void )
{
    // Loop through characters
    for( s32 i = 0; i < m_nCharacters; i++ )
    {
        // Lookup actor
        cinema_character& CinChar = m_Characters[i];
        object_ptr<actor> pActor( CinChar.CharacterAffecter.GetGuid() ) ;

        // Present?
        if( !pActor )
            continue;
                 
        // Get loco?
        loco* pLoco = pActor->GetLocoPointer();
        if( !pLoco )
            continue;
            
        // Playing full body lip sync and finished or blending out?
        loco_lip_sync_controller& LipSyncCont = pLoco->GetLipSyncController();
        if( LipSyncCont.IsFullBody() )
        {
            // Is anim finished or about to be?
            if(    ( LipSyncCont.IsPlaying() == FALSE )
                || ( LipSyncCont.IsBlendingOut() )
                || ( LipSyncCont.IsPlaying() && LipSyncCont.IsAtEnd() ) ) 
            {
                // Resume Ai
                ResumeCharacterAi( CinChar );
            }
        }
    }        
}

//==============================================================================

void cinema_object::StartAnims( void )
{
    // Loop through characters
    for( s32 i=0 ; i<m_nCharacters ; i++ )
    {
        // Lookup info
        cinema_character& CinChar  = m_Characters[i];
        guid              CharGuid = CinChar.CharacterAffecter.GetGuid();

        // Lookup object
        object* pObject = g_ObjMgr.GetObjectByGuid( CharGuid );
        if( !pObject )
            continue;
            
        // Make sure object is active            
        if( !pObject->IsActive() )
            pObject->OnActivate( TRUE );

        // Start objects simple anim player?
        simple_anim_player* pAnimPlayer = pObject->GetSimpleAnimPlayer();
        if( pAnimPlayer )
        {
            // Setup anim player ready for being driven by audio elapsed time
            pAnimPlayer->SetRate( 0.0f );

            // Override default anim?
            if( ( CinChar.hAnimGroup.GetPointer() ) && ( CinChar.AnimName != -1 ) )
            {
                // Set non-looping anim
                pAnimPlayer->SetAnimGroup( CinChar.hAnimGroup );
                pAnimPlayer->SetAnim( g_StringMgr.GetString( CinChar.AnimName ), FALSE );
            }
        }

        // Is it an actor?
        object_ptr<actor> pActor( CharGuid );
        if( pActor )
        {
            // Setup default relative info to be where npc currently is
            vector3 RelPos( pActor->GetPosition() );
            radian  RelYaw( pActor->GetYaw() );
        
            // Pop to marker?
            object_ptr<object> pMarker( CinChar.StartMarkerAffecter.GetGuid() );
            if( pMarker )
            {
                // Lookup marker info
                vector3 MarkerPos = pMarker->GetPosition();
                radian  MarkerYaw = pMarker->GetL2W().GetRotation().Yaw ;
                
                // Pop position?
                f32 DistSqr = ( MarkerPos - pActor->GetPosition() ).LengthSquared();
                if( DistSqr > x_sqr( CinChar.StartMarkerDistance ) )
                {
                    // Use for relative pos
                    RelPos = MarkerPos;
                
                    // Move the actor
                    pActor->OnMove( MarkerPos );
                    
                    // Update actor zone to be markers and reset zone tracking
                    pActor->SetZone1( pMarker->GetZone1() );
                    pActor->SetZone2( pMarker->GetZone2() );
                    pActor->InitZoneTracking();
                    
                    LOG_MESSAGE( "cinema_object::StartAnims", 
                                 "Character %s pos set to start marker %s pos", 
                                 (const char*)guid_ToString( pActor->GetGuid() ),
                                 (const char*)guid_ToString( pMarker->GetGuid() ) );
                }
                
                // Pop yaw?
                if( x_abs( x_MinAngleDiff( MarkerYaw, pActor->GetYaw() ) ) > CinChar.StartMarkerYaw )
                {
                    // Use for relative yaw
                    RelYaw = MarkerYaw;
                
                    // Update actor
                    pActor->SetYaw( MarkerYaw );
                    
                    LOG_MESSAGE( "cinema_object::StartAnims", 
                                 "Character %s yaw set to start marker %s yaw", 
                                 (const char*)guid_ToString( pActor->GetGuid() ),
                                 (const char*)guid_ToString( pMarker->GetGuid() ) );
                }
            }
            
            // Start the lip sync anim
            loco* pLoco = pActor->GetLocoPointer();
            if( pLoco )
            {
                // Store current animation so we can resume back to this once lip sync is done
                CinChar.hNextAnimGroup = pLoco->m_Player.GetCurrAnim().GetAnimGroupHandle();
                CinChar.iNextAnim      = pLoco->m_Player.GetCurrAnim().GetAnimIndex();
                ASSERT( CinChar.iNextAnim != -1 );
            
                // Turn off movement accumulation so start pos/yaw is consistent
                loco_motion_controller& CurrAnim  = pLoco->m_Player.GetCurrAnim();
                loco_motion_controller& BlendAnim = pLoco->m_Player.GetBlendAnim();
                CurrAnim.SetAccumHorizMotion ( FALSE );
                CurrAnim.SetAccumYawMotion   ( FALSE );
                BlendAnim.SetAccumHorizMotion( FALSE );
                BlendAnim.SetAccumYawMotion  ( FALSE );
                
                // Start the lip sync animation going
                const char* pAnimName = g_StringMgr.GetString( CinChar.AnimName );                
                pLoco->PlayLipSyncAnim( CinChar.hAnimGroup,
                                        pAnimName,
                                        m_VoiceID, 
                                        CinChar.AnimFlags | loco::ANIM_FLAG_INTERRUPT_BLEND | loco::ANIM_FLAG_CINEMA );
                                        
                // Setup relative info for full body anims
                if( CinChar.AnimFlags & loco::ANIM_FLAG_MASK_TYPE_FULL_BODY )
                {                
                    // Setup ready for relative mode
                    pLoco->SetCinemaRelativeInfo( RelPos, RelYaw );
                    
                    // Make sure accumulate horiz and yaw are turned on for full body anims!
                    loco_lip_sync_controller& LipSyncAnim = pLoco->GetLipSyncController();
                    LipSyncAnim.SetRemoveYawMotion( TRUE );
                    LipSyncAnim.SetRemoveHorizMotion( TRUE );
                    LipSyncAnim.SetAccumYawMotion( TRUE );
                    LipSyncAnim.SetAccumHorizMotion( TRUE );
                }
                                
                LOG_MESSAGE( "cinema_object::StartAnims", 
                             "Character %s anim %s started", 
                             (const char*)guid_ToString( pActor->GetGuid() ), 
                             pAnimName );
            }
            
            // Setup next ai state
            if( pActor->IsKindOf(character::GetRTTI()) )
            {
                character& CinCharacter = character::GetSafeType( *pActor );
                CinCharacter.SetPostLipSyncState( (character_state::states)CinChar.NextAiState );
            }
        }
    }
}

//==============================================================================

void cinema_object::UpdateAnims( void )
{
    // Loop through characters
    for( s32 i=0; i < m_nCharacters; i++ )
    {
        // Lookup character
        cinema_character& CinChar  = m_Characters[i];
        guid              CharGuid = CinChar.CharacterAffecter.GetGuid();
        
        // Lookup the object
        object* pObject = g_ObjMgr.GetObjectByGuid( CharGuid );
        if( !pObject )
            continue;
        
        // Lookup anim group
        const anim_group* pAnimGroup = CinChar.hAnimGroup.GetPointer();
        if( !pAnimGroup )
            continue;
            
        // Lookup anim index
        if( CinChar.AnimName == -1 )
            continue;
        
        // Is this an actor?
        object_ptr<actor> pActor( CharGuid );
        if( pActor )
        {
            // Lookup loco
            loco* pLoco = pActor->GetLocoPointer();
            if( pLoco )
            {
                // Update lip sync controller
                loco_lip_sync_controller& LipSyncCont = pLoco->GetLipSyncController();
                LipSyncCont.SetTime( m_CurrTime );
            }
        }
                        
        // Update objects simple anim player?
        simple_anim_player* pAnimPlayer = pObject->GetSimpleAnimPlayer();
        if( pAnimPlayer )
        {
            // Update anim player
            if( ( m_AudioStatus == AUDIO_STATUS_OFF ) || ( m_AudioStatus == AUDIO_STATUS_DONE ) )
            {
                // Let anim continue on its own
                pAnimPlayer->SetRate( 1.0f );
            }
            else
            {
                // Update anim position
                if( pAnimPlayer->GetAnimIndex() != -1 )
                    pAnimPlayer->SetTime( m_CurrTime );
            }
        }
    }
}

//==============================================================================

void cinema_object::StopAnims( void )
{
    // Loop through all characters
    for( s32 i=0 ; i<m_nCharacters ; i++ )
    {
        // Lookup character info
        cinema_character& CinChar  = m_Characters[i];
        guid              CharGuid = CinChar.CharacterAffecter.GetGuid();

        // Lookup the object
        object* pObject = g_ObjMgr.GetObjectByGuid( CharGuid );
        if( !pObject )
            continue;

        // Stop simple anim player?
        simple_anim_player* pAnimPlayer = pObject->GetSimpleAnimPlayer();
        if( pAnimPlayer )
        {
            // Let the anim player resume incase it is activated again
            pAnimPlayer->SetRate( 1.0f );
        
            // Deactivate the object
            pObject->OnActivate( FALSE );
        }
        
        // Is it an actor?
        object_ptr<actor> pActor( CharGuid ) ;
        if( pActor )
        {
            // Stop the lip sync controller
            loco* pLoco = pActor->GetLocoPointer();
            if( pLoco )
            {
                pLoco->GetLipSyncController().Clear();
            }
                                
            // Make sure Ai is resumed
            ResumeCharacterAi( CinChar );

            LOG_MESSAGE( "cinema_object::StopAnims", 
                "Character %s anim stopped/resumed", 
                (const char*)guid_ToString( pActor->GetGuid() ) );
        }
            
        // Just do this once
        CinChar.CharacterAffecter.SetStaticGuid( 0 );
    }
}

//==============================================================================

void cinema_object::OnAdvanceLogic( f32 DeltaTime )
{
    (void)DeltaTime;

    // Should only get in here if active
    ASSERT( m_bCinemaActive );

    //==========================================================================
    // Update audio
    //==========================================================================
    switch( m_AudioStatus )
    {
    default:
        ASSERTS( 0, "MISSING STATE!" );
        break;                    
    
    case AUDIO_STATUS_OFF:

        // Warm up the audio
        if( m_bIs2D )
        {
            m_VoiceID = g_ConverseMgr.PlayStream( m_Descriptor, GetPosition(), GetGuid(), GetZone1(), 0.0f, FALSE, PLAY_2D );
        }
        else
        {
            m_VoiceID = g_ConverseMgr.PlayStream( m_Descriptor, GetPosition(), GetGuid(), GetZone1(), 0.0f, FALSE );
        }
        if( g_AudioMgr.IsValidVoiceId( m_VoiceID ) )
        {
            LOG_MESSAGE( "cinema_object::OnAdvanceLogic", "AUDIO_STATUS_WARMING" );
            m_AudioStatus = AUDIO_STATUS_WARMING;
            StartAnimAudioTimer();
        }                
        break;

    case AUDIO_STATUS_WARMING:

        // Ready to start?
        if( g_AudioMgr.GetIsReady( m_VoiceID ) )
        {
            g_AudioMgr.Start( m_VoiceID );
            LOG_MESSAGE( "cinema_object::OnAdvanceLogic", "AUDIO_STATUS_PLAYING" );
            m_AudioStatus = AUDIO_STATUS_PLAYING;
        }
        break;

    case AUDIO_STATUS_PLAYING:
        
        // Finished?
        if( !g_AudioMgr.IsValidVoiceId(m_VoiceID) )
        {
            // Stop the audio
            g_ConverseMgr.Stop( m_VoiceID );
            m_VoiceID = 0;
            
            LOG_MESSAGE( "cinema_object::OnAdvanceLogic", "AUDIO_STATUS_DONE" );
            m_AudioStatus = AUDIO_STATUS_DONE;
            
            // Stop anim audio timer
            m_Timer.Stop();
        }
        
        // Update the current time from the anim audio timer
        m_CurrTime = m_Timer.GetTime();
        break;
        
    case AUDIO_STATUS_DONE:
    
        // Keep timer going incase audio has ended before the animation has finished
        m_CurrTime += DeltaTime;
        break;
    }
    
    //==========================================================================
    // Update animations
    //==========================================================================

    // Update state
    switch( m_AnimStatus )
    {
    default:
        ASSERTS( 0, "MISSING STATE!" );
        break;                    
    
    case ANIM_STATUS_OFF:

        // Wait for audio to start
        if( m_AudioStatus == AUDIO_STATUS_PLAYING )                   
        {
            // Start anims and pop actors to markers if needed
            StartAnims();
            LOG_MESSAGE( "cinema_object::OnAdvanceLogic", "ANIM_STATUS_PLAYING" );
            m_AnimStatus = ANIM_STATUS_PLAYING;
        }
        break;
    
    case ANIM_STATUS_PLAYING:

        // Have all animations finished?
        if( AreAnimsDone() )
        {
            LOG_MESSAGE( "cinema_object::OnAdvanceLogic", "ANIM_STATUS_DONE" );
            m_AnimStatus = ANIM_STATUS_DONE;
        }

        // Fall through to process characters so their Ai can be resumed when anims finish
    
    case ANIM_STATUS_DONE:
        
        // Checks for anims ending so that character ai can be resumed
        ProcessAnims();
        break;
    }

    // Update anim audio timer
    m_Timer.Advance( DeltaTime );

    // Update anim surface and skin prop animations
    UpdateAnims();

    //==========================================================================
    // Process markers
    //==========================================================================

    // Audio or anims running?
    if(     ( m_AudioStatus == AUDIO_STATUS_PLAYING )
        ||  ( m_AnimStatus  == ANIM_STATUS_PLAYING ) )
    {
        LOG_MESSAGE( "cinema_object::OnAdvanceLogic", "m_PrevTime: %f, m_CurrTime: %f", m_PrevTime, m_CurrTime );
        // Process markers
        ProcessMarkers();
    }

    // Update previous.
    m_PrevTime = m_CurrTime;
    
    
    //==========================================================================
    // End logic
    //==========================================================================
    
    // Both audio and anim done?
    if( ( m_AudioStatus == AUDIO_STATUS_DONE ) && ( m_AnimStatus == ANIM_STATUS_DONE ) )
    {
        // Deactivate
        OnActivate( FALSE );
    }
}

//==============================================================================

void cinema_object::OnActivate( xbool Flag )
{
    // Turn on/off logic
    object::OnActivate( Flag );
    
    // Starting the cinema?
    if( Flag )
    {
        // Ending/skipping the cinema...
        LOG_MESSAGE( "cinema_object::OnActivate", 
                     "Cinema %s started!", 
                     (const char*)guid_ToString( GetGuid() ) );
    
        // Clear done flag
        m_bCinemaDone = FALSE;

        // If position_marker[0] is the player, then the cinema is goinf to be 2D.
        if( m_nPositionMarkers == 1 )
        {
            object *pObject = m_PositionMarkers[0].ObjectAffecter.GetObjectPtr();
            if( pObject == (object*)SMP_UTIL_GetActivePlayer() )
            {
                LOG_MESSAGE( "cinema_object::OnActivate", "2D Cinema!!!!" );
                m_bIs2D = TRUE;
            }
        }
        
        // Status should be off
        ASSERT( m_AudioStatus == AUDIO_STATUS_OFF );
        ASSERT( m_AnimStatus  == ANIM_STATUS_OFF );
    }
    else
    {
        // Ending or skipping an active cinema?
        if( m_bCinemaActive )
        {
            // Put audio into off state
            g_ConverseMgr.Stop( m_VoiceID );
            m_VoiceID = 0;
            m_AudioStatus = AUDIO_STATUS_OFF;
            LOG_MESSAGE( "cinema_object::OnActivate", "AUDIO_STATUS_OFF" );

            // Put anims into off state
            StopAnims();
            m_AnimStatus = ANIM_STATUS_OFF;
            LOG_MESSAGE( "cinema_object::OnActivate", "ANIM_STATUS_OFF" );
            
            // Position characters at their end markers                    
            PopCharactersToEndMarkers();

            // Finally, flag cinema is done so blocking stops and game continues...
            m_bCinemaDone = TRUE;
            
            LOG_MESSAGE( "cinema_object::OnActivate", 
                         "Cinema %s done!", 
                         (const char*)guid_ToString( GetGuid() ) );
        }
    }
    
    // Store flag
    m_bCinemaActive = Flag;
}

//==============================================================================

#ifdef X_EDITOR

s32 cinema_object::OnValidateProperties( xstring& ErrorMsg )
{
    s32 i;
    
    s32 nErrors = 0;

    // Audio package not found?
    const char* pPackage = m_AudioPackage.GetName();
    if( ( !pPackage ) || ( pPackage[0] == 0 ) || ( x_stricmp( pPackage, "<null>" ) == 0 ) )
    {
        ErrorMsg += "No audio package assigned.\n";
        return 1;
    }        
    
    // Audio package not loaded?
    if( m_AudioPackage.GetPointer() == NULL )
    {
        ErrorMsg += "Audio package [" + xstring( pPackage ) + "] not loaded.\n";
        return 1;
    }        

    // Invalid descriptor?
    if( g_AudioMgr.IsValidDescriptor( m_Descriptor ) == FALSE )
    {
        ErrorMsg += "Sound [" + xstring( m_Descriptor ) + "] does not exist.\n";
        return 1;
    }

    // Get length of audio
    f32 AudioLength = g_AudioMgr.GetLengthSeconds( m_Descriptor );
    if( AudioLength == 0.0f )
    {
        ErrorMsg += "Sound [" + xstring( m_Descriptor ) + "] has zero length.\n";
        return 1;
    }
    
    // Loop over all characters and check for errors
    xarray<xstring> CinCharErrorMsgs;
    CinCharErrorMsgs.SetCount( m_Characters.GetCount() );
    for( i = 0; i < m_Characters.GetCount(); i++ )
    {
        // Lookup character and error string
        cinema_character& CinChar         = m_Characters[i];
        xstring&          CinCharErrorMsg = CinCharErrorMsgs[i];
        s32               nCharErrors     = 0;

        // Check character guid            
        nCharErrors += object::OnValidateObject( xstring( "Character guid" ), &CinChar.CharacterAffecter, CinCharErrorMsg );
        
        // Check start marker guid
        if( CinChar.StartMarkerAffecter.GetGuid() != 0 )
            nCharErrors += object::OnValidateObject( xstring( "Start Marker guid" ), &CinChar.StartMarkerAffecter, CinCharErrorMsg );
        
        // Check end marker guid
        if( CinChar.EndMarkerAffecter.GetGuid() != 0 )
            nCharErrors += object::OnValidateObject( xstring( "End Marker guid" ), &CinChar.EndMarkerAffecter, CinCharErrorMsg );

        // Make sure character is a valid type
        object* pObject = g_ObjMgr.GetObjectByGuid( CinChar.CharacterAffecter.GetGuid() );
        if( pObject )
        {
            // Is object the wrong type?        
            if(     ( pObject->IsKindOf( actor::GetRTTI() ) == FALSE )
                &&  ( pObject->GetSimpleAnimPlayer() == NULL ) ) 
            {         
                CinCharErrorMsg += "ERROR: Object " + xstring( CinChar.CharacterAffecter.GetObjectInfo() );
                CinCharErrorMsg += " is not a valid actor, anim-surface, or skin-prop-surface - it MUST be one of these types!\n\n";
                nCharErrors++;
            }
        }
        
        // Check anim
        nCharErrors += object::OnValidateAnim( xstring( "Character anim" ), &CinChar.AnimGroupName, CinChar.AnimName, CinCharErrorMsg );

        // Validate anim
        const anim_group* pAnimGroup = CinChar.hAnimGroup.GetPointer();
        if( pAnimGroup )        
        {
            // Look for anim
            s32 iAnim = pAnimGroup->GetAnimIndex( g_StringMgr.GetString( CinChar.AnimName ) );   
            if( iAnim != -1 )
            {
                // Lookup anim info
                const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( iAnim );            
                const char* pName       = AnimInfo.GetName(); 
                s32         nFrames     = AnimInfo.GetNFrames();
                s32         FPS         = AnimInfo.GetFPS();
                f32         AnimLength  = (f32)nFrames / (f32)FPS;

                // Make sure motion prop exists for character animations
                if( ( pObject ) && ( pObject->IsKindOf( actor::GetRTTI() ) ) )
                {
                    // Lookup motion prop
                    s32 iMotionProp = AnimInfo.GetPropChannel( "MotionProp" );
                    if( iMotionProp == -1 )
                    {
                        // Not found, report error
                        CinCharErrorMsg += "ERROR: Motion prop not found in actor animation!!\n";
                        CinCharErrorMsg += "Please check your .matx and .max file to verify you\n";
                        CinCharErrorMsg += "have created and exported a valid motion prop!\n";
                        CinCharErrorMsg += "(this animation will play incorrectly until you fix this)\n\n";
                        nCharErrors++;
                    }
                    else if( CinChar.AnimFlags & loco::ANIM_FLAG_MASK_TYPE_FULL_BODY )
                    {
                        // Is a start marker defined?
                        if( CinChar.StartMarkerAffecter.GetGuid() )
                        {
/*                        
                            // Make sure accumulate horiz flag is set to TRUE
                            if( AnimInfo.AccumHorizMotion() == FALSE )
                            {
                                CinCharErrorMsg += "ERROR: Accumulate horizontal flag should be set to TRUE.\n";
                                CinCharErrorMsg += "Please edit and fix the flag in the anim resource.\n\n";
                                nCharErrors++;
                            }
                            
                            // Make sure accumulate yaw flag is set to TRUE
                            if( AnimInfo.AccumYawMotion() == FALSE )
                            {
                                CinCharErrorMsg += "ERROR: Accumulate yaw flag should be set to TRUE.\n";
                                CinCharErrorMsg += "Please edit and fix the flag in the anim resource.\n\n";
                                nCharErrors++;
                            }
*/
                                
                            // Make sure motion prop is at the origin
                            anim_key FirstKey;
                            AnimInfo.GetPropRawKey( iMotionProp, 0, FirstKey );
                            f32 DistSqr = FirstKey.Translation.LengthSquared();
                            if( DistSqr > x_sqr( 10.0f ) )
                            {
                                CinCharErrorMsg += "ERROR: Motion prop MUST start at the origin.\n\n";
                                nCharErrors++;
                            }
                        }
                    }
                }
                                
                // Make sure "blend frames" is set to true since the cinema object now takes care of this
                if( AnimInfo.BlendFrames() == FALSE )
                {
                    // Report error
                    CinCharErrorMsg += "ERROR: Animation property \"BlendFrames\" property is FALSE when it should be TRUE!\n";
                    CinCharErrorMsg += "The new cinema system now handles frame interpolation automatically to give smooth\n";
                    CinCharErrorMsg += "playback. It will turn frame blending off during camera cuts which it auto-detects.\n";
                    CinCharErrorMsg += "Edit the anim package resource and set the property to FALSE otherwise you'll have\n";
                    CinCharErrorMsg += "a jerky cinema on your hands!!.\n\n";
                    nCharErrors++;
                }
                
                // Is there a lip sync start event there when there shouldn't be?
                s32 LipSyncStartFrame = (s32)AnimInfo.FindLipSyncEventStartFrame();
                if( LipSyncStartFrame != -1 )
                {                
                    CinCharErrorMsg += "ERROR: Lip-sync start event found on frame ";
                    CinCharErrorMsg += xfs( "%d", LipSyncStartFrame );
                    CinCharErrorMsg += " - NOT supported by cinema and will break playback!\n\n";
                    nCharErrors++;
                }
                                        
                // Add info?
                if( nCharErrors )
                {
                    xstring HeaderMsg;
                    HeaderMsg += "\n\n";
                    HeaderMsg += "******* BEGIN CINEMA CHARACTER ERRORS! *******\n\n";
                    HeaderMsg += " Character " + xstring( CinChar.CharacterAffecter.GetObjectInfo() ) + "\n";
                    HeaderMsg += "  AnimGroup [" + xstring( g_StringMgr.GetString( CinChar.AnimGroupName ) ) + "]\n";
                    HeaderMsg += "  AnimName [" + xstring( pName ) + "]\n";
                    HeaderMsg += xfs( "  Length:%.3f secs\n", AnimLength );
                    HeaderMsg += xfs( "  Frames:%d\n",  nFrames );
                    HeaderMsg += xfs( "  FPS:%d\n\n", FPS );
                    
                    // Put infront 
                    CinCharErrorMsg = HeaderMsg + CinCharErrorMsg;
                    CinCharErrorMsg += "******* END CINEMA CHARACTER ERRORS! *******\n\n";
                }    
                
                // Accumulate errors
                nErrors += nCharErrors;                                    
            }
        }           
    }
        
    // Any errors?
    if( nErrors )
    {
        // Show audio info
        ErrorMsg += "AUDIO:\n";
        ErrorMsg += " Package [" + xstring( pPackage ) + "]\n";
        ErrorMsg += " Sound [" + xstring( m_Descriptor ) + "]\n";
        ErrorMsg += " Length:" + xstring( xfs( "%.3f secs", AudioLength ) ) + "\n";
        ErrorMsg += " Frames:" + xstring( xfs( "%d", (s32)(AudioLength*30.0f) ) ) + " (at 30FPS)\n";
        ErrorMsg += "\n";

        // Add all character info
        for( i = 0; i < CinCharErrorMsgs.GetCount(); i++ )
            ErrorMsg += CinCharErrorMsgs[i];
    }
        
    return nErrors;
}

//==============================================================================

void cinema_object::OnDebugRender( void )
{
    // Loop through characters
    for( s32 i=0 ; i<m_nCharacters ; i++ )
    {
        // Lookup marker
        cinema_character& CinChar = m_Characters[i];
        object_ptr<object> pMarker( CinChar.StartMarkerAffecter.GetGuid() );
        if( pMarker )
        {
            // Lookup marker info
            vector3 MarkerPos = pMarker->GetPosition();
            radian  MarkerYaw = pMarker->GetL2W().GetRotation().Yaw;
            
            // Draw info
            draw_3DCircle  ( MarkerPos, CinChar.StartMarkerDistance, XCOLOR_RED );
            draw_Arc       ( MarkerPos, CinChar.StartMarkerDistance, MarkerYaw, CinChar.StartMarkerYaw * 0.5f, XCOLOR_PURPLE );
        }
    }        
}

//==============================================================================

#endif  //#ifdef X_EDITOR


