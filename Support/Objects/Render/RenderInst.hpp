#ifndef RENDER_INST_HPP
#define RENDER_INST_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "MiscUtils\PropertyEnum.hpp"
#include "MiscUtils\Property.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "Render\Render.hpp"
#include "VirtualMeshMask.hpp"

//=========================================================================
// CLASS
//=========================================================================

class render_inst : public prop_interface
{
public:
                        render_inst         ( void );
                       ~render_inst         ( void );

    virtual void        OnEnumProp          ( prop_enum&    List );
    virtual xbool       OnProperty          ( prop_query&   I    );
    
            void        StartFade           ( s8 Direction, f32 TimeToFade );
            void        OnAdvanceLogic      ( f32 DeltaTime );
            u8          GetAlpha            ( void ) const              { return m_Alpha; }
    virtual void        LoadColorTable      ( const char* pFileName )   { (void)pFileName; }
    virtual geom*       GetGeom             ( void ) const = 0;
    virtual const char* GetGeomName         ( void ) const { return ""; }

    render::hgeom_inst          GetInst             ( void                      ) const;
    bbox&                       GetBBox             ( void                      ) const;
    u64                         GetLODMask          ( const matrix4& L2W        );
    u64                         GetLODMask          ( u16            ScreenSize );
    void                        SetVMeshBit         ( s32            Index,
                                                      xbool          OnOff      );
    void                        SetVMeshBit         ( const char*    pName,
                                                      xbool          OnOff      );
    const virtual_mesh_mask&    GetVMeshMask        ( void ) const;
    void                        SetVMeshMask        ( u32            Mask       );
    s32                         GetNActiveBones     ( const u64&     LODMask    ) const ;

#ifdef X_EDITOR
    // These functions are for refreshing resources and will register/unregister
    // the instances for refreshing the data.
    static  void        UnregisterAll   ( void );
    static  void        RegisterAll     ( void );
#endif // X_EDITOR

protected:
    render::hgeom_inst          m_hInst;            // Handle to the instance in the Render Manager

    virtual_mesh_mask           m_VMeshMask;                // which vmeshes are turned on?
    f32                         m_FadeTimeElapsed;          // how much time since we've started fading?
    f32                         m_FadeTime;                 // how long to fade in/out?
    s8                          m_FadeDirection;            // -1 fade-out, 0 no fading, 1 fade-in
    u8                          m_Alpha;                    // alpha value based on fading data    

#ifdef X_EDITOR
    // these variables are for creating a linked-list of render instances.
    // when we go to refresh newly compiled geometry in the editor, we can
    // have it automagically unregister the old geometry and register
    // the new geometry
    virtual void        UnregiserInst   ( void ) = 0;
    virtual void        RegisterInst    ( void ) = 0;
            void        AddToList       ( render_inst* pRenderInst );
            void        RemoveFromList  ( render_inst* pRenderInst );

    static render_inst*         s_pHead;
    render_inst*                m_pNext;
    render_inst*                m_pPrev;
#endif // X_EDITOR
};

//=========================================================================

inline render::hgeom_inst render_inst::GetInst( void ) const
{
    return m_hInst;
}

//=========================================================================

inline u64 render_inst::GetLODMask( u16 ScreenSize )
{
    geom* pGeom = GetGeom();
    if( pGeom )
    {
        return pGeom->GetLODMask( m_VMeshMask, ScreenSize );
    }
    else
    {
        return (u64)-1;
    }
}

//=========================================================================

inline void render_inst::SetVMeshBit( s32 Index, xbool OnOff )
{
    if( OnOff )
        m_VMeshMask.VMeshMask |= (1<<Index);
    else
        m_VMeshMask.VMeshMask &= ~(1<<Index);
}

//=========================================================================

inline void render_inst::SetVMeshBit( const char* pName, xbool OnOff )
{
    geom* pGeom = GetGeom();
    if( pGeom )
    {
        s32 Index = pGeom->GetVMeshIndex( pName );
        if( Index != -1 )
        {
            if( OnOff )
                m_VMeshMask.VMeshMask |= (1<<Index);
            else
                m_VMeshMask.VMeshMask &= ~(1<<Index);
        }
    }
}

//=========================================================================

inline const virtual_mesh_mask& render_inst::GetVMeshMask( void ) const
{
    return m_VMeshMask;
}

//=========================================================================

inline void render_inst::SetVMeshMask( u32 Mask )
{
#ifdef mreed
    char Addr[20];
    x_strcpy( Addr, xfs( "0x%X", (void*)this ) );
    LOG_MESSAGE( "render_inst::SetVMeshMask", "Mask: %i", Mask );
#endif
    m_VMeshMask.VMeshMask = Mask;
}

//=========================================================================

#endif
