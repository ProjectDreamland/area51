#ifndef LORE_OBJECT_HPP
#define LORE_OBJECT_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "x_bitmap.hpp"
#include "Objects\Render\RigidInst.hpp"

enum select_type 
{
    SELECT_NONE,
    SELECT_BRACKETS,
    SELECT_SQUARE,
    SELECT_TRIANGLE,
    SELECT_CIRCLE
};

#define NONLORE_OBJECT_OFFSET  1000

class player;

//=========================================================================
// CLASS
//=========================================================================
class lore_object : public object
{
public:

    CREATE_RTTI( lore_object, object, object )

                            lore_object                         ( void );
                            ~lore_object                        ( void );
    virtual s32             GetMaterial                         ( void ) const { return MAT_TYPE_NULL; }
    virtual void            OnRender                            ( void );
    virtual void            OnRenderTransparent                 ( void );

    // collision/polycache stuff
    virtual xbool               GetColDetails     ( s32 Key, detail_tri& Tri );
    virtual const matrix4*      GetBoneL2Ws       ( void ) ;
            rigid_inst&         GetRigidInst      ( void ) { return( m_RigidInst ); }
    virtual void                OnPolyCacheGather ( void );
    virtual void                OnTransform       ( const matrix4& L2W );

    virtual render_inst*        GetRenderInstPtr( void ) { return &m_RigidInst; }

#ifndef X_RETAIL
    virtual void            OnDebugRender                       ( void );
#endif // X_RETAIL

    virtual bbox            GetLocalBBox                        ( void ) const;      
            bbox            GetFocusBBox                        ( void ) const;

	virtual	void	        OnEnumProp		                    ( prop_enum& list );
	virtual	xbool	        OnProperty		                    ( prop_query& rPropQuery );
            
    virtual const object_desc&  GetTypeDesc                     ( void ) const;
    static  const object_desc&  GetObjectType                   ( void );
            
    virtual void            OnActivate                          ( xbool Flag );
    virtual void            OnAcquire                           ( xbool bIsRestoring = FALSE );
    void                    OnAdvanceLogic                      ( f32 DeltaTime );
    virtual xbool           IsTrueLoreObject                    ( void ) { return m_LoreID < NONLORE_OBJECT_OFFSET; }

    xbool                   HasFocus                            ( void );
    xbool                   IsActivated                         ( void ) { return m_bActive; }
    xbool                   TestPress                           ( void );
    s32                     GetLoreID                           ( void ) { return m_LoreID; }
    xbool                   IsStandingIn                        ( void ) { return m_bStandingIn; }

    void                    DoCollisionCheck                    ( player *pPlayer, vector3 &StartPos, vector3 &EndPos );

    // collision stuff
    virtual void                OnColCheck      ( void );
#ifndef X_RETAIL
    virtual void                OnColRender     ( xbool bRenderHigh );
#endif

#ifdef X_EDITOR
    virtual s32             OnValidateProperties    ( xstring& ErrorMsg );
    virtual xbool           IsIconSelectable        ( void );
#endif


    guid        m_TriggerGuid;
    guid        m_SpatialGuid;

    bbox        m_FocusBox;

    float       m_ViewDist;

    s32         m_DisplayStrTable;
    s32         m_DisplayStrTitle;
   
    float       m_TextDist;
    float       m_ScanDist;
    xbool       m_bScanLOS;

    char*       m_pScanAudio;
    select_type m_ViewType;
    xbool       m_bAutoOff;
    

protected:
    xbool       m_bActive;
    xbool       m_bDestroyAfterAcquired;        // do we kick off the trigger (if any) again on restore?
    xbool       m_bActivateTriggerOnRestore;

    // Internal Stuff
    f32         m_AnimState;
    f32         m_TextAlphaState;
    xbool       m_bLookingAt;
    xbool       m_bRendered;

    xbool       m_bHasLOS;
    xbool       m_bInViewRange;
    xbool       m_bInTextRange;
    xbool       m_bInScanRange;
    xbool       m_bStandingIn;

    u8          m_MaxAlpha;
    f32         m_ColorPhase;
    xbool       m_bUseGeometrySize;         // use geometry size or use size variable?
    s32         m_LoreID;

    static rhandle<xbitmap>     m_Bracket;

    matrix4                     m_RenderL2W;
    rigid_inst                  m_RigidInst;        // Instance for rendering object.
    u32                         m_VMeshMask;

//=========================================================================
};


//=========================================================================
// END
//=========================================================================

#endif