#include "controller.hpp"
#include "element_mesh.hpp"
#include "effect.hpp"

namespace fx_core
{

//============================================================================
// element REGISTRATION
//============================================================================
element*         s_MeshFactory    ( void )  { return (element*)new element_mesh; }
static elem_reg  s_MeshKeyReg               ( "mesh", s_MeshFactory );

//============================================================================
// Constructor

element_mesh::element_mesh( )
: element()
{
    // don't export meshes yet!
    m_Export = false;
    m_pType = "Mesh";
}

element_mesh::element_mesh( const element_mesh& mesh )
 :  element( mesh ),
    m_MeshName( mesh.m_MeshName ),
    m_MeshViewer()
{
    if( !m_MeshName.IsEmpty() )
    {
        LoadMesh();
    }
}

element_mesh::~element_mesh()
{
    m_MeshViewer.Unload();
}

//============================================================================

void element_mesh::Create( const char* pElementID, effect& Effect )
{
    // Assert that the ID not be NULL
    ASSERT( pElementID );

    m_ID        = pElementID;
    m_pEffect   = &Effect;

    m_pScale =       new ctrl_linear;
    m_pRotation =    new ctrl_linear;
    m_pTranslation = new ctrl_linear;
    m_pColor =       new ctrl_linear;
    m_pAlpha =       new ctrl_linear;
    
    // fix the color scalar
    m_pColor->SetScalar( 1.0f/255.0f );
    m_pAlpha->SetScalar( 1.0f/255.0f );

    m_pScale->SetNumFloats      (3);
    m_pRotation->SetNumFloats   (3);
    m_pTranslation->SetNumFloats(3);
    m_pColor->SetNumFloats      (3);
    m_pAlpha->SetNumFloats      (1);

    Effect.AddController( m_pScale       );
    Effect.AddController( m_pRotation    );
    Effect.AddController( m_pTranslation );
    Effect.AddController( m_pColor       );
    Effect.AddController( m_pAlpha       );

    // add myself to the element list
    Effect.AddElement( this );
}

//============================================================================

element* element_mesh::Duplicate( void )
{
    element_mesh*   pNew    = new element_mesh( *this );
    
    pNew->m_pScale =        (ctrl_linear*)m_pScale->CopyOf();
    pNew->m_pRotation =     (ctrl_linear*)m_pRotation->CopyOf();
    pNew->m_pTranslation =  (ctrl_linear*)m_pTranslation->CopyOf();
    pNew->m_pColor =        (ctrl_linear*)m_pColor->CopyOf();
    pNew->m_pAlpha =        (ctrl_linear*)m_pAlpha->CopyOf();

    pNew->MakeDuplicateName( m_ID );

    m_pEffect->AddController( pNew->m_pScale       );
    m_pEffect->AddController( pNew->m_pRotation    );
    m_pEffect->AddController( pNew->m_pTranslation );
    m_pEffect->AddController( pNew->m_pColor       );
    m_pEffect->AddController( pNew->m_pAlpha       );

    // all elements are copied...but use new controllers
    return (element*)pNew;
}

//============================================================================

void element_mesh::PostCopyPtrFix( void )
{
}


//============================================================================

void element_mesh::Render( f32 T )
{
    // Only render if we exist
    if( ExistsAtTime( T ) && (!m_Hide) )
    {
        matrix4 L2W;
        xcolor  Color;

        GetL2WAtTime  ( T, L2W   );
        GetColorAtTime( T, Color );

        // Set L2W
        g_pd3dDevice->SetTransform( D3DTS_WORLD, (D3DMATRIX*)&L2W );

        m_MeshViewer.Render( Color );

        // Set L2W
        draw_SetL2W( L2W );

        // Render element bbox
        RenderBBox( T );

        // Reset L2W
        draw_ClearL2W();
    }

    // Render the translation path of the object
    RenderTrajectory();
}

//============================================================================

xbool element_mesh::GetProperty( s32 Idx, s32 T, xcolor& UIColor, xstring& Name, xstring& Value, xbool& IsDisabled, base::prop_type& Type )
{
    xcolor  HeaderColor     ( 119, 128, 144 );
    xcolor  ItemColor       ( 176, 176, 176 );

    // give the base class a chance to deal with the basics
    if ( element::GetProperty( Idx, T, UIColor, Name, Value, IsDisabled, Type ) == FALSE )
    {
        // return our specific data here
        switch( Idx )
        {
            case 26:
                //============================================================================
                Name.Format( "Mesh" );
                Value.Format( "" );
                Type        = PROP_HEADER;
                UIColor     = HeaderColor;
                IsDisabled  = FALSE;
                return TRUE;
            case 27:
                //============================================================================
                Name.Format( "Mesh\\Mesh" );
                Value = m_MeshName;
                Type        = PROP_FILENAME;
                UIColor     = ItemColor;
                IsDisabled  = FALSE;
                return TRUE;
            default:
                return FALSE;
        }
    }
    else
    {
        return TRUE;
    }

    // keep the compilers happy
    return FALSE;
}

//============================================================================

xbool element_mesh::OnPropertyChanged( s32 T, xstring& Field, xstring& Value )
{
    xbool Dirty = FALSE;

    // non-animatable properties first
    if( x_strstr(Field, "Mesh\\Mesh") )
    {
        Dirty       = TRUE;
        m_MeshName  = Value;
        LoadMesh();
    }
    else
    {
        Dirty = element::OnPropertyChanged( T, Field, Value );
    }

    //m_pEffect->GetDocument()->UpdateAllViews( NULL );

    return Dirty;
}

//============================================================================
void element_mesh::Save( igfmgr& Igf )
{
    hfield NewGrp = Igf.AddGroup( "Element" );
    Igf.EnterGroup( NewGrp );
    {
        Igf.AddString( "Type", "MESH", "Type of element" );

        // add the base stuff
        element::Save( Igf );

        // add our own stuff
        Igf.AddString( "Mesh", m_MeshName );
    }

    // set the group back to wherever we were
    Igf.ExitGroup();
}

//============================================================================
void element_mesh::Load( igfmgr& Igf )
{
    // base data first
    element::Load( Igf );
    
    // Get the mesh filename and load it
    m_MeshName = Igf.GetString( "Mesh" );
    LoadMesh();
}

//============================================================================
void element_mesh::LoadMesh( void )
{
    if( m_MeshName.IsEmpty() )
    {
        m_MeshViewer.Unload();
    }
    else
    {
        // Make sure the file exists before loading it
        X_FILE* pFile = x_fopen( m_MeshName, "r" );

        if( pFile )
        {
            x_fclose( pFile );
            m_MeshViewer.Load( m_MeshName );
        }
    }
}

//============================================================================
// Export data
void element_mesh::ExportData( export::fx_elementhdr& ElemHdr, 
                               xstring& Type,
                               xbytestream& Stream, 
                               s32 ExportTarget )
{
}

} // namespace fx_core
