///////////////////////////////////////////////////////////////////////////////
//
//  cause_damage.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_cause_damage
#define _TRIGGER_ACTIONS_cause_damage

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"


//=========================================================================
// cause_damage 
//=========================================================================

class cause_damage : public actions_base
{
public:
                    cause_damage                 ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Cause Damage"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Causes Damage within an area."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& List );
    virtual			xbool	            OnProperty	( prop_query& I );

    virtual         void                OnRender    ( void );
 
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_CAUSE_DAMAGE;}

protected:

                    bbox                GetDamageBBox   ( void );
                    
    f32             m_Height;
    f32             m_Length;
    f32             m_Width;

    f32             m_MaxDamage;
    f32             m_MinDamage;

    f32             m_MaxForce;
    f32             m_MinForce;

    xbool           m_bCreateDecal;
    f32             m_DecalSize;
    
    xbool           m_bCreateLight;
    xcolor          m_LightColor;
    f32             m_LightRadius;
    f32             m_LightIntensity;
    f32             m_LightFadeTime;

    pain::type      m_PainType;
    static enum_table<pain::type>    m_PainTypeList; 

};

#endif
