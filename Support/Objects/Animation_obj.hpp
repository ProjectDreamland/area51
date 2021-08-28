#ifndef ANIMATION_OBJECT_HPP
#define ANIMATION_OBJECT_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "Animation\AnimPlayer.hpp"

//=========================================================================
// CLASS
//=========================================================================

class animation_obj : public object
{
public:

    CREATE_RTTI( animation_obj, object, object )
    
                            animation_obj     ( void );
    virtual bbox            GetLocalBBox    ( void ) const { return bbox( vector3(0,0,0), 100 ); }
    virtual s32             GetMaterial     ( void ) const { return MAT_TYPE_FLESH; }
    virtual void            OnEnumProp      ( prop_enum& List );
    virtual xbool           OnProperty      ( prop_query& I );
    virtual void            OnActivate      ( xbool Flag );  
    virtual void            OnMove          ( const vector3& NewPos   );        
    virtual void            OnTransform     ( const matrix4& L2W      ); 
    

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=========================================================================

protected:

    enum
    {
        MAX_ANIMATED_OBJECTS = 64
    };

    enum
    {
        FLAGS_ACTIVE                = (1<<0),
        FLAGS_DESTROY_AFTER_PLAYING = (1<<1),
        FLAGS_LOOP                  = (1<<2),
        FLAGS_PING_PONG             = (1<<3),
        FLAGS_PLAY_BACKWARDS        = (1<<4),
    };

    struct keyframe
    {
        quaternion  Rot;
        vector3     Translate;
        f32         Time;
    };

    struct bone
    {
        guid        ObjectGuid;
        matrix4     BindPose;
        matrix4     InvBindPose;
    };

protected:

    virtual void            OnAdvanceLogic	    ( f32 DelaTime );
    virtual void            OnInit              ( void );     

#ifndef X_RETAIL
    virtual void            OnDebugRender       ( void );
#endif // X_RETAIL

            void            UpdateObjects       ( void );
            void            UpdateAnimL2W       ( void );

            void            SetEditKeyFrame     ( s32 Index );
            void            JumpToKey           ( s32 Index );
            s32             GetNextFrameIndex   ( s32 iFrame );
            xbool           HandleStates        ( f32 DelaTime, f32& ParmTime, s32& iNext );
            void            ComputeFrame        ( keyframe& Key, s32 iPrev, s32 iNext, f32 Time );
    
protected:

    rhandle<char>               m_hAudioPackage;

    xarray<keyframe>            m_KeyFrame;
    s32                         m_iCurrentKeyFrame;

    s32                         m_nBones;
    bone                        m_Bone[ MAX_ANIMATED_OBJECTS ];

    u32                         m_Flags;

    f32                         m_Time;
    s32                         m_iFrame;
    s32                         m_Cycle;
    keyframe                    m_CurrentKey;

};

//=========================================================================
// END
//=========================================================================
#endif
