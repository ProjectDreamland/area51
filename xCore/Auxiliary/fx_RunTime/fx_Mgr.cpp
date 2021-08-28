//==============================================================================
//
//  fx_Mgr.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "fx_Mgr.hpp"

#include "x_context.hpp"
#include "e_Draw.hpp"
#include "e_VRAM.hpp"
#include "Entropy.hpp"

#include "Auxiliary/Bitmap/aux_Bitmap.hpp"

//==============================================================================
//  STORAGE
//==============================================================================

fx_mgr              FXMgr;

fx_load_bitmap_fn*      fx_mgr::m_pLoadBitmapFn    = NULL;
fx_unload_bitmap_fn*    fx_mgr::m_pUnloadBitmapFn  = NULL;
fx_resolve_bitmap_fn*   fx_mgr::m_pResolveBitmapFn = NULL;

s32                     fx_mgr::m_NCtrlTypes = 0;
fx_ctrl_type            fx_mgr::m_CtrlType[ MAX_CTRL_TYPES ];
                    
s32                     fx_mgr::m_NElementTypes = 0;
fx_element_type         fx_mgr::m_ElementType[ MAX_ELEMENT_TYPES ];

s32                     fx_mgr::m_SpriteBudget     = S32_MAX;
s32                     fx_mgr::m_SpritesThisFrame = 0;

//==============================================================================

static  char        s_LastDirectory[ X_MAX_PATH ] = ".";
static  bbox        s_DefaultBounds;

#ifdef DEBUG_FX
        fx_debug    FXDebug;
#endif // DEBUG_FX

//==============================================================================
//  DEFAULT LOAD/UNLOAD FUNCTIONS
//==============================================================================

xbool DefaultLoad( const char*    pBitmapName,
                         xhandle& Handle )
{
    xfs XFS( "%s%s", s_LastDirectory, pBitmapName );

    xbitmap* pBitmap = new xbitmap;

    if( pBitmap->Load( (const char*)XFS ) )
    {
        vram_Register( *pBitmap );
        Handle.Handle = (s32)pBitmap;
        return( TRUE );
    }
    else
    {
        delete pBitmap;
        return( FALSE );
    }

    return FALSE;
}

//==============================================================================

void DefaultUnload( xhandle Handle )
{
    (void)Handle;
}

//==============================================================================

const xbitmap* DefaultResolve( xhandle Handle )
{
    return reinterpret_cast<xbitmap*&>(Handle);
}

//==============================================================================
//  REGISTRATION FUNCTIONS
//==============================================================================

fx_ctrl_reg::fx_ctrl_reg( const char* pName, fx_ctrl_ctor_fn* pFactoryFn )
{
//  fx_mgr::m_CtrlType[ fx_mgr::m_NCtrlTypes ].Name = pName;
//  fx_mgr::m_CtrlType[ fx_mgr::m_NCtrlTypes ].Name.MakeUpper();

    x_strcpy( fx_mgr::m_CtrlType[ fx_mgr::m_NCtrlTypes ].Name, pName );

    for( s32 i = 0; i < fx_mgr::m_NCtrlTypes; i++ )
    {
        // Cannot register the same name twice.
        ASSERT( x_stricmp( pName, fx_mgr::m_CtrlType[i].Name ) );
    }

    fx_mgr::m_CtrlType[ fx_mgr::m_NCtrlTypes ].pFactoryFn = pFactoryFn;

    fx_mgr::m_NCtrlTypes += 1;
}

//==============================================================================

fx_element_reg::fx_element_reg( const char*           pName, 
                                fx_element_ctor_fn*   pFactoryFn,
                                fx_element_memory_fn* pMemoryFn )
{
//  fx_mgr::m_ElementType[ fx_mgr::m_NElementTypes ].Name = pName;
//  fx_mgr::m_ElementType[ fx_mgr::m_NElementTypes ].Name.MakeUpper();

    x_strcpy( fx_mgr::m_ElementType[ fx_mgr::m_NElementTypes ].Name, pName );

    for( s32 i = 0; i < fx_mgr::m_NElementTypes; i++ )
    {
        // Cannot register the same name twice.
        ASSERT( x_stricmp( pName, fx_mgr::m_ElementType[i].Name ) );
    }

    fx_mgr::m_ElementType[ fx_mgr::m_NElementTypes ].pFactoryFn = pFactoryFn;
    fx_mgr::m_ElementType[ fx_mgr::m_NElementTypes ].pMemoryFn  = pMemoryFn;

    fx_mgr::m_NElementTypes += 1;
}

//==============================================================================
//  CLASS FX_MGR FUNCTIONS
//==============================================================================

extern s32 fx_Plane;
extern s32 fx_Sprite;
extern s32 fx_Sphere;
extern s32 fx_Cylinder;
extern s32 fx_SPEmitter;
extern s32 fx_ShockWave;
extern s32 fx_LinearKeyCtrl; 
extern s32 fx_SmoothKeyCtrl; 

fx_mgr::fx_mgr( void )
{
    s32 i;

    // No instances.
    for( i = 0; i < MAX_EFFECT_INSTANCES; i++ )
        m_pEffect[ i ] = NULL;

    // No definitions.
    for( i = 0; i < MAX_EFFECT_DEFINITIONS; i++ )
        m_pEffectDef[ i ] = NULL;

    m_pLoadBitmapFn    = DefaultLoad;
    m_pUnloadBitmapFn  = DefaultUnload;
    m_pResolveBitmapFn = DefaultResolve;

    s_DefaultBounds.Clear();

#ifdef DEBUG_FX
    x_memset( &FXDebug, 0, sizeof(fx_debug) / 4 );
#endif // DEBUG_FX

    // Damn linker!
    fx_Plane         = 0;
    fx_Sprite        = 0;
    fx_Sphere        = 0;
    fx_Cylinder      = 0;
    fx_SPEmitter     = 0;
    fx_ShockWave     = 0;
    fx_LinearKeyCtrl = 0;
    fx_SmoothKeyCtrl = 0;
}

//==============================================================================

fx_mgr::~fx_mgr( void )
{
    /*
    s32 i;

    // No instances.
    for( i = 0; i < MAX_EFFECT_INSTANCES; i++ )
        m_pEffect[ i ] = NULL;

    // No definitions.
    for( i = 0; i < MAX_EFFECT_DEFINITIONS; i++ )
        m_pEffectDef[ i ] = NULL;
    */
}

//==============================================================================

void fx_mgr::BindHandle( fx_handle& Handle, s32 Index )
{
    UnbindHandle( Handle );

    if( m_pEffect[Index] != NULL )
    {
        Handle.Index = Index;
        m_pEffect[Index]->AddReference();
        ASSERT( m_pEffect[Index]->GetReferences() >= 0 );
    }
    else
    {
        // Attempting to bind to an non-existant effect.
        ASSERT( FALSE );    
    }
}

//==============================================================================

void fx_mgr::UnbindHandle( fx_handle& Handle )
{   
    s32 Index = Handle.Index;

    if( (IN_RANGE( 0, Index, MAX_EFFECT_INSTANCES-1 )) && 
        (m_pEffect[Index] != NULL) )
    {
        m_pEffect[Index]->RemoveReference();
        ASSERT( m_pEffect[Index]->GetReferences() >= 0 );

        if( m_pEffect[Index]->GetReferences() == 0 )
            KillEffect( Index );
    }

    Handle.Index = -1;
}

//==============================================================================

void fx_mgr::SetBitmapFns( fx_load_bitmap_fn*    pLoad,
                           fx_unload_bitmap_fn*  pUnload,
                           fx_resolve_bitmap_fn* pResolve )
{
    ASSERT( pLoad    != NULL );
    ASSERT( pUnload  != NULL );
    ASSERT( pResolve != NULL );

    m_pLoadBitmapFn    = pLoad;
    m_pUnloadBitmapFn  = pUnload;
    m_pResolveBitmapFn = pResolve;
}

//==============================================================================

void fx_mgr::SetSpriteBudget( s32 MaxSprites )
{
    m_SpriteBudget = MaxSprites;
}

//==============================================================================

s32 fx_mgr::GetSpriteCount( void )
{
    return( m_SpritesThisFrame );
}

//==============================================================================

void fx_mgr::EndOfFrame( void )
{
    CONTEXT( "fx_mgr::EndOfFrame" );

    s32 i;
    
    // Clear logic bits for master copies.
    for( i = 0; i < MAX_EFFECT_INSTANCES; i++ )
    {
        // Is there an effect in this slot?
        if( (m_pEffect[i]) )
        {
            // Get the flags
            u32 Flags = m_pEffect[i]->m_Flags;

            // Check for deferred deletion
            if( Flags & FX_DEFERRED_DELETE )
            {
                // Increment the counter and do the delete
                if( m_pEffect[i]->m_NReferences++ > 2 )
                {
                    x_free( m_pEffect[i] );
                    m_pEffect[i] = NULL;
                }
                continue;
            }

            // Clear the logic ran flag
            if( ( Flags & FX_MASTER_COPY ) &&
                ( Flags & FX_MASTER_LOGIC) )
            {
                m_pEffect[i]->m_Flags &= ~FX_MASTER_LOGIC;
            }
        }
    }

    // Clear count of particles this frame.
    m_SpritesThisFrame = 0;
}

//==============================================================================

xbool fx_mgr::LoadEffect( const char* pEffectName, const char* pFileName )
{
    CONTEXT( "fx_mgr::LoadEffect(char*,char*)" );

    // Save off last directory.
    {
        char  Drive[ X_MAX_DRIVE ];
        char  Dir  [ X_MAX_DIR   ];

        x_splitpath( pFileName, Drive, Dir, NULL, NULL );
        x_makepath( s_LastDirectory, Drive, Dir, NULL, NULL );
    }

    xbool Result;

    X_FILE* pFile = x_fopen( pFileName, "rb" );
    if( !pFile )
        return( FALSE );

    Result = LoadEffect( pEffectName, pFile );
    x_fclose( pFile );
    return( Result );
}

//==============================================================================

xbool fx_mgr::LoadEffect( const char* pEffectName, X_FILE* pFile )
{
    CONTEXT( "fx_mgr::LoadEffect(char*,X_FILE*)" );
    MEMORY_OWNER( "EFFECT DATA" );

    s32     Size;
    s32     Read;
    s32     i;
    s32     Index;
    char*   pNameData;

    ASSERT( pFile );

    //
    // Make sure that there are no other effects using the given EffectName 
    // already loaded.  And, find a slot to put the effect in.
    //

    Index = -1;

    for( i = 0; i < MAX_EFFECT_DEFINITIONS; i++ )
    {
        if( m_pEffectDef[i] )
        {
            // If you get the following assertion failure, then you have
            // attempted to load two effects and give the same logical name.
            ASSERT( x_stricmp( pEffectName, m_pEffectDef[i]->pEffectName ) );
        }
        else
        {
            // Got an index yet?
            if( Index == -1 )
                Index = i;
        }
    }

    //
    // Read the entire effect definition.
    //
    // The "base" size is given.  Increase that by the size of the name.
    // We are going to "append" the effect name onto the end of the data.
    //
    {
        s32   Magic;
        char* pMagic = (char*)&Magic;
        pMagic[0] = 'F';
        pMagic[1] = 'X';
        pMagic[2] = '0';
        pMagic[3] = '2';

        // Read in the magic number.
        Read = x_fread( &Size, 4, 1, pFile );
        if( (Size != Magic) && 
            (Size != (s32)ENDIAN_SWAP_32(Magic)) )
        {
            ASSERT( FALSE );    // File version mismatch.
            return( FALSE );
        }

        // Read memory size (in u32s) of effect definition.
        Read = x_fread( &Size, 4, 1, pFile );
        ASSERT( Read == 1 );

        // Endian swap.
        #ifdef TARGET_GCN
        Size = ENDIAN_SWAP_32( Size );
        #endif

        // Allocate memory.
        {
            MEMORY_OWNER( "EFFECT DEFINITION" );
            m_pEffectDef[Index] = (fx_def*)x_malloc( (Size * 4) + (x_strlen(pEffectName) + 1) );
            ASSERT( m_pEffectDef[Index] );
            m_pEffectDef[Index]->TotalSize = Size;
        }

        // Read the effect definition data.
        Read = x_fread( ((s32*)m_pEffectDef[Index])+1, 4, Size-1, pFile );
        ASSERT( Read == (Size-1) );
        ASSERT( Size == m_pEffectDef[Index]->TotalSize );

        // Endian fixup.
        #ifdef TARGET_GCN
        u32* pData = (u32*)m_pEffectDef[Index];
        for( i = 0; i < Size; i++ )
            pData[i] = ENDIAN_SWAP_32( pData[i] );
        #endif

        // Append the effect name.
        m_pEffectDef[Index]->pEffectName = ((char*)m_pEffectDef[Index]) + (Size * 4);
        x_strcpy( m_pEffectDef[Index]->pEffectName, pEffectName );        
    }

    //
    // TO DO: React to magic number here.
    //
    {
    }

    //
    // TO DO: Endian fixup.
    //
    {
    }

    //
    // Pointer fixups for controllers, elements, and bitmaps, within effect 
    // definition.
    //
    {
        s32* pData;

        // Start data pointer immediately after the fixed portion of the effect 
        // definition.
        pData = (s32*)(m_pEffectDef[Index] + 1);

        // Set the pCtrlDef pointer.
        m_pEffectDef[Index]->pCtrlDef = (fx_ctrl_def**)(pData);
        pData += m_pEffectDef[Index]->NControllers;

        // Set the pElementDef pointer.
        m_pEffectDef[Index]->pElementDef = (fx_element_def**)(pData);
        pData += m_pEffectDef[Index]->NElements;

        // Set the pBmpDiffuse pointer.
        m_pEffectDef[Index]->pDiffuseMap = (xhandle*)(pData);
        pData += m_pEffectDef[Index]->NBitmaps;

        // Set the pBmpAlpha pointer.
        m_pEffectDef[Index]->pAlphaMap = (xhandle*)(pData);
        pData += m_pEffectDef[Index]->NBitmaps;

        // Populate the controller definition pointer array.
        for( i = 0; i < m_pEffectDef[Index]->NControllers; i++ )
        {
            m_pEffectDef[Index]->pCtrlDef[i] = (fx_ctrl_def*)pData;
            pData += ((fx_ctrl_def*)pData)->TotalSize;
        }

        // Populate the element definition pointer array.
        for( i = 0; i < m_pEffectDef[Index]->NElements; i++ )
        {
            m_pEffectDef[Index]->pElementDef[i] = (fx_element_def*)pData;
            pData += ((fx_element_def*)pData)->TotalSize;
        }

        // Sanity check.
        ASSERT( (pData - (s32*)m_pEffectDef[Index]) == Size );
    }

    //
    // Read in the string data which follows the numeric data.
    //
    {
        // Read memory size of string data at end of file.
        Read = x_fread( &Size, 4, 1, pFile );
        ASSERT( Read == 1 );
        
        // Endian swap.
        #ifdef TARGET_GCN
        Size = ENDIAN_SWAP_32( Size );
        #endif

        // Allocate memory for the strings.
        pNameData = (char*)x_malloc( Size );
        ASSERT( pNameData );

        // Read in the strings in a single block.
        Read = x_fread( pNameData, 1, Size, pFile );
        ASSERT( Read == Size );
    }

    //
    // Attach the controllers and elements to their registered types.  Then load
    // the bitmaps.
    //
    {
        s32   j;
        char* pNameCursor = pNameData;
        char* pCustomName;

        // For each controller, search for its registered type by name.
        for( i = 0; i < m_pEffectDef[Index]->NControllers; i++ )
        {
            for( j = 0; j < m_NCtrlTypes; j++ )
            {
            //  if( s_CtrlType[j].Name == pNameCursor )
                if( x_stricmp( m_CtrlType[j].Name, pNameCursor ) == 0 )
                {
                    m_pEffectDef[Index]->pCtrlDef[i]->TypeIndex = j;
                    break;
                }
            }
            ASSERT( j < m_NCtrlTypes );
            while( *pNameCursor )
                pNameCursor++;
            pNameCursor++;  // Skip the '\0'.
        }

        // For each element, search for its registered type by name.
        for( i = 0; i < m_pEffectDef[Index]->NElements; i++ )
        {
            try_again:

            // Watch for custom element types.
            pCustomName = x_strchr( pNameCursor, '~' );

            // Search for the name in the registered element types.
            for( j = 0; j < m_NElementTypes; j++ )
            {
            //  if( s_ElementType[j].Name == pNameCursor )
                if( x_stricmp( m_ElementType[j].Name, pNameCursor ) == 0 )
                {
                    m_pEffectDef[Index]->pElementDef[i]->TypeIndex = j;
                    break;
                }
            }

            // If not type isn't found, and the type name has a '~' (which 
            // indicates a "custom" type), then try again using just the 
            // "base" type name.
            if( (j == m_NElementTypes) && (pCustomName) )
            {
                pNameCursor = pCustomName + 1;
                goto try_again;
            }

            ASSERT( j < m_NElementTypes );
            while( *pNameCursor )
                pNameCursor++;
            pNameCursor++;  // Skip the '\0'.
        }

        // Load each bitmap.
        for( i = 0; i < m_pEffectDef[Index]->NBitmaps; i++ )
        {
            // Load diffuse texture.
            m_pLoadBitmapFn( pNameCursor, m_pEffectDef[Index]->pDiffuseMap[i] ); 

            // Skip the name.
            while( *pNameCursor )
                pNameCursor++;
            pNameCursor++;  // Skip the '\0'.

            // See if there is an alpha texture.
            if( *pNameCursor )
            {
                VERIFY( m_pLoadBitmapFn( pNameCursor, m_pEffectDef[Index]->pAlphaMap[i] ) );
            }
            else
            {
                m_pEffectDef[Index]->pAlphaMap[i] = HNULL;
            }

            // Skip the name.
            while( *pNameCursor )
                pNameCursor++;
            pNameCursor++;  // Skip the '\0'.
        }
    }

    //
    // Ditch the string data and clear the reference count.
    //
    {
        x_free( pNameData );
        m_pEffectDef[Index]->NInstances = 0;
    }

    //
    // Success!
    //
    return( TRUE );
}

//==============================================================================

xbool fx_mgr::UnloadEffect( const char* pEffectName )
{
    s32 i, j;

    // Look for the effect.
    for( i = 0; i < MAX_EFFECT_DEFINITIONS; i++ )
    {
        if( (m_pEffectDef[i]) &&
            (x_stricmp( pEffectName, m_pEffectDef[i]->pEffectName ) == 0) )
            break;
    }

    // Found it?
    if( i == MAX_EFFECT_DEFINITIONS )
        return( FALSE );

    // Is it in use?  If so, we can't proceed.
    if( m_pEffectDef[i]->NInstances > 0 )
        return( FALSE );

    // We are clear to get rid of it!
    for( j = 0; j < m_pEffectDef[i]->NBitmaps; j++ )
    {
        m_pUnloadBitmapFn( m_pEffectDef[i]->pDiffuseMap[j] );
        if( m_pEffectDef[i]->pAlphaMap[j] )
        {
            m_pUnloadBitmapFn( m_pEffectDef[i]->pAlphaMap[j] );
        }

        m_pEffectDef[i]->pDiffuseMap[j] = HNULL;
        m_pEffectDef[i]->pAlphaMap  [j] = HNULL;
    }
    x_free( m_pEffectDef[i] );
    m_pEffectDef[i] = NULL;

    // Done!
    return( TRUE );
}

//==============================================================================

void fx_mgr::CreateEffect( s32 Index, fx_def* pEffectDef )
{
    s32        Size, i;
    fx_effect* pEffect = NULL;

    MEMORY_OWNER( "EFFECT INSTANCE" );

    //
    // Create the new instance.
    // Need to determine how much memory is needed for the effect.
    //

    // Start with base size of an effect.
    Size = sizeof( fx_effect );

    // Add in space for staging area.
    Size += (pEffectDef->NSAValues * sizeof(f32));

    // Add in space for controllers and their pointers.
    Size += (pEffectDef->NControllers * (sizeof(fx_ctrl) + sizeof(fx_ctrl*)));

    // Add in space for element pointers
    Size += (pEffectDef->NElements * sizeof(fx_element*));

    // elements can have vectors and matrices, and so they must be aligned
    Size = ALIGN_16( Size );

    // Add in space for elements and memory required by particular element types,
    // taking into account the required alignment restrictions.
    for( i = 0; i < pEffectDef->NElements; i++ )
    {   
        s32 ETypeIndex = pEffectDef->pElementDef[i]->TypeIndex;
        fx_element_memory_fn* pMemoryFn = m_ElementType[ ETypeIndex ].pMemoryFn;

        s32 MemSize = pMemoryFn( *(pEffectDef->pElementDef[i]) );
        Size += ALIGN_16( MemSize );
    }

    //
    // Allocate the memory.
    //

    pEffect = (fx_effect*)x_malloc( Size );
    ASSERT( pEffect );

    fx_effect::ForceConstruct( pEffect );

    //
    // Now start setting up the structures.
    //

    // Let the effect initialize itself from here on out.
    pEffect->Initialize( pEffectDef );

    // Enter the pointer in the table of effect instances.
    m_pEffect[Index] = pEffect;

    // Track the instances of the effect definition.
    pEffectDef->NInstances++;
}

//==============================================================================

xbool fx_mgr::InitEffect( s32& Index, const char* pName )
{
    CONTEXT( "fx_mgr::InitEffect" );
    MEMORY_OWNER( "EFFECT DATA" );

    if ( !pName )
    {
        ASSERTS( FALSE, "Null effect name pointer passed into fx-mgr::InitEffect" );
        return FALSE;
    }

    s32     i;
    fx_def* pEffectDef = NULL;

    Index = -1;

    // Find the requested effect definition.
    for( i = 0; i < MAX_EFFECT_DEFINITIONS; i++ )
    {
    //  if( m_pEffectDef[i]->pEffectName == pName )
        if( (m_pEffectDef[i]) && 
            (x_stricmp( m_pEffectDef[i]->pEffectName, pName ) == 0) )
        {
            pEffectDef = m_pEffectDef[i];
            break;
        }
    }
    if( !pEffectDef )
        return( FALSE );

    // Find an available slot for the new instance.
    for( Index = 0; Index < MAX_EFFECT_INSTANCES; Index++ )
    {
        if( m_pEffect[Index] == NULL )
            break;
    }
    if( Index == MAX_EFFECT_INSTANCES )
        return( FALSE );

    //
    // If this is a NON-SINGLETON effect, OR it is a SINGLETON but the master 
    // copy hasn't been created yet, then we need to create a normal effect.
    //

    if( !(pEffectDef->Flags & FX_SINGLETON) || (pEffectDef->NInstances == 0) )
    {
        CreateEffect( Index, pEffectDef );

        // If we have now just created the master copy, then set its index and 
        // tweak its flags (remove SINGLETON, add MASTER_COPY).
        if( (pEffectDef->Flags & FX_SINGLETON) && 
            (pEffectDef->NInstances == 1) )
        {
            pEffectDef->MasterCopy     =  Index;
            m_pEffect[Index]->m_Flags &= ~FX_SINGLETON;
            m_pEffect[Index]->m_Flags |=  FX_MASTER_COPY;

            // Clear the index.  We need to find another slot for the clone.
            Index = -1;
        }
    } 

    //
    // If we are creating a singleton effect... well, let's get on with it!
    //
    if( pEffectDef->Flags & FX_SINGLETON )
    {
        fx_effect_base*  pEffect      = m_pEffect[pEffectDef->MasterCopy];
        fx_effect_clone* pEffectClone = NULL;

        // Find an available slot for the new clone instance.
        if( Index == -1 )
        {
            for( Index = 0; Index < MAX_EFFECT_INSTANCES; Index++ )
            {
                if( m_pEffect[Index] == NULL )
                    break;
            }
            if( Index == MAX_EFFECT_INSTANCES )
                return( FALSE );
        }

        // Allocate memory.
        pEffectClone = (fx_effect_clone*)x_malloc( sizeof(fx_effect_clone) );
        ASSERT( pEffectClone );

        fx_effect_clone::ForceConstruct( pEffectClone );

        // Initialize the clone.
        pEffectClone->Initialize( pEffect, pEffectDef );

        // Add the clone to the internal list.
        m_pEffect[Index] = pEffectClone;

        // Track the instances (including clones) of the effect definition.
        pEffectDef->NInstances++;
    }

    //
    // Success!
    //

    return( TRUE );
}

//==============================================================================

void fx_mgr::KillEffect( s32 Index )
{
    ASSERT( IN_RANGE( 0, Index, MAX_EFFECT_INSTANCES-1 ) );
    ASSERT( m_pEffect[ Index ] );
    ASSERT( m_pEffect[ Index ]->GetReferences() == 0 );

    fx_effect_base* pEffect = m_pEffect[ Index ];

    // There is one less instance of this effect definition.
    pEffect->m_pEffectDef->NInstances--;

    // Singleton?
    if( pEffect->m_Flags & FX_SINGLETON )
    {
        Index = pEffect->m_pEffectDef->MasterCopy;
        m_pEffect[ Index ]->RemoveReference();

        if( m_pEffect[ Index ]->GetReferences() == 0 )
        {
            ASSERT( m_pEffect[ Index ]->m_Flags & FX_MASTER_COPY );
            KillEffect( Index );
        }
    }

    pEffect->m_Flags |= FX_DEFERRED_DELETE;
    pEffect->m_NReferences = 0;
}

//==============================================================================

void fx_mgr::AdvanceLogic( fx_handle& Handle, f32 DeltaTime )
{
    CONTEXT( "fx_mgr::AdvanceLogic" );

    if( Validate( Handle ) )
        m_pEffect[ Handle.Index ]->AdvanceLogic( DeltaTime );
}

//==============================================================================

void fx_mgr::Render( const fx_handle& Handle )
{
    CONTEXT( "fx_mgr::Render" );

    if( !Validate( Handle ) )
        return;

    fx_effect_base* pEffect = m_pEffect[ Handle.Index ];

    if( pEffect->IsFinished() )
        return;

#ifdef DEBUG_FX
    // Summarize the debug options.
    FXDebug.EffectReserved  = FXDebug.EffectAxis   |
                              FXDebug.EffectCenter |
                              FXDebug.EffectVolume |
                              FXDebug.EffectBounds;
    FXDebug.ElementReserved = FXDebug.ElementAxis         |
                              FXDebug.ElementCenter       |
                              FXDebug.ElementVolume       |
                              FXDebug.ElementBounds       |
                              FXDebug.ElementWire         |
                              FXDebug.ElementSpriteCenter |
                              FXDebug.ElementSpriteCount  |
                              FXDebug.ElementCustom;
#endif // DEBUG_FX

    // Let 'er rip!
    pEffect->Render();
}

//==============================================================================

void fx_mgr::RestartEffect( fx_handle& Handle )
{
    if( Validate( Handle ) )
        m_pEffect[ Handle.Index ]->Restart();
}

//==============================================================================

xbool fx_mgr::IsFinished( const fx_handle& Handle )
{
    if( Validate( Handle ) )
    {
        return( m_pEffect[ Handle.Index ]->IsFinished() );
    }
    else
    {
        return( TRUE );
    }
}

//==============================================================================

xbool fx_mgr::IsInstanced( const fx_handle& Handle )
{
    if( Validate( Handle ) )
    {
        return( m_pEffect[ Handle.Index ]->IsInstanced() );
    }
    else
    {
        return( TRUE );
    }
}

//==============================================================================

void fx_mgr::SetScale( fx_handle& Handle, const vector3& Scale )
{
    if( Validate( Handle ) )
        m_pEffect[ Handle.Index ]->SetScale( Scale );
}

//==============================================================================

void fx_mgr::SetRotation( fx_handle& Handle, const radian3& Rotation )
{
    if( Validate( Handle ) )
        m_pEffect[ Handle.Index ]->SetRotation( Rotation );
}

//==============================================================================

void fx_mgr::SetTranslation( fx_handle& Handle, const vector3& Translation )
{
    if( Validate( Handle ) )
        m_pEffect[ Handle.Index ]->SetTranslation( Translation );
}

vector3 fx_mgr::GetTranslation( const fx_handle& Handle )
{
    if ( Validate( Handle ) )
    {
        return m_pEffect[Handle.Index]->GetTranslation();
    }
    else
    {
        return vector3( 0.0f, 0.0f, 0.0f );
    }
}

//==============================================================================

void fx_mgr::SetColor( fx_handle& Handle, const xcolor& Color )
{
    if( Validate( Handle ) )
        m_pEffect[ Handle.Index ]->SetColor( Color );
}

//==============================================================================

xcolor fx_mgr::GetColor( fx_handle& Handle )
{
    if( Validate( Handle ) )
    {
        return m_pEffect[ Handle.Index ]->GetColor();
    }

    return xcolor(0,0,0,0);
}

//==============================================================================

const bbox& fx_mgr::GetBounds( const fx_handle& Handle )
{
    if( Validate( Handle ) )
        return( m_pEffect[ Handle.Index ]->GetBounds() );
    else
        return( s_DefaultBounds );
}

//==============================================================================

void fx_mgr::SetSuspended( fx_handle& Handle, xbool Suspended )
{
    if( Validate( Handle ) )
        m_pEffect[ Handle.Index ]->SetSuspended( Suspended );
}

//==============================================================================

xbool fx_mgr::Validate( const fx_handle& Handle )
{
    if( IN_RANGE( 0, Handle.Index, MAX_EFFECT_INSTANCES-1 ) &&
        m_pEffect[ Handle.Index ] &&
        !(m_pEffect[ Handle.Index ]->m_Flags & FX_DEFERRED_DELETE) )
    {
        return( TRUE );
    }
    else
    {
        Handle.Index = -1;
        return( FALSE );
    }    
}

//==============================================================================
