///////////////////////////////////////////////////////////////////////////////
//
//
//
//
///////////////////////////////////////////////////////////////////////////////


#include "Obj_mgr\obj_mgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
//#include "Render\WallGeom.hpp"
#include "Animation\CharAnimPlayer.hpp"
///#include "AI\nav_mgr.hpp"
#include "objects\npc.hpp"
#include "Render\SkinGeom.hpp"



#ifndef AISTATEMACHINE_HPP
#define AISTATEMACHINE_HPP
 
/*
class anim_handle
{
public:
                    anim_handle(char_anim_player* thisAnimPlayer, char* thisAnimName, char* thisOurName );
    virtual         ~anim_handle();

    s32             GetIndexFromName(const char* thisName );
    const char*     GetNameFromIndex( s32 thisIndex );


protected:
    
    char_anim_player    *m_AnimPlayer;
    const char          m_OurName[32];
    const char*         m_NameInAnim;
    s32                 m_IndexInAnim;



};

*/
class ai_state_machine
{
public:


    enum AI_state
    {
        STATE_FIRST = 0,
        STATE_NONE,
        STATE_IDLE,
        STATE_ATTACKING,    
        STATE_AWARE , 
        STATE_FLEE,
        STATE_DEAD,
        STATE_LAST,
        STATE_FORCE32BIT = 0xFFFFFFFF
    };


    enum Turn_Direction
    {
        STATE_DIRECTION_FIRST,
        STATE_DIRECTION_NONE ,
        STATE_RIGHT,
        STATE_LEFT ,
        STATE_DIRECTION_LAST ,
        STATE_DIRECTION_FORCE32BIT = 0xffffffff
    };



public:


                            ai_state_machine(slot_id thisActor);
    virtual                 ~ai_state_machine();

    virtual const char*     GetAIName( void ) = 0;
    virtual void            OnAdvanceLogic(f32 TimeDelta);
    virtual void            SetState(AI_state newState);

    virtual const char*     GetStateName(void );
    virtual void            GotShot( slot_id thisSlot, s32 Damage ) {(void)thisSlot; (void)Damage; }

    xbool                   IsPlayerInCombat( void ) { return m_PlayerInCombat; }
    
    virtual const char*     GetAnimName( void ) { return m_AnimName;   }
    virtual AI_state        GetState( void );

    virtual xbool           IsAngry( void );
    virtual void            SetKilledByGrenade      ( void )  { m_KilledByGrenade = true; }
    virtual void            SetExplodeDirection     ( vector3 thisDirection  ) { m_ExplodeDirection = thisDirection; }
    virtual void            EnableWanderMode        ( xbool enable ) { m_WanderInIdle = enable; }

protected:

    slot_id                 m_Actor;
    AI_state                m_State;
    f32                     m_TimeInState;
                            
    const char*             m_StateNames[STATE_LAST];
    const char*             m_AnimName;
                            
    xbool                   m_npcSpotted;
    xbool                   m_npcHeard;
    f32                     m_LastTimeHeard;
    f32                     m_AwarenessRange;
    f32                     m_AwarenessVelocity;

    Turn_Direction          m_TurnDirection;
    vector3                 m_LastKnownLocation;
    xbool                   m_MoveToLastKnownLocation;
    f32                     m_ScanningTurnRate;
    f32                     m_ChanceToShoot;
    f32                     m_ChanceToWander;
    f32                     m_RandomMoveDistance;
    xbool                   m_WanderInIdle;

    xbool                   m_PlayerInCombat;

    f32                     m_TimeTurnStarted;
    f32                     m_TimeToScanForTarget;
    s32                     m_TimesShotSinceEvade;
                            
    f32                     m_DeactivationDistance;
    xbool                   m_Paused;    
    s32                     m_FramesBetweenVisCheck;
    s32                     m_FrameCount;
    
    static s32              m_sRefCountForMusic;
    
    static s32              m_AngryCritters;
    xbool                   m_KilledByGrenade;
    vector3                 m_ExplodeDirection;

};



#endif//AISTATEMACHINE_HPP