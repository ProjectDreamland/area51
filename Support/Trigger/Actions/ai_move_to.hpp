///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_AI_MOVE_TO
#define _TRIGGER_ACTIONS_AI_MOVE_TO

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// AI_MOVE_TO : moves an AI character to a spot with pre and post states as defined
//=========================================================================

class ai_move_to : public actions_base
{
public:
    enum who_to_modify
    {
        ONE_NPC,
        NPC_CLASS,
        ALL_NPC
    };
public:
                    ai_move_to                      ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Move AI To"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Commands AI to move, with defined pre and post states"; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_AI_MOVE_TO;}

protected:
    
    guid            m_AIGuid;               // AI GUID
    s32             m_BeginState;           // AI State to change too at begining..
    s32             m_EndState;             // AI State to change too at end..
    vector3         m_Desintation;          // Destination position.
    s32             m_MoveStyle;            // Animation set that the AI uses when moving.
    xbool           m_bOverrideAI;          // Set this if the character moves to the point regardless of other things in the world.
    who_to_modify   m_ModifyGroup;          // What kinds of AI does this trigger modify
};

#endif
