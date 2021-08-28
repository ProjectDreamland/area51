//==============================================================================
//
// TweakMgr.hpp
//
//==============================================================================
#ifndef TWEAK_MGR_HPP
#define TWEAK_MGR_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "DataVault\DataVault.hpp"

//==============================================================================
//  TWEAK_HANDLE
//==============================================================================

class tweak_handle : public data_handle
{
public:
                        tweak_handle    ( void );
                        tweak_handle    ( const char* pDataDescriptorName );
                       ~tweak_handle    ( void );
                       
    xbool               Exists          ( void ) const;                       
    f32                 GetF32          ( void ) const;
    s32                 GetS32          ( void ) const;
    radian              GetRadian       ( void ) const;
    xbool               GetBool         ( void ) const;
};

//==============================================================================
// TWEAK_BLOCK
//==============================================================================

struct tweak_data_block : public data_block
{
                        tweak_data_block     ( void );
                       ~tweak_data_block     ( void );

    f32                 GetValue        ( void ) const;
    void                SetValue        ( f32 Value );

public:

    f32             m_Value;
};

//==============================================================================
// TWEAK_MGR
//==============================================================================

xbool LoadTweaks    ( const char* pDirectory );
void  UnloadTweaks  ( void );


//==============================================================================
// GLOBAL TWEAK UTILITY FUNCTIONS
//==============================================================================

// NOTE: For applicable functions, if tweak does not exist, then "Default" is returned.

f32     GetTweakF32     ( const char* pName );
f32     GetTweakF32     ( const char* pName, f32 Default );

s32     GetTweakS32     ( const char* pName );
s32     GetTweakS32     ( const char* pName, s32 Default );

radian  GetTweakRadian  ( const char* pName );
radian  GetTweakRadian  ( const char* pName, radian Default );

xbool   GetTweakBool    ( const char* pName );
xbool   GetTweakBool    ( const char* pName, xbool Default );


//==============================================================================
// END
//==============================================================================
#endif 

