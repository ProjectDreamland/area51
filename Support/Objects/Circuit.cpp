//==============================================================================
//
//  Circuit.cpp
//
//==============================================================================
/*
EXPORT  SAVE    SHOW
Circuit\\Init(0)                                yes     no      no  
Circuit\\DM             = 02 - OMEGA            no      yes     no
Circuit\\TDM            = 05 - NONE             no      yes     no
Circuit\\+DM+Tag+INF+   = 00 - NONE             no      no      yes

*/
//==============================================================================
//  INCLUDES
//==============================================================================

#include "Circuit.hpp"
#include "MP_Settings.hpp"
#include "NetworkMgr\GameMgr.hpp"

//==============================================================================
//  FUNCTIONS
//==============================================================================

circuit::circuit( void )
{
    m_CircuitInit[0] = 0;
    m_CircuitInit[1] = 0;
    m_Circuit        = 0;
}

//==============================================================================

circuit::~circuit( void )
{
}

//==============================================================================

u32 circuit::GetTeamBits( void ) const
{
#ifdef X_EDITOR

    s32 C = -1;
    for( s32 t = 0; t < mp_settings::s_NGameTypes; t++ )
    {
        if( mp_settings::s_GameTypeBits & (1<<t) )
        {
            s32 Shift = (t<8) ? t*4 : (t-8)*4;
            s32 Index = (t<8) ?   0 :       1;
            s32 V = (m_CircuitInit[Index] >> Shift) & 0x0F;
            if( C == -1 )   C = V;
            if( C !=  V )   C = 16;
        }
    }
    if( C == -1 )
        C = 0;

    return( mp_settings::GetTeamBits( C ) );

#else

    return( GameMgr.GetCircuitBits( m_Circuit ) );

#endif
}

//==============================================================================

void circuit::SetCircuit( s32 Circuit )
{
#ifdef X_EDITOR
    ASSERT( IN_RANGE( 0, Circuit, 3 ) );

    // In the editor, this function supports legacy properties.  Set the given
    // circuit for all game types.
    u32 Bits = 0;
    switch( Circuit )
    {
    case 0:     Bits = 0x00000000;  break;
    case 1:     Bits = 0x00000001;  break;
    case 2:     Bits = 0x00000002;  break;
    case 3:     Bits = 0x0000000F;  break;
    }
    m_CircuitInit[0] = 0;
    m_CircuitInit[1] = 0;
    while( Bits )
    {
        m_CircuitInit[0] |= Bits;
        m_CircuitInit[1] |= Bits;
        Bits <<= 4;
    }
#else
    ASSERT( IN_RANGE( 0, Circuit, 15 ) );
    
    // This is called by the GameMgr when it is sending initial circuit values
    // over the wire.  Set the circuit and be done.
    m_Circuit = Circuit;
#endif
}

//==============================================================================

s32 circuit::GetCircuit( void ) const
{
#ifdef X_EDITOR
    ASSERT( FALSE );
#endif

    return( m_Circuit );
}

//==============================================================================

void circuit::OnEnumProp( prop_enum& List )
{   
    List.PropEnumString( "Circuit", 
                         "Circuits for each game type.", 
                         PROP_TYPE_HEADER | 
                         PROP_TYPE_DONT_SAVE | 
                         PROP_TYPE_DONT_EXPORT | 
                         PROP_TYPE_DONT_SAVE_MEMCARD );

    u32 Flags = PROP_TYPE_DONT_SHOW | PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_SAVE_MEMCARD;
    List.PropEnumInt( "Circuit\\Init(0)", "Startup values.", Flags );
    List.PropEnumInt( "Circuit\\Init(1)", "Startup values.", Flags );

#ifdef X_EDITOR
    for( s32 t = 0; t < mp_settings::s_NGameTypes; t++ )
    {
        List.PropEnumEnum( xfs( "Circuit\\%s", mp_settings::s_GameTypeAbbr[t] ),
                           "\0\0", "",
                           PROP_TYPE_DONT_SHOW | 
                           PROP_TYPE_DONT_EXPORT | 
                           PROP_TYPE_DONT_SAVE_MEMCARD );
    }

    if( mp_settings::s_GameTypeBits )
    {
        List.PropEnumEnum( "Circuit\\Select",
                           mp_settings::GetCircuitEnum(), "",
                           PROP_TYPE_DONT_SAVE | 
                           PROP_TYPE_DONT_EXPORT | 
                           PROP_TYPE_DONT_SAVE_MEMCARD );
    }
#endif
}

//==============================================================================

xbool circuit::OnProperty( prop_query& Query )
{
    if( Query.VarInt( "Circuit\\Init(0)", m_CircuitInit[0] ) ||
        Query.VarInt( "Circuit\\Init(1)", m_CircuitInit[1] ) )
    {
#ifndef X_EDITOR
        s32 GType = GameMgr.GetGameType();
        s32 Shift = (GType<8) ? GType*4 : (GType-8)*4;
        s32 Index = (GType<8) ?       0 :           1;
        m_Circuit = (m_CircuitInit[Index] >> Shift) & 0x0F;
#endif X_EDITOR

        return( TRUE );
    }

#ifdef X_EDITOR

    if( Query.IsVar( "Circuit" ) )
    {
        if( Query.IsRead() )
        {
            Query.SetVarString( mp_settings::GetActiveAbbr(), 128 );
        }
        return( TRUE );
    }

    for( s32 t = 0; t < mp_settings::s_NGameTypes; t++ )
    {   
        if( Query.IsVar( xfs( "Circuit\\%s", mp_settings::s_GameTypeAbbr[t] ) ) )
        {
            // Bake up a couple of utility values to extract/encode the circuit
            // value from/to the initial value store.
            s32 Shift = (t<8) ? t*4 : (t-8)*4;
            s32 Index = (t<8) ?   0 :       1;

            // The format of the string is as follows:
            // %02d
            // using the circuit number.  Since the string is an integer, it can 
            // be converted back into a numeric value simply by using atoi().

            if( Query.IsRead() )
            {
                s32 C = (m_CircuitInit[Index] >> Shift) & 0x0F;
                Query.SetVarEnum( xfs( "%02d", C ) );
            }
            else
            {
                s32 C = x_atoi( Query.GetVarEnum() );
                m_CircuitInit[Index] &= ~(0x0F << Shift);
                m_CircuitInit[Index] |=  (   C << Shift);
            }

            return( TRUE );
        }
    }

    if( mp_settings::s_GameTypeBits && Query.IsVar( "Circuit\\Select" ) )
    {
        if( Query.IsRead() )
        {
            s32 C = -1;
            for( s32 t = 0; t < mp_settings::s_NGameTypes; t++ )
            {
                if( mp_settings::s_GameTypeBits & (1<<t) )
                {
                    s32 Shift = (t<8) ? t*4 : (t-8)*4;
                    s32 Index = (t<8) ?   0 :       1;
                    s32 V = (m_CircuitInit[Index] >> Shift) & 0x0F;
                    if( C == -1 )   C = V;
                    if( C !=  V )   C = 16;
                }
            }
            ASSERT( C != -1 );

            if( C == 16 )
                Query.SetVarEnum( "<circuits differ>" );
            else
            {
                const char* pEnum = mp_settings::GetCircuitEnum();
                while( C )
                {
                    C -= 1;
                    while( *pEnum )
                        pEnum++;
                    pEnum++;
                }
                Query.SetVarEnum( pEnum );
            }
        }
        else
        {
            s32 C = x_atoi( Query.GetVarEnum() );
            for( s32 t = 0; t < mp_settings::s_NGameTypes; t++ )
            {
                if( mp_settings::s_GameTypeBits & (1<<t) )
                {
                    s32 Shift = (t<8) ? t*4 : (t-8)*4;
                    s32 Index = (t<8) ?   0 :       1;
                    m_CircuitInit[Index] &= ~(0x0F << Shift);
                    m_CircuitInit[Index] |=  (   C << Shift);
                }
            }
        }

        return( TRUE );
    }
#endif

    return( FALSE );
}

//==============================================================================
#ifdef X_EDITOR
//------------------------------------------------------------------------------

xcolor circuit::GetColor( void )
{
    xcolor Color;

    switch( GetTeamBits() )
    {
    case 0x00000000:    Color = XCOLOR_YELLOW;      break;
    case 0x00000001:    Color = XCOLOR_RED;         break;
    case 0x00000002:    Color = XCOLOR_GREEN;       break;
    case 0xFFFFFFFF:    Color = XCOLOR_BLUE;        break;
    case 0xDEADC0DE:    Color = XCOLOR_WHITE;       break;
    default:            ASSERT( FALSE );            break;
    }

    return Color;
}

//------------------------------------------------------------------------------

void circuit::SpecialRender( vector3& Position )
{
    xcolor Color = GetColor();

    vector3 Lo = Position;
    vector3 Hi = Lo;

    if( mp_settings::s_HighY < Hi.GetY() )
        mp_settings::s_HighY = Hi.GetY();

    Hi.GetY() = mp_settings::s_HighY + 1000.0f;

    draw_Line ( Lo, Hi, Color );
    draw_Point( Hi, Color, 3 );
}

//------------------------------------------------------------------------------
#endif // ifdef X_EDITOR
//==============================================================================
