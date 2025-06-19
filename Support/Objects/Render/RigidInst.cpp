//=============================================================================
//
//  RigidInst.cpp
//
//=============================================================================

#include "Entropy.hpp"
#include "Objects\Render\RigidInst.hpp"
#include "ResourceMgr\ResourceMgr.hpp"

//=============================================================================

#ifdef TARGET_XBOX
void xbox_Unregister ( rigid_geom* pGeom );
#endif

//=============================================================================
// LOADER FOR THE RIGID GEOM RESOURCE
//=============================================================================

static struct rigid_loader : public rsc_loader
{
    //-------------------------------------------------------------------------

    rigid_loader( void ) : rsc_loader( "RIGID GEOM", ".rigidgeom" ) {}

    //-------------------------------------------------------------------------

    virtual void* PreLoad ( X_FILE* FP )
    {
        MEMORY_OWNER( "RIGID GEOM DATA" );

        fileio File;
        return( File.PreLoad( FP ) );
    }

    //-------------------------------------------------------------------------

    virtual void* Resolve ( void* pData ) 
    {
        fileio      File;
        rigid_geom* pRigidGeom = NULL;

        File.Resolved( (fileio::resolve*)pData, pRigidGeom );

        return( pRigidGeom );
    }

    //-------------------------------------------------------------------------

    virtual void Unload( void* pData )
    {
        rigid_geom* pRigidGeom = (rigid_geom*)pData;
        ASSERT( pRigidGeom );

        #ifdef TARGET_XBOX
        xbox_Unregister( pRigidGeom );
        #endif

        delete pRigidGeom;
    }

} s_Rigid_Geom_Loader;

//=============================================================================
// LOADER FOR THE RIGID COLOR RESOURCE
//=============================================================================

static struct rigid_color : public rsc_loader
{
    //-------------------------------------------------------------------------

    rigid_color( void ) : rsc_loader( "RIGID COLOR", ".rigidcolor" ) {}

    //-------------------------------------------------------------------------

    virtual void* PreLoad( X_FILE* FP )
    {
        MEMORY_OWNER( "RIGID COLOR DATA" );
        fileio File;
        return( File.PreLoad( FP ) );
    }

    //-------------------------------------------------------------------------

    virtual void* Resolve( void* pData )
    {
        fileio              File;
        color_info*   pRigidColor = NULL;

        File.Resolved( (fileio::resolve*)pData, pRigidColor );
        // The XBOX uses 32 bit color and PS2 is using 16 bit.

        return( pRigidColor );
    }

    //-------------------------------------------------------------------------

    virtual void Unload( void* pData )
    {
        color_info* pRigidColor=( color_info* )pData;
    #ifdef TARGET_XBOX
        delete pRigidColor->m_hColors;
    #endif
        delete pRigidColor;
    }

} s_Rigid_Color_Loader;

//=============================================================================
//  FUNCTIONS
//=============================================================================

rigid_inst::rigid_inst( void ) :
    render_inst(),
    m_pRigidColor   (NULL),
    m_nColors       (0),
    m_iColor        (0)
{
}

//=============================================================================

rigid_inst::~rigid_inst( void )
{
    if ( m_hInst.IsNonNull() )
    {
        render::UnregisterRigidInstance( m_hInst );
    }
}

//=============================================================================

s32 rigid_inst::GetNumColors( void ) const 
{
    return( m_nColors );
}

//=============================================================================

const void* rigid_inst::GetColorTable( platform PlatformType ) const
{
    if( m_pRigidColor )
    {
        if( PlatformType == PLATFORM_XBOX )
            return(( u32* )m_pRigidColor )+m_iColor;
        else if ( PlatformType == PLATFORM_PS2 )
            return(( u16* )m_pRigidColor )+m_iColor;
		else if ( PlatformType == PLATFORM_PC )
            return(( u32* )m_pRigidColor )+m_iColor;
		else
            return(0);
    }
    return NULL;
}

//=============================================================================

const void* rigid_inst::GetColorTable( void ) const
{
    if( !m_pRigidColor )
        return NULL;
#if defined(TARGET_XBOX)
    u32* pCol=( u32* )m_pRigidColor;
#elif defined(TARGET_PS2)
    u16* pCol=( u16* )m_pRigidColor;
#elif defined(TARGET_PC)
    u32* pCol=( u32* )m_pRigidColor;
#endif
    return( pCol+m_iColor );
}

//=============================================================================

void rigid_inst::SetColorTable( const void* pColorTable, s32 iColor, s32 nColors )
{
    m_pRigidColor = pColorTable;
    m_iColor      = iColor;
    m_nColors     = nColors;
}

//=============================================================================

void rigid_inst::LoadColorTable( const char* pFileName )
{
    rhandle<color_info> hRigidColor;
    hRigidColor.SetName( pFileName );
    
    color_info* pInfo = hRigidColor.GetPointer();
    if ( pInfo )
        m_pRigidColor = *pInfo;
    else
        m_pRigidColor = NULL;
}

//=============================================================================

const char* rigid_inst::GetRigidGeomName( void ) const
{
    return( m_hRigidGeom.GetName() );
}

//=============================================================================

void rigid_inst::Render( const matrix4* pL2W, u32 Flags, u64 Mask )
{
    if ( m_Alpha == 0 )
        return;

    if ( m_Alpha != 255 )
        Flags |= render::FADING_ALPHA;

    // Add a Rigid Instance
    render::AddRigidInstance( m_hInst,
                              GetColorTable(),
                              pL2W,
                              Mask,
                              Flags,
                              m_Alpha );
}

//=============================================================================

void rigid_inst::Render( const matrix4* pL2W, u32 Flags, u64 Mask, u8 Alpha )
{
    if ( Alpha == 0 )
        return;

    if ( Alpha != 255 )
        Flags |= render::FADING_ALPHA;

    // Add a Rigid Instance
    render::AddRigidInstance( m_hInst,
        GetColorTable(),
        pL2W,
        Mask,
        Flags,
        Alpha );
}

//=============================================================================

void rigid_inst::Render( const matrix4* pL2W, u32 Flags, u32 VTextureMask, s32 Alpha )
{
    if ( Alpha == 0 )
        return;

    if ( Alpha != 255 )
        Flags |= render::FADING_ALPHA;

    // Add a Rigid Instance
    render::AddRigidInstance( m_hInst,
        GetColorTable(),
        pL2W,
        GetLODMask( *pL2W ),
        VTextureMask,
        Flags,
        Alpha );
}

//=============================================================================

void rigid_inst::Render( const matrix4* pL2W, u32 Flags )
{
    // Add a Rigid Instance
    Render( pL2W, Flags, GetLODMask( *pL2W ) );
}

//=============================================================================

void rigid_inst::OnEnumProp( prop_enum& List )
{
    // Important: The Header and External MUST be enumerated first!
    List.PropEnumHeader  ( "RenderInst", "Render Instance", 0 );
    List.PropEnumExternal( "RenderInst\\File", "Resource\0rigidgeom", "Resource File", PROP_TYPE_MUST_ENUM );
    
    render_inst::OnEnumProp( List );

    List.PropEnumInt( "RenderInst\\iColor",  "iColor",  PROP_TYPE_INT | PROP_TYPE_DONT_SHOW );
    List.PropEnumInt( "RenderInst\\nColors", "nColors", PROP_TYPE_INT | PROP_TYPE_DONT_SHOW );
}

//=============================================================================

xbool rigid_inst::OnProperty( prop_query& I )
{
static u32 Count = 0;
Count ++;
    if( render_inst::OnProperty( I ) )
        return( TRUE );

    // External
    if( I.IsVar( "RenderInst\\File" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hRigidGeom.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();
            ASSERT( pString );
            
            // Clear?
            if( x_strcmp( pString, "<null>" ) == 0 )
            {
                SetUpRigidGeom( "" );
            }
            else if( pString[0] )
            {   
                // Setup
                SetUpRigidGeom( pString );
            }
    
            // if the filename has changed, this means we need to reset the vmesh mask
#if defined(X_EDITOR)
            m_VMeshMask.nVMeshes  = 0;
#endif
            m_VMeshMask.VMeshMask = 0xffffffff;
        }
        return( TRUE );
    }

    if( I.VarInt( "RenderInst\\iColor", m_iColor ) )
        return( TRUE );
    
    if( I.VarInt( "RenderInst\\nColors", m_nColors ) )
        return( TRUE );

    return( FALSE );
}

//=============================================================================


xbool rigid_inst::SetUpRigidGeom		( const char* pFileName )
{

    if( m_hInst.IsNonNull() )
    {
        render::UnregisterRigidInstance( m_hInst );
        m_hInst = HNULL;
    }

    m_hRigidGeom.SetName( pFileName );
    rigid_geom* pRigidGeom = m_hRigidGeom.GetPointer();

    if( pRigidGeom )
    {
        // Register the instance with the Render Manager
        m_hInst = render::RegisterRigidInstance( *pRigidGeom );
        return TRUE;

    }

    return FALSE;
}

//=============================================================================

#ifdef X_EDITOR
void rigid_inst::UnregiserInst( void )
{
    if ( m_hInst.IsNonNull() )
    {
        render::UnregisterRigidInstance( m_hInst );
        m_hInst = HNULL;
    }
}
#endif // X_EDITOR

//=============================================================================

#ifdef X_EDITOR
void rigid_inst::RegisterInst( void )
{
    rigid_geom* pGeom = m_hRigidGeom.GetPointer();
    if ( pGeom )
    {
        m_hInst = render::RegisterRigidInstance( *pGeom );
    }
}
#endif // X_EDITOR

//=============================================================================

// EOF
