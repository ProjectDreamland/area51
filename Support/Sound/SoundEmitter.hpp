#ifndef SOUND_EMITTER_HPP
#define SOUND_EMITTER_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"

//=========================================================================
// CLASS
//=========================================================================
class sound_emitter : public object
{
public:

    CREATE_RTTI( sound_emitter, object, object )

    enum sound_emitter_type{ 
                            POINT, 
                            RANDOM, 
                            AMBIENT };

    enum sound_emitter_shape{ 
                            SHAPE_SPHERE, 
                            SHAPE_ORIENTED_BOX };

                            sound_emitter                       ( void );
    virtual bbox            GetLocalBBox                        ( void ) const;
    virtual s32             GetMaterial                         ( void ) const { return MAT_TYPE_NULL; }
    virtual void            OnAdvanceLogic                      ( f32 DeltaTime );      
    virtual void            OnEnumProp                          ( prop_enum& List );
    virtual xbool           OnProperty                          ( prop_query& I );
    virtual void            OnActivate                          ( xbool Flag );
    virtual void            OnKill                              ( void );               
    virtual void            OnMove                              ( const vector3& NewPos   );      
    virtual void            OnMoveRel                           ( const vector3& DeltaPos );    
    virtual void            OnTransform                         ( const matrix4& L2W      );
    virtual void            OnTriggerTransform                  ( const matrix4& L2W      );
#ifdef X_EDITOR
    virtual s32             OnValidateProperties                ( xstring& ErrorMsg );
#endif
            void            OnTeleportActivate                  ( const vector3& Position );
    
protected:
            f32             ClosestPointToAABox                 ( const vector3& Point, const bbox& Box, vector3& ClosestPoint);

            f32             DistanceBetweenNearandFarClipOBox   ( const vector3& Point, const matrix4& ObjectL2W, vector3& Extent, 
                                                                  const bbox& FarClip, vector3& PointOnBox );

            f32             DistanceBetweenNearandFarClipAABox  ( const bbox& NearClipBox, const bbox& FarClipBox, 
                                                                  const vector3& ListnerPos, vector3& PointOnBox );

            vector3         GetPointOnEmitter                   ( const vector3& ListnerPos, f32& DistanceToPoint );
            f32             GetEmitterClipScale                 ( f32 Distance );

            void            UpdateSound                         ( vector3& VirtualPos, f32 EmitterClipScale,
                                                                  f32 FinalVolume );
            void            Update2DSound                       ( f32 FinalVolume );

            void            StopSoundEmitter                    ( void );
            xbool           IsSoundActive                       ( void );

    virtual const object_desc&  GetTypeDesc                     ( void ) const;
    static  const object_desc&  GetObjectType                   ( void );

//=========================================================================

protected:

#ifndef X_RETAIL
    virtual void            OnDebugRender   ( void );
#endif // X_RETAIL

    virtual void            OnColCheck      ( void );
    virtual void            OnColNotify     ( object& Object );

    rhandle<char>           m_hAudioPackage;
    s32                     m_EmitterType;
    s32                     m_EmitterShape;
    f32                     m_NearClip;
    f32                     m_FarClip;
    f32                     m_TriggerArea;
    char                    m_Label[64];
    f32                     m_MinVolume;
    f32                     m_MaxVolume;
    f32                     m_CurrentVolume;
    f32                     m_MinInterval;
    f32                     m_MaxInterval;
    f32                     m_AmbientTransition;

    bbox                    m_FarClipBox;
    bbox                    m_NearClipBox;
    f32                     m_AudioMgrNearClip;
    f32                     m_AudioMgrFarClip;
    s32                     m_VoiceID;
    xbool                   m_Collided;
    vector3                 m_ListnerPos;
    f32                     m_IntervalTime;
    f32                     m_XScale;
    f32                     m_YScale;
    f32                     m_ZScale;
    xbool                   m_Debug;

    f32                     m_PitchDepth;
    f32                     m_PitchSpeed;
    f32                     m_PitchBaseDepth;
    f32                     m_PitchBaseSpeed;
    f32                     m_PitchDepthVar;
    f32                     m_PitchSpeedVar;

    f32                     m_VolumeDepth;
    f32                     m_VolumeSpeed;
    f32                     m_VolumeBaseDepth;
    f32                     m_VolumeBaseSpeed;
    f32                     m_VolumeDepthVar;
    f32                     m_VolumeSpeedVar;
    f32                     m_VolumeFadeTime;

    f32                     m_CurrentPitchTime;
    f32                     m_CurrentVolumeTime;

    u8                      m_Flags;
    f32                     m_ReleaseTime;
    xbool                   m_EnableEndingRoutineCheck;
    xbool                   m_bForceActive;
    xbool                   m_bCollisionActivate;

    xbool                   m_bReverbEnable;
    f32                     m_WetDryMix;

    vector3 TestPoint;
    vector3 TestPoint2;
    vector3 TestPoint3;
    vector3 m_FinalPosTest;
    vector3 m_PojectectedFinalPosTest;


    bbox    NearDraw;
    bbox    FarDraw;
        
// Make friends here
};

//=========================================================================
// END
//=========================================================================
#endif