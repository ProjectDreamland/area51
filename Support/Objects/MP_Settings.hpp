//==============================================================================
//
//  MP_Settings.hpp
//
//==============================================================================

#ifndef MP_SETTINGS_HPP
#define MP_SETTINGS_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Obj_Mgr\Obj_Mgr.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

#define MAX_CIRCUITS        16
#define MAX_CIRCUIT_GAMES   16
#define MAX_CIRCUIT_NAME    16

//==============================================================================
//  TYPES
//==============================================================================

class game_mgr;

class mp_settings : public object
{

public:

    CREATE_RTTI( mp_settings, object, object )

                                mp_settings     ( void );
                               ~mp_settings     ( void );
                                     
virtual         bbox            GetLocalBBox    ( void ) const;
virtual const   object_desc&    GetTypeDesc     ( void ) const;
static  const   object_desc&    GetObjectType   ( void );
virtual         s32             GetMaterial     ( void ) const;
                void            OnEnumProp      ( prop_enum&  List );
                xbool           OnProperty      ( prop_query& List );

protected:

static          s32             s_Circuit[MAX_CIRCUIT_GAMES];   // [GameType]

public: 

static  const   char*           s_ValueName[];
static  const   char*           s_GameTypeAbbr[];
static          s32             s_NGameTypes;

//------------------------------------------------------------------------------
#ifdef X_EDITOR
//------------------------------------------------------------------------------

public:

    static          u32     GetTeamBits     ( s32 Circuit );
    static  const   char*   GetValueName    ( s32 Circuit );
    static  const   char*   GetCircuitEnum  ( void );
    static  const   char*   GetActiveAbbr   ( void );

    static          char    s_CircuitEnum[512];
    static          char    s_ActiveAbbrList[128];
    static          xbool   s_DirtyEnum;
    static          xbool   s_DirtyAbbr;
    static          xbool   s_Selected;
    static          f32     s_HighY;
    static          u32     s_GameTypeBits;

friend game_mgr;

//------------------------------------------------------------------------------
#else
//------------------------------------------------------------------------------

static          u32     GetTeamBits     ( s32 Circuit, s32 GameType );

//------------------------------------------------------------------------------
#endif // ifdef X_EDITOR
//------------------------------------------------------------------------------

};

//==============================================================================
//  INLINES
//==============================================================================

inline
s32 mp_settings::GetMaterial( void ) const 
{ 
    return( MAT_TYPE_NULL );
}

//==============================================================================
#endif // MP_SETTINGS_HPP
//==============================================================================
