///////////////////////////////////////////////////////////////////////////////
//
//  action_ai_base.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_ai_base_H
#define _action_ai_base_H

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "..\Affecters\object_affecter.hpp"
#include "Characters\ResponseList.hpp"

//=========================================================================
// Check Property
//=========================================================================
class character_trigger_state;

class action_ai_base : public actions_ex_base
{
public:

/*    enum response_flags
    {
        RF_NULL                         =      0,
        RF_IGNORE_ATTACKS               = BIT( 1),  
        RF_IGNORE_SIGHT                 = BIT( 2),  
        RF_IGNORE_SOUND                 = BIT( 3),  
        RF_IGNORE_ALERTS                = BIT( 4),  
        RF_INVINCIBLE                   = BIT( 5)
    };*/

    // NOTE: Always add new enums to the end of the list or break existing game data!!!
    enum ai_action_types
    {
        AI_ACTION_NULL,
        AI_PATHFIND_TO_GUID,
        AI_ATTACK_GUID,
        AI_LOOK_AT_GUID,
        AI_DIALOG_LINE,
        AI_PLAY_ANIMATION,
        AI_SEARCHTO_GUID,
        AI_INVENTORY,        
        AI_DEATH,
    };

    CREATE_RTTI( action_ai_base, actions_ex_base, actions_ex_base )

                    action_ai_base                          ( guid ParentGuid );

    virtual         xbool                   Execute         ( f32 DeltaTime );    

    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );
    virtual         void                    OnActivate      ( xbool Flag );

#ifdef X_EDITOR
    virtual         object_affecter*        GetObjectRef0   ( xstring& Desc ) { Desc = "Character object error: "; return &m_CharacterAffecter; }
#endif

#ifndef X_RETAIL
    virtual         void                    OnDebugRender   ( s32 Index );
#endif // X_RETAIL

    virtual         ai_action_types         GetAIActionType ( void ) = 0;
    inline          void                    ReleaseBlocking ( void ) { m_bIsBlockingAction = FALSE; }
    
                    void                    SetResponseFlags( u32 newFlags )                    { m_ResponseFlags.SetFlags(newFlags); }
                    u32                     GetResponseFlags( void )                            { return m_ResponseFlags.GetFlags(); }
                    void                    SetCharacterAffecter(object_affecter newAffecter )  { m_CharacterAffecter = newAffecter; }
                    xbool                   GetBlockUntilComplete( void )                       { return m_bBlockUntilComplete; }
                    xbool                   GetIsRunningAction( void )                          { return m_bIsRunningAction;    }
                    xbool                   GetIsBlockingAction( void )                         { return m_bIsBlockingAction;   }
                    xbool                   GetMustSucceed( void )                              { return m_bMustSucceed;        }

protected:

    const char*     GetAIName( void );

    object_affecter  m_CharacterAffecter;
    xbool            m_bBlockUntilComplete;
    xbool            m_bIsRunningAction;
    xbool            m_bIsBlockingAction;
    xbool            m_bMustSucceed;

//    u32              m_ResponseFlags;
    response_list    m_ResponseFlags;

    friend character_trigger_state;
};

#endif
