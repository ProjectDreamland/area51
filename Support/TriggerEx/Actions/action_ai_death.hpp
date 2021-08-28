///////////////////////////////////////////////////////////////////////////////
//
//  action_ai_death.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_ai_death_
#define _action_ai_death_

//=========================================================================
// INCLUDES
//=========================================================================

#include "action_ai_base.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "Characters\Character.hpp"

//=========================== ==============================================
// action_ai_death
//=========================================================================

class action_ai_death : public action_ai_base
{
// Defines
public:
    
    enum death_type
    {
        DEATH_TYPE_RAGDOLL,             // Go to ragdoll with force
        DEATH_TYPE_ANIM,                // Play specified death anim
        DEATH_TYPE_RAGDOLL_INACTIVE,    // Go to frozen ragdoll
    };
    
    
// Functions
public:
                    action_ai_death                     ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_AI_DEATH;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Kills npc via ragdoll or by specific anim."; } 
    virtual         const char*         GetDescription  ( void );

    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         object_affecter*    GetObjectRef1   ( xstring& Desc );
    virtual         s32*                GetAnimRef      ( xstring& Desc, s32& AnimName );
#endif

    virtual         ai_action_types     GetAIActionType ( void ) { return AI_DEATH; }

                    death_type          GetDeathType            ( void );
                    const char*         GetAnimGroup            ( void );
                    const char*         GetAnimName             ( void );                       
                    guid                GetRagdollForceLocation ( void );                       
                    f32                 GetRagdollForceAmount   ( void );                       
                    f32                 GetRagdollForceRadius   ( void );                       
    virtual         s32                 GetNextState            ( void ) { return character_state::STATE_DEATH; }


protected:

    death_type      m_DeathType;                // Type of death
    
    // Animation vars
    s32             m_AnimName;                 // Name of animation to play
    s32             m_AnimGroupName;            // Name of animation group
    
    // Ragdoll vars
    object_affecter m_RagdollForceLocation;     // Marker for force direction
    f32             m_RagdollForceAmount;       // Force amount
    f32             m_RagdollForceRadius;       // Force radius
};

//=========================================================================

inline
action_ai_death::death_type action_ai_death::GetDeathType( void )
{
    return m_DeathType;
}

//=========================================================================

inline
const char* action_ai_death::GetAnimGroup( void ) 
{ 
    if (m_AnimGroupName != -1)
        return g_StringMgr.GetString(m_AnimGroupName); 
    else
        return "";
}

//=========================================================================

inline
const char* action_ai_death::GetAnimName( void ) 
{ 
    if (m_AnimName != -1)
        return g_StringMgr.GetString(m_AnimName); 
    else
        return "";
}

//=========================================================================

inline
guid action_ai_death::GetRagdollForceLocation( void )
{
    return m_RagdollForceLocation.GetGuid();
}

//=========================================================================

inline
f32 action_ai_death::GetRagdollForceAmount( void )
{
    return m_RagdollForceAmount;
}

//=========================================================================

inline
f32 action_ai_death::GetRagdollForceRadius( void )
{
    return m_RagdollForceRadius;
}

//=========================================================================

#endif
