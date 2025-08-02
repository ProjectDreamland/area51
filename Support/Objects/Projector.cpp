#include "Projector.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "Render\Render.hpp"

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

//=========================================================================

static struct projector_obj_desc : public object_desc
{
    projector_obj_desc( void ) : object_desc( 
            object::TYPE_PROJECTOR, 
            "Projector", 
            "RENDER",

            object::ATTR_RENDERABLE,

            FLAGS_GENERIC_EDITOR_CREATE | FLAGS_IS_DYNAMIC) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new projector_obj; }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return EDITOR_ICON_PROJECTOR;
    }

#endif // X_EDITOR

} s_ProjectorObj_Desc;

//=========================================================================

const object_desc& projector_obj::GetTypeDesc( void ) const
{
    return s_ProjectorObj_Desc;
}

//=========================================================================

const object_desc& projector_obj::GetObjectType( void )
{
    return s_ProjectorObj_Desc;
}


//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

projector_obj::projector_obj( void ) :
    m_bIsDynamic    ( FALSE   ),
    m_bIsShadow     ( TRUE    ),
    m_bIsActive     ( TRUE    ),
    m_bIsFlashlight ( FALSE   ),
    m_FOV           ( R_30    ),
    m_Length        ( 1000.0f ),
    m_hTexture      (         )
{
}

//=========================================================================

projector_obj::~projector_obj( void )
{
}

//=========================================================================

#ifndef X_RETAIL
void projector_obj::OnDebugRender  ( void )
{
    CONTEXT( "projector_obj::OnDebugRender" );

    // get the viewport dimensions
    s32 W, H;
    if( m_hTexture.GetPointer() )
    {
        // get the viewport dimensions from the texture
        texture* pTex = m_hTexture.GetPointer();
        W = pTex->m_Bitmap.GetWidth();
        H = pTex->m_Bitmap.GetHeight();
    }
    else
    {
        // Assume it's a square texture.
        W = 32;
        H = 32;
    }

    view ProjView;
    matrix4 L2W = GetL2W();
    ProjView.SetV2W(L2W);
    ProjView.SetXFOV(m_FOV);
    ProjView.SetPixelScale(1.0f);
    ProjView.SetViewport(0, 0, W, H);
    ProjView.SetZLimits(1.0f, GetLength()); // not really important

    // If this is a flashlight, don't render the icon.
    if ( !IsFlashlight() )
    {
        if( GetAttrBits() & ATTR_EDITOR_SELECTED )
        {
            draw_Frustum(ProjView, XCOLOR_RED, GetLength());
        }
        else
        {
            draw_Frustum(ProjView, XCOLOR_WHITE, GetLength());
        }
    }
}
#endif // X_RETAIL

//=========================================================================

void projector_obj::OnRender( void )
{
    if ( IsActive() )
    {
        if( IsShadow() )
            render::SetShadowProjection( GetL2W(), GetFOV(), GetLength(), GetTexture() );
        else
            render::SetTextureProjection( GetL2W(), GetFOV(), GetLength(), GetTexture() );
    }
}

//=========================================================================

bbox projector_obj::GetLocalBBox( void ) const
{
    bbox Box;

    // this will keep the bbox around the sphere
    Box.Set( vector3( -40.0f, -40.0f, -40.0f ), vector3( 40.0f, 40.0f, 40.0f ) );

    // get the viewport dimensions
    s32 W, H;
    if( m_hTexture.GetPointer() )
    {
        // get the viewport dimensions from the texture
        texture* pTex = m_hTexture.GetPointer();
        W = pTex->m_Bitmap.GetWidth();
        H = pTex->m_Bitmap.GetHeight();
    }
    else
    {
        // Assume it's a square texture.
        W = 32;
        H = 32;
    }

    // this will keep the bbox around the frustum
    view    ProjView;
    vector3 P[5];
    s32     X0,X1,Y0,Y1;
    matrix4 L2W = GetL2W();
    L2W.SetTranslation(vector3(0.0f, 0.0f, 0.0f));
    ProjView.SetV2W(L2W);
    ProjView.SetXFOV(m_FOV);
    ProjView.SetPixelScale(1.0f);
    ProjView.SetViewport(0, 0, W, H);
    ProjView.SetZLimits(1.0f, GetLength()); // not really important
    ProjView.GetViewport(X0,Y0,X1,Y1);
    P[0] = ProjView.GetPosition();
    P[1] = ProjView.RayFromScreen( (f32)X0, (f32)Y0, view::VIEW );
    P[2] = ProjView.RayFromScreen( (f32)X0, (f32)Y1, view::VIEW );
    P[3] = ProjView.RayFromScreen( (f32)X1, (f32)Y1, view::VIEW );
    P[4] = ProjView.RayFromScreen( (f32)X1, (f32)Y0, view::VIEW );
    // Normalize so that Z is Dist
    for( s32 i=1; i<=4; i++ )
    {
        P[i] *= GetLength() / P[i].GetZ();
    }
    Box.AddVerts( P, 5 );

    return Box;
}

//=========================================================================

void projector_obj::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader  ( "Projector", "Projector Properties", 0 );
    List.PropEnumBool    ( "Projector\\Dynamic",   "Can this guy move?", 0 );
    List.PropEnumBool    ( "Projector\\Shadow",    "Is this a shadow projector or light projector?", 0 );
    List.PropEnumAngle   ( "Projector\\FOV",       "FOV for the projector", 0 );
    List.PropEnumFloat   ( "Projector\\Length",    "Max distance the projector can extend.", 0 );
    List.PropEnumExternal( "Projector\\Texture",   "Resource\0xbmp\0", "Resource File", PROP_TYPE_MUST_ENUM );
}

//=============================================================================

xbool projector_obj::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
    {
    }
    else if( I.IsVar( "Projector\\Texture" ) )
    {
        // External
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hTexture.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            xstring String = I.GetVarExternal();
            if( !String.IsEmpty() )
            {
                m_hTexture.SetName( String );

                // Force the bbox to be recomputed
                OnMove( GetPosition() );
            }
        }
        return( TRUE );
    }
    else if( I.VarBool("Projector\\Dynamic", m_bIsDynamic) )
    {
    }
    else if( I.VarBool("Projector\\Shadow", m_bIsShadow) )
    {
    }
    else if( I.VarAngle("Projector\\FOV", m_FOV, R_15, R_179) )
    {
        if( !I.IsRead() )
        {
            // Force the bbox to be recomputed
            OnMove( GetPosition() );
        }
    }
    else if( I.VarFloat("Projector\\Length", m_Length, 2.0f, 10000.0f) )
    {
        if( !I.IsRead() )
        {
            // Force the bbox to be recomputed
            OnMove( GetPosition() );
        }
    }
    else
    {   
        return FALSE;
    }

    return TRUE;
}

