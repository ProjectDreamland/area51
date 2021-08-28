//==============================================================================
//
// bomb_object.hpp
//
// Copyright (c) 2003 Inevitable Entertainment Inc. All rights reserved.
//
// Bomb object
//
//==============================================================================

#ifndef __BOMB_OBJECT__ 
#define __BOMB_OBJECT__ 


//=============================================================================
// INCLUDES
//=============================================================================
#include "DestructibleObj.hpp"


//=============================================================================
// CLASSES
//=============================================================================

// Class bomb_object
class bomb_object : public destructible_obj    
{
// Defines
public:

    // Misc 
    enum defines
    {
        MAX_MESHES = 12     // Max meshes for this object 
    };

    // These enums are relative to the HUD
    enum hud_bomb_object_event_states
    {
        HUD_BOMB_NOT_ACTIVE = 0,
        HUD_BOMB_DISARMING,
        HUD_BOMB_TARGETED,
        HUD_BOMB_DISARMED,
        HUD_BOMB_DETONATED,
    };

    // These enums are relative to the bomb
    enum bomb_object_event_states
    {
        BOMB_INVALID = -1,
        BOMB_ACTIVE,
        BOMB_NOT_ACTIVE,
        BOMB_DISARMED,
        BOMB_EXPOLDING,
        BOMB_DETONATED,
        BOMB_TRIGGERED,
        BOMB_COUNTING_DOWN,
        BOMB_END
    };

// Public functions
public:

    CREATE_RTTI( bomb_object, object, object )

	                            bomb_object         (void);
                               ~bomb_object         (void);

    virtual bbox                GetLocalBBox        ( void ) const;
    virtual s32                 GetMaterial         ( void ) const { return MAT_TYPE_NULL; }
                                                    
    virtual void                OnEnumProp          ( prop_enum&    List );
    virtual xbool               OnProperty          ( prop_query&   I    );
    virtual void                OnAdvanceLogic      ( f32 DeltaTime );
                                                    
            rigid_inst&         GetRigidInst        ( void ) { return( m_Inst); }
                                                    
    s32                         FindCorrectMesh     ( const char* pName ) ;
                                                    
    virtual const object_desc&  GetTypeDesc         ( void ) const;
    static  const object_desc&  GetObjectType       ( void );

            void                SetHUDEventState    ( s32 state );
            void                SetBombEventState   ( s32 state);
            s32                 GetBombClockSegments( void );
            s32                 GetHUDEventState    ( void );
            s32                 GetBombEventState   ( void );

// Private functions
protected:
    virtual void                UpdateHUDBombStates ( s32 state, f32 DeltaTime ); 
    virtual void                UpdateBombStates    ( s32 state, f32 DeltaTime );
    virtual void                OnRender            ( void );
    virtual xbool               RenderDigit         ( s32 iMesh, u32 Digit );
    virtual void                DisplayBombCount    ( u32 Minutes, u32 Seconds );
    virtual void                InitMeshIndices     ( void );
    virtual void                OnActivate          ( xbool Flag );
        void                    TurnOnDigitMeshes(void);
    void                        TurnOnBlankSreenMesh(void);
    void                        TurnOnAboutToExpoldeMesh(void);


// Data    
protected:
    f32                         m_EventTimeStamp;
    f32                         m_EventTimerVector;
    u8                          m_EventUnit;
    f32                         m_DetonateTimeMinutes;
    f32                         m_DetonateTimeSeconds;

    f32                         m_DeactivateTime;

    s32                         m_EventHUDState;
    s32                         m_EventBombState;
    f32                         m_HUDBombTimer;

    s32                         m_BombTimerMinutes;
    s32                         m_BombTimerSeconds; 
    f32                         m_BombTimer;
    s8                          m_iMesh[MAX_MESHES];
    u64                         m_MeshMask;
    u32                         m_BombExplodState;
    s32                         m_DisarmSound;
    s32                         m_CountDownSound;
    s32                         m_BombAboutToExplodeSound;
    xbool                       m_MeshInitFlag;
    f32                         m_DisarmBlink;
    xbool                       m_BlinkDisarmFlag;

    f32                         m_ScreenShakeTime;
    f32                         m_ScreenShakeAmount;
    f32                         m_ScreenShakeSpeed;
    xbool                       m_UseScreenShake;

    f32                         m_FeedBackIntensity;
    f32                         m_FeedBackDuration;
      
    typedef enum_pair<bomb_object_event_states> BombState_Pair;
    typedef enum_table<bomb_object_event_states> BombState_Table;
     
    static BombState_Pair       s_BombStatePairs[];
    static BombState_Table      s_BombStateTable;

};

//=============================================================================

#endif // BOMB_OBJECT
