//=========================================================================
// INCLUDES
//=========================================================================
#include "Controller.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Path.hpp"


//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct controller_desc : public object_desc
{
    controller_desc( void ) : object_desc( 
            object::TYPE_CONTROLLER, 
            "Controller", 
            "SCRIPT",
            object::ATTR_NEEDS_LOGIC_TIME,
            FLAGS_GENERIC_EDITOR_CREATE | FLAGS_TARGETS_OBJS |
            FLAGS_IS_DYNAMIC ) {}

    //---------------------------------------------------------------------
    virtual object* Create          ( void ) { return new controller; }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32     OnEditorRender  ( object& Object ) const 
    { 
        object_desc::OnEditorRender( Object );

        return EDITOR_ICON_CONTROLLER;
    }

#endif // X_EDITOR

} s_controller_Desc;

//=========================================================================

const object_desc& controller::GetTypeDesc( void ) const
{
    return s_controller_Desc;
}

//=========================================================================

const object_desc& controller::GetObjectType( void )
{
    return s_controller_Desc;
}
//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

controller::controller( void ) : tracker()
{
    for (s32 i = 0 ; i < MAX_OBJECTS ; i++)
    {
        m_ObjectInfo[i].Pivot.Identity();
    }
}

//=============================================================================

void controller::OnEnumProp( prop_enum& List )
{
    // Call base class
    tracker::OnEnumProp( List );

    // Add properties
    List.PropEnumHeader("Controller", "Properties for the controller object", 0 );

    // Objects
    for (s32 i = 0 ; i < MAX_OBJECTS ; i++)
    {
        List.PropEnumHeader      (xfs("Controller\\Object[%d]", i),       "Object to control info.", 0 );
        List.PropEnumGuid        (xfs("Controller\\Object[%d]\\Guid", i), "Guid of object to control.", 0 ) ;

        // Res Respect the order of this. Because if not editing will be hard.
        List.PropEnumVector3     (xfs("Controller\\Object[%d]\\PivotPosition", i),    "This is a pibot point which the object can use to be relative to.", 0 ) ;
        List.PropEnumRotation    (xfs("Controller\\Object[%d]\\PivotRotation", i),    "This is a pibot point which the object can use to be relative to.", 0 ) ;
        List.PropEnumBool        (xfs("Controller\\Object[%d]\\UsePibot", i), "When this is valid the pivot point will be use. If not the pivot point will be ignore.", PROP_TYPE_MUST_ENUM ) ;
    }
}

//=============================================================================

xbool controller::OnProperty( prop_query& I )
{
    // Call base class
    if( tracker::OnProperty( I ) )
        return TRUE ;

    // Objects?
    if (I.IsSimilarPath("Controller\\Object["))
    {
        s32 Index = I.GetIndex(0) ;

        // Guid?
        if (I.VarGUID("Controller\\Object[]\\Guid", m_ObjectInfo[Index].Guid))
        {
            if (I.IsRead())
            {
            }
            else
            {
                if( m_ObjectInfo[Index].bPivot )
                {
                    object_ptr<object> pObject(m_ObjectInfo[Index].Guid) ;
                    if (pObject)
                    {
                        matrix4 InvL2W( GetL2W() );
                        InvL2W.InvertRT();
                        m_ObjectInfo[Index].Pivot = InvL2W * pObject->GetL2W();
                        //m_ObjectInfo[Index].Pivot.SetTranslation( pObject->GetPosition() - GetPosition() );
                    }
                }
            }
            return TRUE ;
        }

        if( I.VarBool("Controller\\Object[]\\UsePibot", m_ObjectInfo[Index].bPivot) )
            return TRUE ;

        if( I.IsVar( "Controller\\Object[]\\PivotPosition" )) 
        {
            if( I.IsRead() )
            {
                I.SetVarVector3( m_ObjectInfo[Index].Pivot.GetTranslation() );
            }
            else
            {
                m_ObjectInfo[Index].Pivot.SetTranslation( I.GetVarVector3() );
            }
            return TRUE;
        }

        if( I.IsVar( "Controller\\Object[]\\PivotRotation" )) 
        {
            if( I.IsRead() )
            {
                I.SetVarRotation( m_ObjectInfo[Index].Pivot.GetRotation() );
            }
            else
            {
                m_ObjectInfo[Index].Pivot.Setup  ( vector3(1,1,1), I.GetVarRotation(), m_ObjectInfo[Index].Pivot.GetTranslation() );
            }
            return TRUE;
        }

    }

    return FALSE ;
}


//=========================================================================

void controller::MoveObject( s32 Index, const matrix4& L2W ) const
{
    object_ptr<object> pObject(m_ObjectInfo[Index].Guid) ;
    if( !pObject )
        return;

    if( m_ObjectInfo[Index].bPivot )
    {
        matrix4 FL2W;
        FL2W = L2W * m_ObjectInfo[Index].Pivot;

        //-- Waiting for steve to fix things
        ///pObject->OnTransform(FL2W);

        // Waiting to fix the line above
        pObject->OnMove( FL2W.GetTranslation() );
    }
    else
    {
        //-- Waiting for steve to fix things
        ///pObject->OnTransform(L2W);
        // Waiting to fix the line above
        pObject->OnMove( L2W.GetTranslation() );
    }

    pObject->SetZone1( m_ZoneTracker.GetMainZone() );
	pObject->SetZone2( m_ZoneTracker.GetZone2() );
}


//=========================================================================

void controller::OnMove( const vector3& NewPos )
{
    // Call base class
    tracker::OnMove(NewPos) ;

    matrix4 L2W;
    L2W.Identity();

    // Update objects
    for (s32 i = 0 ; i < MAX_OBJECTS ; i++)
    {
        L2W.SetTranslation( NewPos );

        MoveObject( i, L2W );
    }
}

//=========================================================================

void controller::OnTransform( const matrix4& L2W )
{
    // Call base class
    tracker::OnTransform(L2W) ;

    // Update objects position AND rotation?
    path* pPath = GetPath() ;
    if ( (pPath) && (pPath->GetFlags() & path::FLAG_KEY_ROTATION) )
    {
        // Update objects position and rotation
        for (s32 i = 0 ; i < MAX_OBJECTS ; i++)
        {
            // Update existing objects
            MoveObject( i, L2W );
        }
    }
    else
    {
        // Just update objects position
        vector3 NewPos = L2W.GetTranslation() ;
        matrix4 FL2W;
        FL2W.Identity();
        FL2W.SetTranslation( NewPos );

        for (s32 i = 0 ; i < MAX_OBJECTS ; i++)
        {
            // Update existing objects
            MoveObject( i, FL2W );
        }
    }
}
    
//=========================================================================

void controller::OnInit( void )
{
    // Call base class
    tracker::OnInit() ;
}

//=========================================================================

void controller::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "controller::OnAdvanceLogic" );

    // Call base class for controller position
    tracker::OnAdvanceLogic(DeltaTime) ;
}

//=========================================================================
