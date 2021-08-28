///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  action
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef ACTION_HPP
#define ACTION_HPP

class action 
{
public:
    
    enum action_type
    {
        TYPE_FIRST = 0,
        TYPE_ACTOR_PLAY_ANIMATION,
        TYPE_OBJECT_PLAY_SOUND,
        TYPE_RUN_SCRIPT,
        TYPE_DELETE_OBJECT,
        TYPE_CREATE_OBJECT,
        TYPE_DAMAGE_OBJECT,
        TYPE_SET_VAR,
        TYPE_ACTION_SET,

        TYPE_LAST = 0xFFFFFFFF
    
    };

                                action(void);
    virtual                     ~action();

    virtual action_type         GetType(void) = 0;

    void                        PlayAction(void) = 0;


protected:
    


};


#endif//ACTION_HPP