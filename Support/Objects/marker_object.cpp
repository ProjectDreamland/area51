//==============================================================================
//
//  marker_object.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "marker_object.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Entropy.hpp"

#ifdef X_EDITOR
#include "Dictionary\global_dictionary.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#endif


//=========================================================================
// DATA
//=========================================================================
const f32 c_Sphere_Radius = 50.0f;

#ifdef X_EDITOR
static xbool s_bRenderPreview = TRUE;
#endif

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct marker_object_desc : public object_desc
{
    marker_object_desc( void ) : object_desc( object::TYPE_MARKER_OBJECT, 
                                        "Marker",
                                        "SCRIPT",
                                        
                                        #ifdef X_EDITOR   
                                        object::ATTR_RENDERABLE             | 
                                        #endif                                        
                                        object::ATTR_NULL,
                                        
                                        FLAGS_GENERIC_EDITOR_CREATE | 
                                        FLAGS_IS_DYNAMIC ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new marker_object; }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual void    OnEnumProp      ( prop_enum& List )
    {
        xbool s_bRenderPreview = TRUE;

        // Call base class
        object_desc::OnEnumProp( List ) ;

        // Debug
        List.PropEnumHeader( "Debug", "Marker debug settings.", 0 );
        List.PropEnumBool  ( "Debug\\Render Character Preview",  "If present, renders the character and animation preview.", 0 );
    }

    virtual xbool   OnProperty      ( prop_query& I )
    {
        // Call base class
        if ( object_desc::OnProperty( I ) )
            return TRUE ;

        // Debug
        if ( I.VarBool( "Debug\\Render Character Preview", s_bRenderPreview ) )
            return TRUE ;
            
        return FALSE;            
    }

    virtual s32  OnEditorRender( object& Object ) const
    {
        (void)Object;
        //object_desc::OnEditorRender( Object );
        return EDITOR_ICON_MARKER;
    }

#endif // X_EDITOR

} s_marker_object_Desc;

//==============================================================================

const object_desc& marker_object::GetTypeDesc( void ) const
{
    return s_marker_object_Desc;
}

//==============================================================================

const object_desc& marker_object::GetObjectType   ( void )
{
    return s_marker_object_Desc;
}

//==============================================================================
// marker_object
//==============================================================================

marker_object::marker_object(void)
{
#ifdef X_EDITOR
    m_AnimGroupName = -1;           // Name of animation group
    m_AnimName      = -1;           // Name of animation to play
    m_AnimFrame     = 0;            // Frame of animation to display
    m_bAnimate      = TRUE;         // Animate or show static frame
#endif
}

//==============================================================================

marker_object::~marker_object(void)
{
}

//==============================================================================

bbox marker_object::GetLocalBBox( void ) const 
{ 
    return bbox(vector3(0,0,0), c_Sphere_Radius);
}

//==============================================================================

#ifdef X_EDITOR

void marker_object::OnEnumProp( prop_enum& List )
{
    // Call base class
    object::OnEnumProp( List );
    
    // Geometry
    m_SkinInst.OnEnumProp( List );
    
    // Add animation info
    List.PropEnumHeader  ( "Anim",          "Preview animation info", PROP_TYPE_DONT_EXPORT );
    List.PropEnumExternal( "Anim\\Group",   "Resource\0animexternal", "Select the animation and animation group.", PROP_TYPE_MUST_ENUM | PROP_TYPE_EXTERNAL | PROP_TYPE_DONT_EXPORT );
    List.PropEnumString  ( "Anim\\Name",    "Name of animation to display", PROP_TYPE_DONT_EXPORT );
    List.PropEnumInt     ( "Anim\\Frame",   "Frame to display", PROP_TYPE_DONT_EXPORT );
    List.PropEnumBool    ( "Anim\\Animate", "Animate or show static frame", PROP_TYPE_MUST_ENUM | PROP_TYPE_DONT_EXPORT );
}

//==============================================================================

xbool marker_object::OnProperty( prop_query& I )
{
    // Call base class
    if( object::OnProperty( I ) )
    {
        return TRUE;
    }        
    else if( m_SkinInst.OnProperty( I ) )
    {
        // Init anim player bone count
        if( I.IsRead() == FALSE )
        {
            const anim_group* pAnimGroup = GetAnimGroupPtr();
            geom* pGeom = GetGeomPtr();
            if( ( pAnimGroup ) && ( pGeom ) )
                m_AnimPlayer.SetNActiveBones( x_min( (s32)pAnimGroup->GetNBones(), (s32)pGeom->m_nBones ) );
        }    
        return TRUE;
    }    
    else if( SMP_UTIL_IsAnimVar( I, "Anim\\Group", "Anim\\Name", m_hAnimGroup, m_AnimGroupName, m_AnimName ) )
    {
        // Init anim player
        if( I.IsRead() == FALSE )
        {        
            const anim_group* pAnimGroup = GetAnimGroupPtr();
            if( pAnimGroup )
            {
                // Set group
                m_AnimPlayer.SetAnimGroup( m_hAnimGroup );
                
                // Lookup anim index
                if( m_AnimName != -1 )
                {
                    // Get anim name
                    const char* pAnim = g_StringMgr.GetString( m_AnimName );
                    ASSERT( pAnim );
                    
                    // Get anim index
                    s32 iAnim = pAnimGroup->GetAnimIndex( pAnim );
                    if( iAnim != -1 )
                    {
                        // Start anim and force to be looping for preview
                        m_AnimPlayer.SetAnim( m_hAnimGroup, iAnim, 0.0f, 1.0f, 0 );
                        m_AnimPlayer.GetCurrAnim().SetLooping( TRUE );
                    }
                }
            }

            // Reset anim player to start frame
            ResetAnimPlayer( (f32)m_AnimFrame );
        }
                               
        return TRUE;
    }        
    else if( I.VarInt( "Anim\\Frame", m_AnimFrame, 0, 1000 ) )
    {
        if( I.IsRead() == FALSE )
        {    
            // Reset anim player to specified frame
            ResetAnimPlayer( (f32)m_AnimFrame );
        }        
        return TRUE;
    }        
    else if( I.VarBool( "Anim\\Animate", m_bAnimate ) )
    {
        if( I.IsRead() == FALSE )
        {    
            // Reset anim player to specified frame
            ResetAnimPlayer( (f32)m_AnimFrame );
        }    
        return TRUE;
    }        
    else       
    {    
        return FALSE;        
    }        
}

//==============================================================================

void marker_object::OnMove( const vector3& NewPos )
{
    // Call base class
    object::OnMove( NewPos );
    
    // Reset anim player so accumulation works
    ResetAnimPlayer( m_AnimPlayer.GetFrame() );
}

//==============================================================================

void marker_object::OnTransform( const matrix4& L2W )
{
    // Call base class
    object::OnTransform( L2W );
    
    // Reset anim player so accumulation works
    ResetAnimPlayer( m_AnimPlayer.GetFrame() );
}

//==============================================================================

void marker_object::OnRender( void )
{
    // Setup render defaults
    u32 Flags = render::CLIPPED;
    u8  Alpha = 255;
    
    // Not selected?
    if( ( GetAttrBits() & object::ATTR_EDITOR_SELECTED ) == 0 )
    {
        // Skip preview?
        if( !s_bRenderPreview )
            return;
    
        // Render transparent
        Alpha = 128;
        Flags |= render::FADING_ALPHA;
    }
           
    // Lookup geometry
    geom* pGeom = GetGeomPtr();
    if( !pGeom )
        return;
    
    // Init matrices
    matrix4* pMatrices = NULL;
    s32      nMatrices = pGeom->m_nBones;
    
    // If animation is present, get matrices from animation
    const anim_group* pAnimGroup = GetAnimGroupPtr();
    if( pAnimGroup )
    {
        // Lookup anim index
        s32 iAnim = 0;
        if( m_AnimName != -1 )
        {
            // Get anim name
            const char* pAnim = g_StringMgr.GetString( m_AnimName );
            ASSERT( pAnim );

            // Get anim index
            iAnim = pAnimGroup->GetAnimIndex( pAnim );
        }
    
        // Invalid animation?
        if( pAnimGroup->GetNBones() < pGeom->m_nBones )
        {
            // Show error message
            draw_Label( GetPosition(), XCOLOR_YELLOW, "ERROR - Incompatible\nanimation package!\nShowing bind pose." );
        }
        else if( iAnim < 0 )   // Found anim?
        {
            // Show error message
            draw_Label( GetPosition(), XCOLOR_YELLOW, "ERROR - Animation not found!\nShowing bind pose." );
        }
        else        
        {
            // Selected?
            if( GetAttrBits() & object::ATTR_EDITOR_SELECTED )
            {
                const vector3& Pos = m_AnimPlayer.GetPosition();
                
                // Render facing direction
                f32 Yaw = m_AnimPlayer.GetFacingYaw() ;
                vector3 Offset( 0, 5, 0 );
                vector3 Dir   ( 0, 0, 200 );
                Dir.RotateY( Yaw );
                draw_Marker( Pos, XCOLOR_BLUE );
                draw_Line( Pos + Offset, Pos + Offset + Dir, XCOLOR_PURPLE );
                
                // Show current frame
                draw_Label( Pos, XCOLOR_YELLOW, "Frame:%d", (s32)m_AnimPlayer.GetFrame() );
            }
            
            // Get matrices for current frame
            pMatrices = (matrix4*)m_AnimPlayer.GetBoneL2Ws();
            
            // Animate?
            if( m_bAnimate )
            {
                // Compute delta time since last render
                static f32    DeltaTime = 0.0f;
                static xtimer Timer;
                DeltaTime += Timer.ReadSec();
                Timer.Reset();
                Timer.Start();
                
                // Cap at 10 FPS
                if( DeltaTime > 0.1f )
                    DeltaTime = 0.1f;
                    
                // Advance animation                
                while( DeltaTime >= ( 1.0f / 30.0f ) )
                {
                    DeltaTime -= 1.0f / 30.0f;
                    AdvanceAnimPlayer( 1.0f / 30.0f );
                }                    
            }            
        }
    }
            
    // If no animation present, use bind pose
    if( pMatrices == NULL )
    {
        // Allocate matrices
        pMatrices = (matrix4*)smem_BufferAlloc( nMatrices * sizeof( matrix4 ) );
        if ( !pMatrices )
            return;

        // NOTE: characters are exported 180 degrees off!)
        matrix4 L2W = GetL2W();
        L2W.PreRotateY( R_180 );

        // Use identity
        for( s32 i = 0; i < nMatrices ; i++ )
            pMatrices[i] = L2W;
    }        

    // Render
    m_SkinInst.Render( &GetL2W(), 
                       pMatrices, 
                       nMatrices,
                       Flags,
                       m_SkinInst.GetLODMask( GetL2W() ),
                       xcolor( 255, 255, 255, Alpha ) );
}

//==============================================================================

render_inst* marker_object::GetRenderInstPtr( void )
{
    // Only return geometry if it's assigned so object validation doesn't fire off
    if( m_SkinInst.GetGeom() )
    {
        return &m_SkinInst;
    }
    else
    {
        return NULL;
    }
}

//==============================================================================

anim_group::handle* marker_object::GetAnimGroupHandlePtr( void )
{
    // Only return animation if it's assigned so object validation doesn't fire off
    if( m_hAnimGroup.GetPointer() )
    {
        return &m_hAnimGroup;
    }        
    else
    {    
        return NULL;
    }        
}

//==============================================================================

void marker_object::ResetAnimPlayer( f32 Frame )
{
    // Animation package not present?
    if( GetAnimGroupPtr() == NULL )
        return;

    // Reset anim player so accumulation works
    const matrix4& L2W = GetL2W();
    m_AnimPlayer.SetPosition( L2W.GetTranslation() );
    m_AnimPlayer.SetCurrAnimYaw( L2W.GetRotation().Yaw );
    m_AnimPlayer.GetCurrAnim().SetFrame( 0.0f );    

    // Advance to specified frame?
    if( m_AnimPlayer.GetAnimIndex() != -1 )
    {
        // Lookup last frame
        f32 LastFrame = (f32)m_AnimPlayer.GetNFrames() - 2.0f;
        if( LastFrame < 0.0f )
            return;
           
        // Range check requested frame            
        if( Frame < 0.0f )
            Frame = 0.0f;
        else if( Frame > LastFrame )
            Frame = LastFrame;

        // Advance animation to requested frame so pos/yaw accumulation works
        while( m_AnimPlayer.GetFrame() < Frame )
            AdvanceAnimPlayer( 1.0f / 30.0f );
    }
}

//==============================================================================

void marker_object::AdvanceAnimPlayer( f32 DeltaTime )
{
    // Animation package not present?
    if( GetAnimGroupPtr() == NULL )
        return;

    // Animation not set?
    if( m_AnimPlayer.GetAnimIndex() == -1 )
        return;

    // Advance
    vector3 DeltaPos( 0.0f, 0.0f, 0.0f );
    vector3 Pos = m_AnimPlayer.GetPosition();
    f32     Frame = m_AnimPlayer.GetFrame();

    // Accumulate position (anim player internally accumulates yaw)
    m_AnimPlayer.Advance( DeltaTime, DeltaPos );
    Pos += DeltaPos;
    m_AnimPlayer.SetPosition( Pos );

    // If animation has looped, then reset position/yaw so accumulation works
    if( m_AnimPlayer.GetFrame() < Frame )
        ResetAnimPlayer( m_AnimPlayer.GetFrame() );
}

#endif // #ifdef X_EDITOR
