///////////////////////////////////////////////////////////////////////////////
//
//  ObjectState.hpp
//
//
///////////////////////////////////////////////////////////////////////////////
#ifndef OBJECT_STATE_HPP
#define OBJECT_STATE_HPP

#include "Obj_mgr\obj_mgr.hpp"
#include "LuaLib\LuaMgr.hpp"

class  object_state
{
public:

    enum state_mood
    {
        STATE_MOOD_FIRST = 0,
        STATE_MOOD_NORMAL,
        STATE_MOOD_BORED,
        STATE_MOOD_HAPPY,
        STATE_MOOD_SCARED,
        STATE_MOOD_ANGRY,
        STATE_MOOD_CURIOUS,
        STATE_MOOD_CAUTIOUS,
        STATE_MOOD_LAST,
        MOOD_FORCE_32BIT = 0xFFFFFFFF
    };

    enum state_speed
    {
        STATE_SPEED_FIRST = 0,
        STATE_SPEED_NONE,            //  For states that involve no movement
        STATE_SPEED_STATIONARY,      //  For states that involve movement but are currently stationary
        STATE_SPEED_WALK,
        STATE_SPEED_RUN,
        STATE_SPEED_SPRINT,
        STATE_SPEED_SNEAK,
        STATE_SPEED_LAST,
        SPPED_FORCE_32BIT = 0xFFFFFFFF    
    };


 
public:
                            object_state( void) ;
    virtual                 ~object_state( );
                
    virtual void            OnInit(  void );        
    virtual void            OnEnter( void );
    virtual void            OnExit(  void );
    virtual void            OnKill(  void );

    virtual void            OnAdvanceLogic( f32 DeltaTime );        //  Only current state gets an OnAdvanceLogic call
//    virtual void            OnHeartBeat   ( f32 DeltaTime );        //  Inactive states still get heart beat calls

    virtual xbool           OnAssociateScript( lua_script* thisScript );


    //  Request version of the function will return false if the request is denied
    virtual xbool           RequestSpeedChange( state_speed newSpeed );
    virtual void            SetSpeed(           state_speed newSpeed );
    virtual void            OnSpeedChange(      state_speed oldSpeed, state_speed newSpeed );
 
    virtual xbool           RequestMoodChange(  state_mood  newMood );
    virtual void            SetMood(            state_mood  newMood );
    virtual void            OnMoodChange(       state_mood  oldMood,  state_mood  newMood );

    //  RequestMove will return false if request was denied
    virtual xbool           RequestMove(        vector3&    target );
    virtual void            OnCollision(        object*     collisionObject);
    
 
    virtual state_speed     GetSpeedType( void );
    virtual state_mood      GetMood     ( void );
    
    virtual const char*     MoodToString(   state_mood thisMood );
    virtual const char*     SpeedToString(  state_speed thisSpeed);

    virtual state_mood      StringToMood(   const char* thisMood );
    virtual state_speed     StringToSpeed(  const char* thisSpeed);

protected:

    s32                     m_Flags;
    char                    m_StateName[32];
    s32                     m_RefCount;
    state_mood              m_Mood;
    state_speed             m_Speed; 
    lua_script*             m_Script;
                            
    
};

#endif//OBJECT_STATE_HPP