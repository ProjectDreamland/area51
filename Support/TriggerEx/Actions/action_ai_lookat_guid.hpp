///////////////////////////////////////////////////////////////////////////////
//
//  action_ai_lookat_guid.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_ai_lookat_guid_
#define _action_ai_lookat_guid_

//=========================================================================
// INCLUDES
//=========================================================================

#include "action_ai_base.hpp"

//=========================== ==============================================
// action_ai_lookat_guid
//=========================================================================

class action_ai_lookat_guid : public action_ai_base
{
public:
                    action_ai_lookat_guid               ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_AI_LOOKAT_GUID;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Look at a particular guid."; } 
    virtual         const char*         GetDescription  ( void );

    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         object_affecter*    GetObjectRef1   ( xstring& Desc );
#endif

    virtual         ai_action_types     GetAIActionType ( void ) { return AI_LOOK_AT_GUID; }

                    guid                GetLookAtGuid   ( void );
    inline          xbool               GetHeadLookAt   ( void ) { return m_HeadLookat; }
    inline          f32                 GetLookAtDist   ( void ) { return m_SightDistance; }
    inline          f32                 GetLookAtFOV    ( void ) { return m_SightFOV; }
    inline          s32                 GetNextState    ( void ) { return m_NextAiState; }

protected:

    object_affecter m_TargetAffecter;

    xbool           m_HeadLookat;
    f32             m_SightDistance;
    f32             m_SightFOV;
    s32             m_NextAiState;             // AI State to change too at end..
};
    

#endif
