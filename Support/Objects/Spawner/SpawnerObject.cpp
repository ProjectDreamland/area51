#include "SpawnerObject.hpp"
#include "Obj_mgr\obj_mgr.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "Entropy.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "Characters\\Character.hpp"
#include "Objects\Group.hpp"

/*
 *
	
Spawner specs from design:

Ideally, we would be able to set s trigger up so it would spawn a guy, and then immediately spawn another 
when that guy dies.  This way that trigger will always have 1 enemy present in the world.  The next step 
would be to make it so we could set a cap on how many enemies it could create total before shutting off.

Ideally (I know, I know I’m a dreamer) we could create a spawner, set how many enemies we want it to have 
present in the world simultaneously (always have 3 out for example) and then set a max number of enemies 
spawned if we want to (shut off after spawning its 20th enemy or whatever).  I’ve worked with spawners 
like this in the past and they are great (which is why I’ve been pushing for this functionality for quite 
some time).

-C

*
*/

//===========================================================================

static struct spawner_object_desc : public object_desc
{
    spawner_object_desc( void ) : object_desc( 
        object::TYPE_SPAWNER_OBJECT, 
        "Object Spawner",
        "SCRIPT",

        object::ATTR_NEEDS_LOGIC_TIME,

        FLAGS_IS_DYNAMIC   |
        FLAGS_TARGETS_OBJS |
        FLAGS_GENERIC_EDITOR_CREATE ) {}
    
    //-------------------------------------------------------------------------
    
    virtual object* Create          ( void ) { return new spawner_object; }

#ifdef X_EDITOR
    virtual s32     OnEditorRender  ( object& Object ) const { Object.OnDebugRender(); return EDITOR_ICON_SPAWNER; }
#endif // X_EDITOR

} s_Spawner_Object_Desc;

//===========================================================================

spawner_object::spawner_object() :
    m_MaxInWorldAtATime( 1 ),
    m_MaxAbleToCreate( -1 ),
    m_bActive( FALSE ),
    m_fDelayBetweenSpawns(  5.f ),
    m_Timer( 0.f ),
    m_TotalObjectsSpawned( 0 ),
    m_CurrentAliveSpawnedObjects( 0 ),
    m_TemplateID( -1 ),
    m_ActivateObject( NULL_GUID ),
    m_SpawnGroup( NULL_GUID )
{
}

//===========================================================================

spawner_object::~spawner_object()
{
}

//===========================================================================

const object_desc& spawner_object::GetTypeDesc( void ) const
{
    return s_Spawner_Object_Desc;
}

//=========================================================================

const object_desc&  spawner_object::GetObjectType   ( void )
{
    return s_Spawner_Object_Desc;
}

//===========================================================================

void spawner_object::OnInit( void )
{
    SetAttrBits( GetAttrBits() &~ ATTR_NEEDS_LOGIC_TIME );
}

//===========================================================================

void spawner_object::OnEnumProp( prop_enum& rList )
{
    // Base class enumeration.
    object::OnEnumProp( rList );

    // Set how many enemies we want it to have present in the world simultaneously
    rList.PropEnumHeader( "Spawner", "Properties of the spawner", PROP_TYPE_HEADER );
    rList.PropEnumGuid( "Spawner\\Object To Activate", "Object to activate on Guid.", 0 );
    rList.PropEnumGuid( "Spawner\\Spawn Group", "Group to put spawned object into.", 0 );
    rList.PropEnumBool( "Spawner\\Active at Start", "Is this spawner active when we start?", 0 );
    rList.PropEnumInt( "Spawner\\Max at once", 
                        "How many objects we want it to have present in the world simultaneously.  I have arbitrarily capped this at 10.  If you want more, let me know.", 0 );

    // Set a max number of enemies spawned
    rList.PropEnumInt( "Spawner\\Max Objects Spawned", "Total number of objects that this spawner can create.  If set to -1, it will always create.", 0 );
    rList.PropEnumFloat( "Spawner\\Delay Between Spawns", "How many seconds between spawns?", 0 );

    // Finally, the spawned object.
    rList.PropEnumFileName(    "Spawner\\BlueprintPath", 
                                "Spawned object blueprint (*.bpx)|*.bpx|All Files (*.*)|*.*||", 
                                "This is the actual file name for the blueprint", PROP_TYPE_MUST_ENUM );

//    m_GVarInterface.OnEnumProp( rList );
}

//===========================================================================

xbool spawner_object::OnProperty( prop_query& rPropQuery )
{
/*
    if ( m_GVarInterface.OnProperty( rPropQuery ) )
    {
        return TRUE;
    }
*/

    // Base class properties.
    if ( object::OnProperty( rPropQuery ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarGUID( "Spawner\\Object To Activate", m_ActivateObject ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarGUID( "Spawner\\Spawn Group", m_SpawnGroup ) )
    {
        return TRUE;
    }
    
    if ( rPropQuery.VarInt( "Spawner\\Max at once", m_MaxInWorldAtATime, 0, 10  ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarInt( "Spawner\\Max Objects Spawned", m_MaxAbleToCreate, -1 ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarBool( "Spawner\\Active at Start", m_bActive ) )
    {
        if ( m_bActive )
        {
            SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );
        }
        else
        {
            SetAttrBits( GetAttrBits() &~ object::ATTR_NEEDS_LOGIC_TIME );
        }
        return TRUE;
    }

    if ( rPropQuery.VarFloat( "Spawner\\Delay Between Spawns", m_fDelayBetweenSpawns, 1.f ) )
    {
        return TRUE;
    }

    if( rPropQuery.IsVar( "Spawner\\BlueprintPath" ))
    {
        if( rPropQuery.IsRead() )
        {
            if ( m_TemplateID < 0 )
            {
                rPropQuery.SetVarFileName( "", 256 );
            }
            else
            {
                rPropQuery.SetVarFileName( g_TemplateStringMgr.GetString( m_TemplateID ), 256 );
            }
        }
        else
        {
            if ( x_strlen( rPropQuery.GetVarFileName() ) > 0 )
            {
                m_TemplateID = g_TemplateStringMgr.Add( rPropQuery.GetVarFileName() );
            }
        }

        return TRUE;
    }

    
    return FALSE;
}

//===========================================================================

void spawner_object::OnActivate( xbool bFlag )
{
    if ( bFlag )
    {
        SetAttrBits( GetAttrBits() | ATTR_NEEDS_LOGIC_TIME );
        m_Timer = m_fDelayBetweenSpawns;
        m_bActive = TRUE;
    }
    else
    {
        SetAttrBits( GetAttrBits() &~ ATTR_NEEDS_LOGIC_TIME );
        m_bActive = FALSE;
    }
}

//===========================================================================

void spawner_object::OnAdvanceLogic( f32 DeltaTime )
{
    // If we have spawned the maximum number of objects in the world that we are allowed,
    // shut down the spawner.
    if ( m_CurrentAliveSpawnedObjects == m_MaxInWorldAtATime )
    {
        SetAttrBits( GetAttrBits() &~ ATTR_NEEDS_LOGIC_TIME );
        return;
    }

    // Increment the timer.
    m_Timer += DeltaTime;
    if ( m_Timer < m_fDelayBetweenSpawns )
    {
        return;
    }

    if ( m_MaxAbleToCreate >= 0 && m_TotalObjectsSpawned >= m_MaxAbleToCreate )
    {
//        m_GVarInterface.ExecuteVariableLogic();
        if ( m_ActivateObject != NULL_GUID )
        {
            object_ptr<object> ObjPtr( m_ActivateObject );
            if ( ObjPtr.IsValid() )
            {
                ObjPtr.m_pObject->OnActivate( TRUE );
            }
        }
        g_ObjMgr.DestroyObject( GetGuid() );
        return;
    }
    // Time to spawn an object.
    SpawnObject();
    m_Timer = 0.f;
}

//===========================================================================

void spawner_object::SpawnObject( void )
{
    m_TotalObjectsSpawned++;
    m_CurrentAliveSpawnedObjects++;
    guid CreatedGuid = g_TemplateMgr.CreateSingleTemplate( g_TemplateStringMgr.GetString( m_TemplateID ), GetPosition(), GetL2W().GetRotation(), GetZone1(), GetZone2() );
    
    object* pObject = g_ObjMgr.GetObjectByGuid( CreatedGuid );

    if ( pObject && pObject->IsKindOf( character::GetRTTI() ) )
    {
        character* pCharacter = ( character* ) pObject;
        pCharacter->SetNotifyOnDeathGuid( GetGuid() );
    }

    if ( NULL_GUID != m_SpawnGroup)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_SpawnGroup );
        if (NULL != pObj)
        {
            if (pObj->IsKindOf( group::GetRTTI() ))
            {   
                group& Group = group::GetSafeType( *pObj );
                Group.AddGuid( CreatedGuid );
            }
        }
    }
}


//===========================================================================

void spawner_object::OnSpawnedObjectKill( object* pObject )
{
    ( void )pObject;

    m_CurrentAliveSpawnedObjects--;

    if ( m_CurrentAliveSpawnedObjects < m_MaxInWorldAtATime && m_bActive )
    {
        SetAttrBits( GetAttrBits() | ATTR_NEEDS_LOGIC_TIME );
//        m_Timer = m_fDelayBetweenSpawns;
    }
}

//===========================================================================

#ifndef X_RETAIL
void spawner_object::OnDebugRender( void )
{
    CONTEXT( "spawner_object::OnRender" );

#ifdef X_EDITOR
    if( GetAttrBits() & ATTR_EDITOR_SELECTED )
    {
        object_ptr<object> ObjPtr( m_ActivateObject );
        if ( ObjPtr.IsValid() )
        {
            draw_Line( GetPosition(), ObjPtr.m_pObject->GetPosition(), XCOLOR_PURPLE );
            draw_Label( ObjPtr.m_pObject->GetPosition(), XCOLOR_WHITE, "Activate" );
        }

        draw_BBox( GetBBox() );
    }
#endif // X_EDITOR
}
#endif // X_RETAIL

//===========================================================================

#ifdef X_EDITOR
void spawner_object::EditorPreGame( void )
{
    matrix4 rMat;
    rMat.Identity();

    if ( m_TemplateID >= 0 )
    {
        guid ObjGuid = g_TemplateMgr.EditorCreateSingleTemplateFromPath( g_TemplateStringMgr.GetString( m_TemplateID ), rMat.GetTranslation(), rMat.GetRotation(), -1, -1 );
        g_ObjMgr.GetObjectByGuid( ObjGuid )->EditorPreGame();
        g_ObjMgr.DestroyObject( ObjGuid );
    }
}
#endif // X_EDITOR
