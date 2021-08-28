//==============================================================================
//
// PainMgr.hpp
//
//==============================================================================
#ifndef PAIN_MGR_HPP
#define PAIN_MGR_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "DataVault\DataVault.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"

class  pain_handle;
class  health_handle;
class  pain_health_handle;
struct pain_profile;
struct health_profile;
struct pain_health_profile;

//==============================================================================
// SUPPORT FOR GENERIC PAIN
//==============================================================================

enum generic_pain_type
{
    TYPE_GENERIC_1,
    TYPE_GENERIC_2,
    TYPE_GENERIC_3,
    TYPE_LETHAL,
    TYPE_EXPLOSIVE,
    TYPE_LASER,
    TYPE_ACID_1, 
    TYPE_ACID_2,
    TYPE_ACID_3, 

    TYPE_GOO_1,  
    TYPE_GOO_2,  
    TYPE_GOO_3,  

    TYPE_FIRE_1, 
    TYPE_FIRE_2, 
    TYPE_FIRE_3, 

    TYPE_DROWNING,
};

extern enum_table<generic_pain_type>  g_GenericPainTypeList;
pain_handle GetPainHandleForGenericPain( generic_pain_type Type );

//==============================================================================
//  HANDLES
//==============================================================================

class pain_handle : public data_handle
{
public:
                         pain_handle    ( const char* pPainDescriptor );
                         pain_handle    ( void );
                        ~pain_handle    ( void );

const pain_profile*   GetPainProfile ( void )const;
pain_health_handle    BuildPainHealthProfileHandle( const health_handle& Handle ) const;  
};

//==============================================================================

class health_handle : public data_handle
{
public:
            health_handle    ( const char* pHealthDescriptor );
            health_handle    ( void );
           ~health_handle    ( void );

const health_profile*   GetHealthProfile ( void ) const;
};

//==============================================================================

class pain_health_handle : public data_handle
{
public:
            pain_health_handle    ( const char* pPainHealthDescriptor );
            pain_health_handle    ( void );
           ~pain_health_handle    ( void );

const pain_health_profile*   GetPainHealthProfile ( void ) const;
};

//==============================================================================
// DATA BLOCKS
//==============================================================================

struct pain_profile : public data_block
{
        pain_profile     ( void );
        ~pain_profile     ( void );

public:

    f32     m_DamageNearDist;
    f32     m_DamageFarDist;
    f32     m_ForceNearDist;
    bbox    m_BBox;
    f32     m_ForceFarDist;
    s16     m_iPainHealthTable;
    u16     m_bSplash:1,
            m_bCheckLOS:1;
};

//==============================================================================

struct health_profile : public data_block
{
        health_profile     ( void );
        ~health_profile     ( void );

public:

    s16     m_iPainHealthTable;
};

//==============================================================================

struct pain_health_profile : public data_block
{
        pain_health_profile     ( void );
        ~pain_health_profile     ( void );

public:

    f32     m_Damage;
    f32     m_Force;
    s32     m_HitType;
};

//==============================================================================
// PAIN_MGR
//==============================================================================

xbool LoadPain    ( const char* pDirectory );
void  UnloadPain  ( void );

//==============================================================================
// END
//==============================================================================
#endif 

