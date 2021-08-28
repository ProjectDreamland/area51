///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  trigger_set
//
//      trigger_set is a list of triggers and information about how to evaluate them
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef TRIGGER_SET_HPP
#define TRIGGER_SET_HPP

#include "trigger.hpp"

const s32 k_MAX_TRIGGERS_PER_SET = 8;


class trigger_set : public trigger
{
public:

    enum trigger_set_type
    {
        TYPE_FIRST =0,
        TYPE_ALL_O
    
    };

                                trigger_set(void);
    virtual                     ~trigger_set();                                 


    virtual trigger_type        GetTriggerType(void) = 0;
    virtual xbool               IsTriggered( void ) = 0;

protected:

    slot_ID         m_Trigger[k_MAX_TRIGGERS_PER_SET];





};



#endif//TRIGGER_SET_HPP