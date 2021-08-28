
#ifndef PLAY_SURFACE_HPP
#define PLAY_SURFACE_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Objects\Render\RigidInst.hpp"

//=========================================================================
// CLASS
//=========================================================================

class play_surface : public object
{
public:

    CREATE_RTTI( play_surface, object, object )

                                 play_surface    ( void );
                                ~play_surface    ( void );

    virtual bbox                GetLocalBBox    ( void ) const;
    virtual s32                 GetMaterial     ( void ) const { return MAT_TYPE_NULL; }
    
    virtual void                OnEnumProp      ( prop_enum&    List );
    virtual xbool               OnProperty      ( prop_query&   I    );
    virtual void                OnTransform     ( const matrix4& L2W      );
    
    virtual xbool               GetColDetails   ( s32 Key, detail_tri& Tri );
    virtual const matrix4*      GetBoneL2Ws     ( void ) ;
            rigid_inst&         GetRigidInst    ( void ) { return( m_Inst ); }
                                
    virtual render_inst*        GetRenderInstPtr( void ) { return &m_Inst; }

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

    virtual void                OnPolyCacheGather   ( void );

protected:

    virtual void                OnImport        ( text_in& TIn );
    virtual void                OnRender        ( void );
    virtual void                OnColCheck      ( void );
    virtual void                OnKill          ( void );               
                                
            void                DoColCheck      ( const matrix4* pBones );
#ifndef X_RETAIL
    virtual void                OnColRender     ( xbool bRenderHigh );
            void                DoColRender     ( const matrix4* pBones, xbool High );
#endif // X_RETAIL

protected:

    rigid_inst                  m_Inst;         // Render Instance for the Play Surface
};

//=========================================================================
// END
//=========================================================================
#endif
