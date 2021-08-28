///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Trigger.hpp
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef TRIGGER_HPP
#define TRIGGER_HPP

#include "Entropy.hpp"
#include "obj_mgr\Obj_Mgr.hpp"
#include "resourcemgr\resourcemgr.hpp"


class trigger 
{ 
public:

    enum trigger_type
    {
        TYPE_FIRST = 0,
        TYPE_OBJECT_IN_VOLUME,
        TYPE_OBJECT_EXITED_VOLUME,
        TYPE_OBJECT_ENTERED_VOLUME,
        TYPE_OBJECT_EXISTS,
        TYPE_OBJECT_PROXIMITY_OBJECT,
        TYPE_PLAYER_HAS_OBJECT,
        TYPE_BUTTON_IS_PRESSED,
        TYPE_IS_VAR_SET,
        TYPE_TIMER,
        TYPE_TRIGGER_SET,

        TYPE_LAST = 0xFFFFFFFF    
    };

    enum trigger_attr
    {
        ATTR_NULL                   =       0,
        ATTR_MOVING_OBJECTS         =  BIT( 0),
        ATTR_AI_OBJECTS             =  BIT( 1),
        ATTR_PLAYER_OBJECTS         =  BIT( 2),
        ATTR_ONLY_ONCE              =  BIT( 3),
        ATTR_PERIODICAL             =  BIT( 4),
        ATTR_NOT_THIS_TRIGGER       =  BIT( 5),     //  This trigger returns opposite of what is expected
        
        ATTR_MASK                   = 0x0000001F,
        ATTR_LAST                   = 0xFFFFFFFF
    
    };


                            trigger(void);
    virtual                ~trigger(void);

///////////////////////////////////////////////////////////////////////////////////////////////////  
//
//  GetTriggerType  - returns the type of trigger from the enum list above
//
//  IsTriggered     - the public method to check if triggered.  This just calls the protected
//                      method CheckTriggered and NOTs it if the NOT_THIS_TRIGGER flag is set
//                      in most cases.
//
//  OnUpdate        - Called every tick to give the triggers a chance to think.  Non-periodical
//                      triggers will just return.  Periodical will
//
//
//
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual trigger_type    GetTriggerType(void) = 0;

    xbool                   IsTriggered(void);  

    virtual void            OnUpdate( f32 deltaTime );

    virtual void            SetPeriod(f32 newPeriod );    

    inline trigger_attr     GetAttributes ( void );
    inline xbool            IsPeriodical( void);
                    
protected:

///////////////////////////////////////////////////////////////////////////////////////////////////
// Protected FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////
    virtual xbool           CheckTriggered( void ) = 0;
    
    virtual         void    OnInit(void);
   
///////////////////////////////////////////////////////////////////////////////////////////////////
//  Protected Types
///////////////////////////////////////////////////////////////////////////////////////////////////
    f32         m_Period;       //  Only check this one every m_Period seconds.  0 is every update    
    f32         m_NextActivationTime;  // Once triggered, this is set to current time + m_ResetPeriod

    xbool       m_Triggered;    //  Bottom line, is this trigger currently true or false

    trigger_attr m_Attributes;  //  shared trigger attributes


};
 


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Inlines
///////////////////////////////////////////////////////////////////////////////////////////////////

inline 
xbool  trigger::IsPeriodical( void)
{
    return ( m_Attributes & ATTR_PERIODICAL );
}


inline 
trigger::trigger_attr trigger::GetAttributes ( void )
{
    return m_Attributes;
    
}


#endif//TRIGGER_HPP