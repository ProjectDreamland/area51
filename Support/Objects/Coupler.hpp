#ifndef __COUPLER_HPP__
#define __COUPLER_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "Object.hpp"
#include "Obj_mgr\Obj_mgr.hpp"

#define COUPLER_MAX_CHILDREN     4

//=========================================================================
// CLASS
//=========================================================================

class coupler : public object
{
//=====================================================================
// DEFINES
//=====================================================================
public:


//=====================================================================
// PUBLIC BASE CLASS FUNCTIONS
//=====================================================================
public:

    CREATE_RTTI( coupler, object, object )
    
                            coupler         ( void );
                            ~coupler        ( void );
    virtual bbox            GetLocalBBox    ( void ) const { return bbox( vector3(0,0,0), 100 ); }
    virtual s32             GetMaterial     ( void ) const { return MAT_TYPE_FLESH; }
    virtual void            OnEnumProp      ( prop_enum& List );
    virtual xbool           OnProperty      ( prop_query& I );
    virtual void            OnMove          ( const vector3& NewPos   );        
    virtual void            OnTransform     ( const matrix4& L2W      ); 

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );


            xbool           AddChild            ( guid        ChildGuid, 
                                                  const char* AttachPoint = "BaseObject" );
            xbool           RemoveChild         ( guid ChildGuid );            
            void            SnapAttachPoints    ( guid ChildGuid );
            void            UpdateRelative      ( guid ChildGuid );  
            
            void            UpdateAllRelative   ( void );
            
//=====================================================================
// PRIVATE BASE CLASS FUNCTIONS
//=====================================================================

protected:
    virtual void            OnInit              ( void );     
    virtual void            OnAdvanceLogic      ( f32 DeltaTime );
    virtual void            OnActivate          ( xbool bFlag   );

#ifndef X_RETAIL
    virtual void            OnDebugRender       ( void );
#endif // X_RETAIL

#ifdef X_EDITOR
    virtual s32             OnValidateProperties        ( xstring& ErrorMsg );
#endif // X_EDITOR


//=====================================================================
// PRIVATE INTERNAL FUNCTIONS
//=====================================================================
protected:
      
    void            UpdateAttachPoints  ( void );
    void            UpdateRelativeData  ( s32 iChild );
    void            SnapAttachPoints    ( s32 iChild );
    s32             GetChildIDByGuid    ( guid ChildGuid );
    
    void            AdvancePhysics      ( f32 DeltaTime );

//=====================================================================
// DATA
//=====================================================================

protected:
    struct child
    {
        void        Reset();
        void        SetAttachPoint( const char* pAttachPoint );

        guid        m_Guid;                 // Guid
        s32         m_AttachPtID;           // ID of attach point
        vector3     m_RelativePos;          // 
        radian3     m_RelativeRot;          //         
        s32         m_AttachPointString;
        u8          m_bAttachPointDirty:1,
                    m_bApplyPosition:1,
                    m_bApplyRotation:1;
    };

    struct child_phys
    {
        child_phys():
            m_Mass(5.0f),
            m_StrutLength(100.0f),
            m_Dampening(0.98f),
            m_LastVelocity(0,0,0)
        {
            m_RelativeTransform.Identity();
            m_FreeEnd.Set(0,0,0);
        }

        f32         m_Mass;
        f32         m_StrutLength;          // Dist between Parent AP and bbox center of child
        vector3     m_FreeEnd;
        matrix4     m_RelativeTransform;
        f32         m_Dampening;
        vector3     m_LastVelocity;
    };

    s32         m_nChild;
    child       m_Child[ COUPLER_MAX_CHILDREN ];
    child_phys* m_pChildPhys;

    guid        m_ParentGuid;                   // Guid
    s32         m_ParentID;                     // ID of attach point
    s32         m_ParentAttachPointString; 
    matrix4     m_LastParentL2W;
    u8          m_bParentAttachPointDirty:1,
                m_bChildPhysics:1,
                m_bFirstTimeThrough:1;

};

//=========================================================================
// END
//=========================================================================
#endif
