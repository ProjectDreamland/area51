///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ai_state_attack_normal.hpp
//
//      - attack normal is what would be considered a normal attack state.  AI tried to get to an
//          appropriate range and then attack.
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef AI_STATE_ATTACK_NORMAL_HPP
#define AI_STATE_ATTACK_NORMAL_HPP
#include "ai_state_attack.hpp"

class ai_state_attack_normal : public ai_state_attack
{
public:
                                ai_state_attack_normal(brain* myBrain = NULL);
    virtual                     ~ai_state_attack_normal();

    virtual     void            OnAdvanceLogic( f32 deltaTime );

    virtual     void            OnEnterState( void ); 
    virtual     xbool           OnAttemptExit( void );
    virtual     void            OnExitState( void );
    
    virtual     void            OnInit( void );
    
    virtual     ai_state_type   GetType(void) { return ai_state::AI_STATE_TYPE_ATTACK_NORMAL; }
    virtual     const char*     GetTypeName(void) { return ("ATTACK_NORMAL"); }



///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual     void            OnEnumProp( prop_enum&  List );
    virtual     xbool           OnProperty( prop_query& I    );


protected:
    
    guid        m_Target;


};


#endif//AI_STATE_ATTACK_NORMAL_HPP