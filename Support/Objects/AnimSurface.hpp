
#ifndef ANIM_SURFACE_HPP
#define ANIM_SURFACE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Objects\PlaySurface.hpp"
#include "Animation\AnimPlayer.hpp"
#include "ZoneMgr\ZoneMgr.hpp"

//=========================================================================
// CLASS
//=========================================================================

class anim_surface : public play_surface
{
public:

    CREATE_RTTI( anim_surface, play_surface, object )

                                anim_surface    ( void );
                               ~anim_surface    ( void );

    virtual void                OnEnumProp      ( prop_enum&    List );
    virtual xbool               OnProperty      ( prop_query&   I    );

    virtual void                OnMove          ( const vector3& NewPos   );      
    virtual void                OnTransform     ( const matrix4& L2W      );
    virtual void                OnAdvanceLogic  ( f32 DeltaTime );
    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );
    virtual bbox                GetLocalBBox    ( void ) const;
    virtual void                GetBoneL2W      ( s32 iBone, matrix4& L2W );
    virtual void                OnEvent         ( const event& Event );

    virtual void                OnPolyCacheGather( void );


    virtual void                EnumAttachPoints        ( xstring& String   ) const;
    virtual s32                 GetAttachPointIDByName  ( const char* pName ) const;
    virtual xstring             GetAttachPointNameByID  ( s32 iAttachPt     ) const;
    virtual xbool               GetAttachPointData      ( s32      iAttachPt,
                                                          matrix4& L2W,
                                                          u32      Flags = 0 );
    virtual void                OnAttachedMove          ( s32             iAttachPt,
                                                          const matrix4&  L2W );

    virtual simple_anim_player* GetSimpleAnimPlayer     ( void ) { return &m_AnimPlayer; }
    virtual anim_group::handle* GetAnimGroupHandlePtr   ( void ) { return &m_hAnimGroup; }

protected:

    virtual void                OnRender        ( void );
    virtual void                OnColCheck      ( void );

#ifndef X_RETAIL
    virtual void                OnColRender     ( xbool bRenderHigh );
#endif // X_RETAIL

    virtual const matrix4*      GetBoneL2Ws     ( void );
            void                UpdateZoneTrack ( void );
    

protected:

    anim_group::handle          m_hAnimGroup;
    rhandle<char>               m_hAudioPackage;
    simple_anim_player          m_AnimPlayer;
    s16                         m_iBackupAnimString;
    zone_mgr::tracker           m_ZoneTracker;
};

//=========================================================================
// END
//=========================================================================
#endif