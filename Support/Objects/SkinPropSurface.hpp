
#ifndef SKIN_PROP_SURFACE_HPP
#define SKIN_PROP_SURFACE_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "x_color.hpp"
#include "Obj_mgr\obj_mgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Objects\Render\SkinInst.hpp"
#include "Animation\AnimPlayer.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Characters\FloorProperties.hpp"

//=========================================================================
// CLASS
//=========================================================================

class skin_prop_surface : public object
{
public:

    CREATE_RTTI( skin_prop_surface, object, object )

                            skin_prop_surface( void );
                           ~skin_prop_surface( void );

    virtual         bbox    GetLocalBBox    ( void ) const;
    virtual         s32     GetMaterial     ( void ) const { return m_iMaterial; }
    virtual void            OnEnumProp      ( prop_enum&    List );
    virtual xbool           OnProperty      ( prop_query&   I    );
//    virtual void            OnActivate      ( xbool Flag ) {m_OnActivate = Flag;}
    
            xbool           IsLoopMode       ( void ) const;
            xbool           IsAlwaysPlaying  ( void ) const;
            xbool           IsPlaying        ( void ) const;

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual render_inst*        GetRenderInstPtr      ( void ) { return &m_Inst; }

    virtual simple_anim_player* GetSimpleAnimPlayer   ( void ) { return &m_AnimPlayer; }
    virtual anim_group::handle* GetAnimGroupHandlePtr ( void ) { return &m_hAnimGroup; }

    virtual void            EnumAttachPoints        ( xstring& String   ) const;
    virtual s32             GetAttachPointIDByName  ( const char* pName ) const;
    virtual xstring         GetAttachPointNameByID  ( s32 iAttachPt     ) const;
    virtual xbool           GetAttachPointData      ( s32      iAttachPt,
                                                      matrix4& L2W,
                                                      u32      Flags = 0 );
    virtual void            OnAttachedMove          ( s32             iAttachPt,
                                                      const matrix4&  L2W );

protected:

    enum anims_type
    {
        ANIM_INVLAID = -1,
       
        ANIM_NORMAL,
        ANIM_DESTORY_ONCE_FINISH
    };

protected:

    virtual void                OnAdvanceLogic  ( f32 DeltaTime );
    virtual void                OnRender        ( void );
    virtual void                OnColCheck      ( void );

#ifndef X_RETAIL
    virtual void                OnColRender     ( xbool bRenderHigh );
#endif // X_RETAIL

    virtual void                OnMove          ( const vector3& NewPos   );        
    virtual void                OnTransform     ( const matrix4& L2W      );  
            xcolor              GetFloorColor   ( void )    { return m_FloorProperties.GetColor(); }
    
protected:

    skin_inst                   m_Inst;         // Render Instance for the Play Surface

    anim_group::handle          m_hAnimGroup;
    rhandle<char>               m_hAudioPackage;
    simple_anim_player          m_AnimPlayer;
    s16                         m_iBackupAnimString;
    s32                         m_iMaterial;
    floor_properties            m_FloorProperties;

protected:
    
    typedef enum_pair<anims_type>        anim_pair;
    typedef enum_table<anims_type>       anim_table;

    static anim_pair                        s_PairTable[];
    static anim_table                       s_EnumTable;

};

//=========================================================================
// END
//=========================================================================
#endif