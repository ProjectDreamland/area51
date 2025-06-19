
#include "Manipulator.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "EventMgr\EventMgr.hpp"

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

//=========================================================================

static struct manipulator_desc : public object_desc
{
    manipulator_desc( void ) : object_desc( 
            object::TYPE_MANIPULATOR, 
            "Manipulator", 
            "SCRIPT",

            object::ATTR_NEEDS_LOGIC_TIME,

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_TARGETS_OBJS          |
            FLAGS_IS_DYNAMIC ) {}

    //---------------------------------------------------------------------
    virtual object* Create          ( void ) { return new manipulator; }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32     OnEditorRender  ( object& Object ) const 
    { 
        object_desc::OnEditorRender( Object );
        return EDITOR_ICON_GEAR; 
    }

#endif // X_EDITOR

} s_Manipulator_Desc;

//=========================================================================

const object_desc& manipulator::GetTypeDesc( void ) const
{
    return s_Manipulator_Desc;
}

//=========================================================================

const object_desc& manipulator::GetObjectType( void )
{
    return s_Manipulator_Desc;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

manipulator::manipulator( void )
{
    m_Flags  = FLAGS_DESTROY_AFTER_PLAYING;
    m_nGuids = 0;
}

//=============================================================================

void manipulator::OnInit( void )
{
    // No logic from the start
    SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );
}

//=============================================================================

void manipulator::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );
    
    List.PropEnumHeader( "Manipulator", "This are the properties for the manipulator", 0 );

    List.PropEnumButton  ( "Manipulator\\UpdateObjects", "This button is use to update the transformation of the object that the objects dependent.", PROP_TYPE_DONT_SAVE );

    List.PropEnumInt     ( "Manipulator\\BoneNumberSave", "This indicates how many bones does the animation has there for how many guids must be set", PROP_TYPE_DONT_SHOW );

    List.PropEnumExternal( "Manipulator\\Anim",           "Resource\0anim\0",       "Resource File", PROP_TYPE_MUST_ENUM );
    
    List.PropEnumHeader  ( "Manipulator", "This is the properties that are unique for the anim surface", 0 );

    List.PropEnumExternal( "Manipulator\\Audio Package",  "Resource\0audiopkg\0",   "The audio package associated with this anim surface.", 0 );

    List.PropEnumBool    ( "Manipulator\\Active",       "This will indicate whether the object is active from the begining or not.", 0 );
    List.PropEnumBool    ( "Manipulator\\Loop",         "Tells that the animation is going to loop for ever.", 0 );

    List.PropEnumInt     ( "Manipulator\\BoneCount",    "This indicates how many bones does the animation has there for how many guids must be set", PROP_TYPE_READ_ONLY );

    for( s32 i=0; i<m_nGuids; i++ )
    {
        List.PropEnumGuid( xfs("Manipulator\\Guid[%d]",i), "Object which bone x is asociated with.", 0 );
    }
}

//=============================================================================

xbool manipulator::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
        return( TRUE );

    if( I.VarInt( "Manipulator\\Bone Number", m_nGuids ) )
    {
        ASSERT( I.IsRead() == TRUE);
    }
    else if( I.IsVar( "Manipulator\\UpdateObjects" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Update" );
        }
        else
        {
            UpdateAnimL2W();
            UpdateObjects();
        }
    }
    else if( I.IsVar( "Manipulator\\Active" ) )
    {
        if( I.IsRead() ) I.SetVarBool( (m_Flags&FLAGS_ACTIVE) == FLAGS_ACTIVE );
        else             m_Flags = I.GetVarBool()?m_Flags|FLAGS_ACTIVE:m_Flags&~FLAGS_ACTIVE;

        // Make sure to set it active if we need to activated
        if(m_Flags&FLAGS_ACTIVE) 
            SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );
        else
            SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );            
    }
    else if( I.IsVar( "Manipulator\\Loop" ) )
    {
        if( I.IsRead() ) I.SetVarBool( (m_Flags&FLAGS_LOOP) == FLAGS_LOOP );
        else             m_Flags = I.GetVarBool()?m_Flags|FLAGS_LOOP:m_Flags&~FLAGS_LOOP;
    }
    else if( I.VarInt( "Manipulator\\Bone Number", m_nGuids ) )
    {
    }
    else if( I.VarGUID( "Manipulator\\Guid[]", m_Guid[I.GetIndex(0)] ))
    {
    }
    else if( I.IsVar( "Manipulator\\Anim" ) )
    {
        if( I.IsRead() )
        {
            if( m_hAnimGroup.IsLoaded() )
            {
                I.SetVarExternal( m_hAnimGroup.GetName(), RESOURCE_NAME_SIZE );
            }
            else
            {
                I.SetVarExternal( "\0", RESOURCE_NAME_SIZE );
            }
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();
            if( pString[0] )
            {
                m_hAnimGroup.SetName( pString );
                anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
                
                if( pAnimGroup )
                {
                    m_Anim.SetAnimGroup( m_hAnimGroup );
                    m_Anim.SetAnim( 0, TRUE );

                    // Set how many bones are avariable.
                    m_nGuids = m_Anim.GetNBones();
                    ASSERT( m_nGuids <= MAX_ANIMATED_OBJECTS );
                }
            }
        }
    }
    else if( I.IsVar( "Manipulator\\Audio Package" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                if( xstring(pString) == "<null>" )
                {
                    m_hAudioPackage.SetName( "" );
                }
                else
                {
                    m_hAudioPackage.SetName( pString );                

                    // Load the audio package.
                    if( m_hAudioPackage.IsLoaded() == FALSE )
                        m_hAudioPackage.GetPointer();
                }
            }
        }
    }
    else
    {
        return FALSE;
    }
        
    return( TRUE );
}

//=========================================================================

#ifndef X_RETAIL
void manipulator::OnDebugRender  ( void )
{
    CONTEXT( "manipulator::OnRender" );

#ifdef X_EDITOR

    if( GetAttrBits() & ATTR_EDITOR_SELECTED )
    {
        // Draw a line to all the objects that it refers to
        for( s32 i=0; i<m_nGuids; i++ )
        {
            if( m_Guid[i] == 0 )
                continue;

            object* pObject = g_ObjMgr.GetObjectByGuid( m_Guid[i] );
       
            if( pObject ) 
            {
                draw_Line( GetPosition(), pObject->GetBBox().GetCenter(), xcolor( 255,0,0,255) );
            }
        }
    }
#endif // X_EDITOR
}
#endif // X_RETAIL

//=========================================================================

void manipulator::UpdateAnimL2W( void )
{
    anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
    if( pAnimGroup == NULL ) 
        return;

    matrix4 MBind;

    MBind.Identity();
    MBind.SetTranslation( pAnimGroup->GetBoneBindInvMatrix(0).GetTranslation() );
    MBind.RotateY( R_180 ); // Fix max bug
    MBind = GetL2W() * MBind;

    m_Anim.SetL2W( MBind );
}

//=========================================================================

void manipulator::OnMove( const vector3& NewPos )
{
    object::OnMove( NewPos );

    UpdateAnimL2W();
    UpdateObjects();
}

//=========================================================================

void manipulator::OnTransform( const matrix4& L2W )
{
    object::OnTransform( L2W );

    UpdateAnimL2W();
    UpdateObjects();
}

//=========================================================================

void manipulator::UpdateObjects( void )
{
    // If nothing is loaded there is nothing to do
    if( m_hAnimGroup.IsLoaded() == FALSE )
        return;

    //
    // Compute the Local to world matrix
    //
    const matrix4* pMatrix = m_Anim.GetBoneL2Ws(FALSE) ;
    if (!pMatrix)
        return ;

    //
    // Move the objects
    //
    matrix4 MaxBug;
    MaxBug.Identity();
    MaxBug.RotateY( R_180 );
    MaxBug.RotateX( R_90 );

    for( s32 i=0; i<m_nGuids; i++ )
    {
        if( m_Guid[i] == 0 )
            continue;

        object* pObject = g_ObjMgr.GetObjectByGuid( m_Guid[i] );
       
        if( pObject ) 
        {
            pObject->OnTransform( pMatrix[i] * MaxBug ); 
        }
    }
}

//=========================================================================

void manipulator::OnAdvanceLogic( f32 DelaTime )
{
    CONTEXT( "anim_surface::OnAdvanceLogic" );

    // If nothing is loaded there is nothing to do
    if( m_hAnimGroup.IsLoaded() == FALSE )
        return;

    //
    // Advance the Animation
    //
    if( m_hAnimGroup.IsLoaded() )
    {
        m_Anim.Advance( DelaTime );
        UpdateAnimL2W();
    }

    // Kill the guy if it is not going to loop
    if( !(m_Flags&FLAGS_LOOP) && m_Anim.IsAtEnd() )
    {
        g_ObjMgr.DestroyObject( GetGuid() );
        return;
    }

    //
    // Handle events in the animation
    //
    g_EventMgr.HandleSuperEvents( m_Anim, this );

    //
    // Now update the objects
    //
    UpdateObjects();
}

//=========================================================================

void manipulator::OnActivate( xbool Flag )
{
    (void)Flag;

    SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );
    m_Flags |= FLAGS_ACTIVE;
}
