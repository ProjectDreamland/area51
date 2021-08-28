///////////////////////////////////////////////////////////////////////////////////////////////////
//
//  brain.hpp
//
//      -the brain object manages an actors AI states, its virtual sensors, and its emotions.
//
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef BRAIN_HPP
#define BRAIN_HPP

#include "x_math.hpp"
#include "x_debug.hpp"
#include "MiscUtils\RTTI.hpp"
#include "obj_mgr\obj_mgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "navigation\nav_plan.hpp"
#include "emotion_controller.hpp"
#include "sensory_record.hpp"
#include "ai_states\ai_state.hpp"
//#include "objects\npc.hpp"
#include "meshutil\RawMesh2.hpp"
#include "locomotion\LocoSpecialOps.hpp"
#include "locomotion\LocoStage5.hpp"
#include "Weapons\weapon.hpp"
#include "AudioMgr\AudioMgr.hpp"


const s32 MAX_SENSORY_RECORDS = 8;
const s32 kMAX_AI_STATES = 32;
const s32 kMAX_WEAPONS = 4;

class npc;

class brain
{
public:
    enum brain_type
    {
        BRAIN_TYPE_FIRST =0,
        BRAIN_TYPE_NONE_SET,
        BRAIN_TYPE_BASE,
        BRAIN_TYPE_STAGE_FIVE,
        BRAIN_TYPE_SPEC_OPS,

        BRAIN_TYPE_LAST,
        BRAIN_TYPE_FORCE32BIT = 0xFFFFFFFF    
    };





public:
                        brain(npc* newOwner);
    virtual             ~brain( );
    
    virtual void        Init(void);

    virtual void        SetNewDestination( vector3& newDestination );
    virtual void        UpdateNavPlan( void );
    virtual nav_plan&   GetNavPlan( void );

    virtual void        SetLookAtObject( guid thisGuid );
    virtual guid        GetLookAtObject( void );

    virtual void        SetAimAtObject( guid thisGuid );
    virtual guid        GetAimAtObject( void );

    virtual void        SetCurrentAIState( ai_state* thisAIState );

    virtual void        OnAdvanceLogic( f32 deltaTime );
    virtual void        OnDamaged     ( s32 Damage, vector3* thisDirection = NULL);
    virtual void        OnPain        ( const pain& Pain ) { (void)Pain ; }


    virtual void        UpdateSenses( f32 deltaTime );

    virtual void        AddSensorRecord( vector3& thisPoint,guid thisGuid, type_of_contact thisType);

    virtual ai_state*   GetStateByName(const char* thisName );

    virtual void        RequestStateChange( ai_state* thisState );
    virtual void        RequestStateChange( const char* thisState );

    virtual xbool       IsNewContactHostile( void );
    virtual void        ResetNewContact( void );
    virtual sensory_record* GetMostRecentContact(xbool enemyOnly = false);

    virtual xbool       UpdateMovement( f32 deltaTime );
    emotion_controller& GetEmotions(void )  { return m_Emotions; }

    virtual npc*        GetOwner(void)      { return m_Owner; }

    virtual void        OnRender ( void );

    virtual guid        EvaluateTargets( void );



    virtual xbool       IsObjectVisible( guid thisObject );
    virtual void        ClearDestination( void );

    virtual xbool       FindCover( vector3& thisTarget, vector3& thisResult );


    virtual xbool       AttemptShot( void );

    virtual f32         GetDistanceToDestinationSquared(void );
    virtual void        SetLookAtDestination(void);
    virtual void        SetLookAtPoint(vector3 thisPoint);
    virtual vector3     GetCurrentLookAtVector( void );


    virtual xbool       IsNavPlanComplete(void )  { return m_DestinationReached;   }
    
    virtual vector3     GetLastKnownPosition( guid thisObject );

    virtual guid        FindAFriend( void );

    virtual f32         GetEyeLevel(void);
    virtual f32         GetCoverLevel(void);

    virtual f32         GetTimeSinceObjectSeen( guid thisGuid );
    virtual void        UpdateLookAtDestination( void );
    virtual vector3     GetPointOnPath( f32  thisFarAhead );
    
//    virtual void        GoToNearestPatrolPoint( void );
    virtual vector3     GetNearestPatrolPoint( void );
    virtual node_slot_id GetNearestPatrolNode( void );
    virtual void        GetNextPatrolPointSlot( node_slot_id &thisSlot, xbool &forward );
            
    virtual         void OnLoad                  ( void );
    virtual         void SetCurrentFromStarting  ( void );
    virtual         void SetStartingFromCurrent  ( void );
    
    virtual         void EnableMovement(void ) { m_MoveDisabled = false; }
    virtual         void DisableMovement(void ){ m_MoveDisabled = true;  }

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Weapon functions
///////////////////////////////////////////////////////////////////////////////////////////////////
    virtual xbool       FireWeaponAt( guid thisTarget, s32 thisWeapon = 0 );


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Wander functions
///////////////////////////////////////////////////////////////////////////////////////////////////
    virtual void        UpdateWander( f32 deltaTime );
    virtual void        PickANewDestination( void );


///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////

    virtual     void            OnEnumProp( prop_enum&  List );
    virtual     xbool           OnProperty( prop_query& I    );

    virtual     void            SetStateToDelete( ai_state* thisState );

    virtual const char*         GetAIStateEnumText( void );

//==============================================================================
// AUDIO ID'S
    s32                         m_SpecOpType;               // One of the three different spec ops type.
    s32                         m_AlertVoiceID;             // The alert voice id.
    s32                         m_ManDownVoiceID;           // Man down voice id.
    s32                         m_SpotStage5VoiceID;        // Spot stage 5 voice id.
    s32                         m_IdleVoiceID;              // Idle voice id.


protected:

    emotion_controller          m_Emotions;
    xarray<ai_state*>           m_AIStates;

    ai_state*                   m_CurrentState;
    ai_state*                   m_DesiredState;
    ai_state*                   m_StateToDelete;

    nav_plan                    m_NavPlan;
    f32                         m_RecognitionRangeVisual;
    f32                         m_RecognitionRangeAudio;

    f32                         m_TimeBetweenSensoryUpdates;
    f32                         m_TimeSinceLastSensoryUpdate;
    f32                         m_RecognitionMemoryLength;
    f32                         m_MaxSearchRangeForCover;

    npc*                        m_Owner;
    xbool                       m_NewContactHostile;

    
    xbool                       m_DestinationReached;
    xbool                       m_LastNodeReached;

    xbool                       m_LookAtDestination;

    ai_state::ai_state_type     m_Type[32];

    xbool                       m_MoveDisabled;
    
///////////////////////////////////////////////////////////////////////////////////////////////////
//  Locomotion
///////////////////////////////////////////////////////////////////////////////////////////////////
    vector3                     m_LookAtPoint;

    guid                        m_LookAtObject;
    guid                        m_AimAtObject;
    
    vector3                     m_LastPointDuringMove;
    f32                         m_TimeSinceLastMove;
    f32                         m_MinimumDistanceToConsiderMovementActive;
    f32                         m_TimeTillReThinkPath;
    f32                         m_TimeTillCancelPath;

    f32                         m_EyeLevel;
    f32                         m_CoverLevel;
    f32                         m_LookThisFarAhead;

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Weapon 
///////////////////////////////////////////////////////////////////////////////////////////////////
    
    weapon*                     m_Weapons[ kMAX_WEAPONS ];
    s32                         m_BoneIndexForWeapon;
    
///////////////////////////////////////////////////////////////////////////////////////////////////
//  Starting Values
///////////////////////////////////////////////////////////////////////////////////////////////////

   char                         m_StartingAIName[64];
   

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Editor
///////////////////////////////////////////////////////////////////////////////////////////////////
    char                        m_StateEnumName[512];
    char                        m_AIName[64];
    sensory_record              m_Records[MAX_SENSORY_RECORDS];

};

///////////////////////////////////////////////////////////////////////////////////////////////////

inline
xbool brain::IsNewContactHostile( void )
{
    return m_NewContactHostile;
}

inline 
void brain::ResetNewContact( void )
{
    m_NewContactHostile = false;
}



#endif//BRAIN_HPP