#include "Level.hpp"
#include "Obj_mgr\obj_mgr.hpp"
#include "Objects\Render\RenderInst.hpp"

//=============================================================================

level::level( void )
{
}

//=============================================================================

level::~level( void )
{
}

//=============================================================================

void level::Open( const char* pFileName, xbool DoLoad )
{
    m_DoLoad = DoLoad;
    
    if( m_DoLoad == TRUE )
    {
        m_TextIn.OpenFile( pFileName );
    }
    else
    {
        m_TextOut.OpenFile( pFileName );
    }
}

//=============================================================================

void level::Close( void )
{
    if( m_DoLoad == TRUE )
    {
        m_TextIn.CloseFile();
    }
    else
    {
        m_TextOut.CloseFile();
    }
}

//=============================================================================

void level::Load( void )
{
    if( m_DoLoad != TRUE )
        x_throw( "File is not opened for loading" );

    while( m_TextIn.IsEOF() == FALSE )
    {
        m_TextIn.ReadHeader();
    
        if( x_stricmp( m_TextIn.GetHeaderName(), "Object" ) == 0 )
        {
            char ObjTypeName[ 256 ];
            guid ObjGuid;
    
            m_TextIn.ReadFields();
            m_TextIn.GetField( "Type:s", ObjTypeName );
            m_TextIn.GetGuid ( "Guid", ObjGuid );

            if( x_stricmp( ObjTypeName, "Light" ) == 0 )
                continue;
            
            // Create a new object
            g_ObjMgr.CreateObject( ObjTypeName, ObjGuid );
            
            // Initialize the object
            object* pObject = g_ObjMgr.GetObjectByGuid( ObjGuid );
            
            if( pObject != NULL )
            {
                pObject->OnLoad( m_TextIn );
            }
        }
    }
}

//=============================================================================

#if defined( X_EDITOR )

void level::Save( guid Guid )
{
    if( m_DoLoad != FALSE )
        x_throw( "File is not opened for saving" );

    object* pObject = g_ObjMgr.GetObjectByGuid( Guid );
    
    if( pObject != NULL )
    {
        m_TextOut.AddHeader( "Object", 1 );
        m_TextOut.AddField ( "Type:s", pObject->GetTypeDesc().GetTypeName() );
        m_TextOut.AddGuid  ( "Guid", Guid );
        m_TextOut.AddEndLine();
        
        pObject->OnSave( m_TextOut );
    }
}

#endif // defined( X_EDITOR )

//=============================================================================

void level::SetRigidColor( const char* pFileName )
{
    // loop through all of the objects, and if the need to have rigid
    // colors associated with them, then do that now
    s32 i;
    for( i = 0; (object::type)i < object::TYPE_END_OF_LIST; i++ )
    {
        const object_desc* pDesc = g_ObjMgr.GetTypeDesc( (object::type)i );
        if( pDesc && pDesc->IsBurnVertexLighting() )
        {
            slot_id SlotID = g_ObjMgr.GetFirst( (object::type)i );
            while( SlotID != SLOT_NULL )
            {
                object*         pObject = g_ObjMgr.GetObjectBySlot( SlotID );
                render_inst*    pInst   = pObject->GetRenderInstPtr();
                
                if( pInst )
                    pInst->LoadColorTable( pFileName );
                
                SlotID = g_ObjMgr.GetNext( SlotID );
            }
        }
    }
}

//=============================================================================

