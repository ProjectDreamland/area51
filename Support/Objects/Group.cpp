
#include "Group.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "Objects\Actor\Actor.hpp"
#include "TriggerEx\TriggerEx_Object.hpp"

//=============================================================================
// CONSTANTS
//=============================================================================

//=============================================================================
// SHARED
//=============================================================================

//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

//=============================================================================

static struct group_desc : public object_desc
{
    group_desc( void ) : object_desc( 
        object::TYPE_GROUP, 
        "Group", 
        "SCRIPT",
        object::ATTR_NULL,
        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_TARGETS_OBJS |
        FLAGS_IS_DYNAMIC            )   {}

        //-------------------------------------------------------------------------

        virtual object* Create( void ) { return new group; }

#ifdef X_EDITOR

        virtual s32     OnEditorRender  ( object& Object ) const 
        { 
            // Call default render
            object_desc::OnEditorRender( Object );        
            return EDITOR_ICON_GROUP; 
        }

#endif // X_EDITOR

} s_Group_Desc;

//=============================================================================

group::group()
{
    
}

//=============================================================================

group::~group()
{

}

//=============================================================================

const object_desc& group::GetTypeDesc( void ) const
{
    return s_Group_Desc;
}

//=============================================================================

const object_desc& group::GetObjectType( void )
{
    return s_Group_Desc;
}

//=============================================================================


void group::OnEnumProp      ( prop_enum&    List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader  ( "Group",           "Group Object Properties", 0 );

    s32 i;
    s32 nChildren = m_Child.GetCount();

    List.PropEnumButton   ( "Group\\Refresh", "Refresh the child list", PROP_TYPE_DONT_SHOW |
                                                                   PROP_TYPE_DONT_SAVE | 
                                                                   PROP_TYPE_DONT_EXPORT |
                                                                   PROP_TYPE_DONT_SAVE_MEMCARD );

    List.PropEnumButton   ( "Group\\Add Child", 
                       "Press the button to add a new target.",
                       PROP_TYPE_MUST_ENUM );

    u32 ScriptingProperties =   PROP_TYPE_MUST_ENUM | 
                                PROP_TYPE_DONT_SHOW | 
                                PROP_TYPE_DONT_SAVE |
                                PROP_TYPE_DONT_SAVE_MEMCARD |
                                PROP_TYPE_DONT_EXPORT |
                                PROP_TYPE_EXPOSE
                                ;
    List.PropEnumGuid     ( "Group\\Insert Child", "", ScriptingProperties );

    List.PropEnumGuid     ( "Group\\Delete Child", "", ScriptingProperties );

    List.PropEnumBool    ( "Group\\Destroy All", "", ScriptingProperties );

    List.PropEnumBool    ( "Group\\Is Empty", "", ScriptingProperties );

    List.PropEnumBool    ( "Group\\Has Living Actors", "", ScriptingProperties );

    List.PropEnumInt     ( "Group\\Child Count", "", ScriptingProperties );


    for (i=0;i<nChildren;i++)
    {
        const char* pTypeName = GetObjectName( m_Child[i].m_Object.GetGuid() );
        big_string description = "unknown";
        if (m_Child[i].m_Object.GetGuid() != 0)
        {
            object* pObject = g_ObjMgr.GetObjectByGuid( m_Child[i].m_Object.GetGuid() );
            if (pObject)
            {
#ifdef X_EDITOR
                if (pObject->IsKindOf( trigger_ex_object::GetRTTI()))
                {
                    //special naming for triggers
                    trigger_ex_object* pTrigger = (trigger_ex_object*)pObject;
                    description.Set(pTrigger->GetTriggerName());
                    if (description.IsEmpty())
                    {
                        description.Set(pObject->GetTypeDesc().GetTypeName());
                    }
                }
                else
#endif // X_EDITOR
                {
#ifdef X_EDITOR
                    description.Set(pObject->GetName());
                    if (description.IsEmpty())
                    {
                        description.Set(pObject->GetTypeDesc().GetTypeName());
                    }
#else
                    description.Set(pObject->GetTypeDesc().GetTypeName());
#endif
                }
            }
        }
        
        List.PropEnumString( xfs("Group\\Child [%d] - %s",i,pTypeName),description.Get(), PROP_TYPE_HEADER );
        s32 iHeader = List.PushPath(xfs("Group\\Child [%d] - %s\\",i,pTypeName));

        m_Child[i].m_Object.OnEnumProp( List, "Object" );

        List.PropEnumButton( "Remove", "Remove child from group", PROP_TYPE_MUST_ENUM );

        List.PopPath(iHeader);          
    }  
}

//=============================================================================

xbool group::OnProperty( prop_query&   I    )
{
    // Call base class
    if( object::OnProperty( I ) )
    {
        return TRUE;
    }
    else if (I.IsVar("Group\\Child Count"))
    {
        if (I.IsRead())
        {
            I.SetVarInt( GetNumChildren() );
        }
        return TRUE;
    }
    else if (I.IsVar("Group\\Has Living Actors"))
    {
        if (I.IsRead())
        {           
            I.SetVarBool( HasLivingActors() );            
        }        
        return TRUE;
    }
    else if (I.IsVar("Group\\Is Empty"))
    {
        if (I.IsRead())
        {
            if (GetNumChildren() == 0)
                I.SetVarBool( TRUE );
            else 
                I.SetVarBool( FALSE );
        }        
        return TRUE;
    }
    else if (I.IsVar("Group\\Destroy All"))
    {
        if (I.IsRead())
        {
            I.SetVarBool( FALSE );
        }
        else
        {
            DestroyAll();
        }
        return TRUE;
    }
    else if (I.IsVar("Group\\Insert Child"))
    {
        if (I.IsRead())
        {
            I.SetVarGUID( NULL_GUID );
        }
        else
        {
            AddGuid( I.GetVarGUID() );
        }
        return TRUE;
    }
    else if (I.IsVar("Group\\Delete Child"))
    {
        if (I.IsRead())
        {
            I.SetVarGUID( NULL_GUID );
        }
        else
        {
            RemoveGuid( I.GetVarGUID() );
        }
        return TRUE;
    }
    else
        if( I.IsVar( "Group\\Refresh" ) )
        {
            if( I.IsRead() )
            {
                I.SetVarButton( "Refresh" );
            }
            else
            {
                ValidateChildren();
            }
            return TRUE;
        }
    else
    if( I.IsVar( "Group\\Add Child" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarButton( "Add Child" );
        }
        else
        {
            if (m_Child.GetCount() < GROUP_MAX_CHILDREN)
            {                
                child& C = m_Child.Append();
                C.m_Type = object::TYPE_NULL;
                C.m_Object.SetStaticGuid(NULL_GUID);
            }
        }
        return TRUE;
    }
    else 
    if (I.IsSimilarPath( "Group\\Child" ) )
    {
        s32 i = I.GetIndex(0);
        if (i>=0)
        {
            if (i >= m_Child.GetCount())
            {
                // We are accessing beyond what we have stored
                m_Child.SetCount( i+1 );
            }

            const char* pTypeName = GetObjectName( m_Child[i].m_Object.GetGuid() );
            if ( I.IsVar( xfs("Group\\Child [] - %s",pTypeName)) )
            {
                if( I.IsRead() )
                {
                    big_string description = "unknown";

                    if (m_Child[i].m_Object.GetGuid() != 0)
                    {
                        object* pObject = g_ObjMgr.GetObjectByGuid( m_Child[i].m_Object.GetGuid() );
                        if (pObject)
                        {
#ifdef X_EDITOR
                            if (pObject->IsKindOf( trigger_ex_object::GetRTTI()))
                            {
                                //special naming for triggers
                                trigger_ex_object* pTrigger = (trigger_ex_object*)pObject;
                                description.Set(pTrigger->GetTriggerName());
                                if (description.IsEmpty())
                                {
                                    description.Set(pObject->GetTypeDesc().GetTypeName());
                                }
                            }
                            else
#endif // X_EDITOR
                            {
#ifdef X_EDITOR
                                description.Set(pObject->GetName());
                                if (description.IsEmpty())
                                {
                                    description.Set(pObject->GetTypeDesc().GetTypeName());
                                }
#else
                                description.Set(pObject->GetTypeDesc().GetTypeName());
#endif
                            }
                        }
                    }
                    I.SetVarString(description.Get(), 255 );
                }
                return TRUE;
            }

            // Can these xfs calls be removed by using push/pop path inside the onproperty function?
            else 
            if (I.IsSimilarPath(xfs("Group\\Child [] - %s\\Object",pTypeName)))
            {
                s32 iPath = I.PushPath(xfs("Group\\Child [] - %s\\",pTypeName));
                if (m_Child[i].m_Object.OnProperty( I, "Object" ))
                {
                    if (!I.IsRead())
                    {
                        // Update object type for all static guids
                        if (m_Child[i].m_Object.GetObjectCode() == object_affecter::OBJECT_CODE_CHECK_BY_STATIC_GUID)
                        {
                            m_Child[i].m_Type = object::TYPE_NULL;
                            guid gObj = m_Child[i].m_Object.GetGuid();
                            if (NULL_GUID != gObj)
                            {
                                object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
                                if (NULL != pObj)
                                {                                
                                    m_Child[i].m_Type = pObj->GetType();
                                }
                            }
                        }
                    }

                    I.PopPath( iPath );
                    return TRUE;
                }
                I.PopPath( iPath );
            }
            else 
            if ( I.IsVar( xfs("Group\\Child [] - %s\\Remove",pTypeName) ) )
            {
                if( I.IsRead() )
                {
                    I.SetVarButton( "Remove" );
                }
                else
                {
                    m_Child.Delete( i );
                }
                return TRUE;
            }
        }
    }
    

    return FALSE;    
}

//=============================================================================
#ifndef X_RETAIL

void group::OnDebugRender( void )
{
    object::OnDebugRender();

    s32 i;

    vector3 MyPos = GetL2W().GetTranslation();

    for (i=0;i<m_Child.GetCount();i++)
    {
        guid gObj = m_Child[i].m_Object.GetGuid();

        if (NULL_GUID == gObj)
            continue;

        object* pObj = g_ObjMgr.GetObjectByGuid( gObj );

        if (NULL == pObj)
            continue;

        bbox Box = pObj->GetBBox();
        vector3 Pos = pObj->GetL2W().GetTranslation();

        draw_Line( MyPos, Pos, XCOLOR_YELLOW );
        draw_BBox( Box, XCOLOR_YELLOW );
        draw_Label( Pos, XCOLOR_RED, "[ %d ] - %s",i,g_ObjMgr.GetNameFromType( pObj->GetType() ) );
    }
}
#endif // X_RETAIL

//=============================================================================

void group::OnInit( void )
{
    object::OnInit();
}

//=============================================================================

const char* group::GetObjectName( guid gObj )
{
    (void)gObj;
    // FOR NOW, I'm removing the name display.  It screws up the property load/save
    // There is also potential for data disconnection if they change the name of an object
    // and then load a lyer with a group in it.   This all came up when I realized
    // that the name of the object has to be known at load time in order for the 
    // onProperty call to operate correctly.
    return NULL;

/*
    const char* pTypeName = "unknown";    
    if (NULL != gObj)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
        if (pObj)
        {
#ifdef USE_OBJECT_NAMES
            pTypeName = pObj->GetName();
            if (x_strlen(pTypeName) == 0)
            {
                pTypeName = pObj->GetLogicalName();
            }
#else
            pTypeName = pObj->GetLogicalName();
#endif
        }
    }
    return pTypeName;
*/
}

//=============================================================================

void group::ValidateChildren( void )
{
    s32     iCur = 0;
    xbool   bAdvance;

    while (iCur < m_Child.GetCount())
    {
        bAdvance = TRUE;

        guid gObj = m_Child[iCur].m_Object.GetGuid();
        if (NULL_GUID != gObj)
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
            if (NULL == pObj)
            {
                m_Child.Delete( iCur );
                bAdvance = FALSE;
            }       
            else
            {
                xbool bIsGlobal = !!(m_Child[iCur].m_Object.GetObjectCode() == object_affecter::OBJECT_CODE_CHECK_BY_GLOBAL_GUID);

                if (bIsGlobal || (m_Child[iCur].m_Type == object::TYPE_NULL))
                {
                    // Grab the type
                    m_Child[iCur].m_Type = pObj->GetType();
                }
            }
        }

        if (bAdvance)
            iCur++;
    }
}

//=============================================================================

xbool group::AddGuid( guid ChildGuid )
{
    ValidateChildren();
    
    s32 i;
    xbool bAdd = TRUE;
    for (i=0;i<m_Child.GetCount();i++)
    {   
        if (m_Child[i].m_Object.GetGuid() == ChildGuid)
        {
            bAdd = FALSE;
            break;
        }
    }
    if (bAdd)
    {
        if (m_Child.GetCount() == GROUP_MAX_CHILDREN)
            return FALSE;

        object* pObj = g_ObjMgr.GetObjectByGuid( ChildGuid );
        if (NULL == pObj)
        {
            // no object
            return FALSE;
        }            

        child& C = m_Child.Append();
        C.m_Object.SetStaticGuid( ChildGuid );
        C.m_Type = pObj->GetType();

        pObj->OnAddedToGroup( GetGuid() );
    }       
    return TRUE;
}

//=============================================================================

xbool group::RemoveGuid( guid ChildGuid )
{
    ValidateChildren();
    s32 i;
    for (i=0;i<m_Child.GetCount();i++)
    {   
        if (m_Child[i].m_Object.GetGuid() == ChildGuid)
        {
            m_Child.Delete(i);
            return TRUE;
        }
    }
    return FALSE;
}

//=============================================================================

xbool group::ContainsGuid( guid ChildGuid )
{
    ValidateChildren();
    s32 i;
    for (i=0;i<m_Child.GetCount();i++)
    {   
        if (m_Child[i].m_Object.GetGuid() == ChildGuid)
        {
            return TRUE;
        }
    }
    return FALSE;
}

//=============================================================================

s32 group::GetNumChildren( object::type Type )
{
    ValidateChildren();
    if (Type == object::TYPE_NULL)
    {
        return m_Child.GetCount();
    }

    s32 iCount = 0;
    s32 i;
    for (i=0;i<m_Child.GetCount();i++)
    {
        if (m_Child[i].m_Type == Type)
            iCount++;
    }

    return iCount;
}

//=============================================================================

guid group::GetChild( s32 iChild, object::type Type )
{
    ValidateChildren();

    if ((iChild < 0) || (iChild >m_Child.GetCount()))
        return NULL_GUID;

    if (Type == object::TYPE_NULL)
    {
        return m_Child[iChild].m_Object.GetGuid();
    }

    s32  iCount = 0;
    s32  i;
    guid gRet   = NULL_GUID;

    for (i=0;i<m_Child.GetCount();i++)
    {
        if (m_Child[i].m_Type == Type)
        {
            if (iCount == iChild)
            {
                gRet = m_Child[i].m_Object.GetGuid();
                break;
            }
            iCount++;
        }       
    }

    return gRet;
}

//=============================================================================

guid group::GetRandomChild( object::type Type )
{
    ValidateChildren();

    s32 Max = GetNumChildren( Type );
    s32 Idx = x_irand(0,Max-1);

    return GetChild( Idx, Type );
}

//=============================================================================

void group::OnActivate( xbool Flag )
{
    s32 i;
    for (i=0;i<m_Child.GetCount();i++)
    {
        guid gObj = m_Child[i].m_Object.GetGuid();
        if (NULL_GUID == gObj)
            continue;

        object* pObj = g_ObjMgr.GetObjectByGuid(gObj);
        if (NULL == pObj)
            continue;

        pObj->OnActivate( Flag );
    }
}

//=============================================================================

void group::OnPain( const pain& Pain )
{
    s32 i;
    for (i=0;i<m_Child.GetCount();i++)
    {
        guid gObj = m_Child[i].m_Object.GetGuid();
        if (NULL_GUID == gObj)
            continue;

        object* pObj = g_ObjMgr.GetObjectByGuid(gObj);
        if (NULL == pObj)
            continue;

        (const_cast<pain&>(Pain)).SetDirectHitGuid( gObj );

        pObj->OnPain( Pain );
    }
}

//=============================================================================

void group::DestroyAll( void )
{
    s32 i;
    for (i=0;i<m_Child.GetCount();i++)
    {
        guid gObj = m_Child[i].m_Object.GetGuid();
        if (NULL_GUID == gObj)
            continue;

        object* pObj = g_ObjMgr.GetObjectByGuid(gObj);
        if (NULL == pObj)
            continue;

        g_ObjMgr.DestroyObject( gObj );
    }
}

//=============================================================================

xbool group::HasLivingActors( void )
{
    s32 nChildren = GetNumChildren();
    s32 i;
    for (i=0;i<nChildren;i++)
    {
        guid gObj = m_Child[i].m_Object.GetGuid();
        if (NULL_GUID == gObj)
            continue;

        object* pObj = g_ObjMgr.GetObjectByGuid(gObj);
        if (NULL == pObj)
            continue;

        if (!(pObj->IsKindOf( actor::GetRTTI() )))
            continue;
        
        actor& Actor = actor::GetSafeType( *pObj );

        if (Actor.IsAlive())
            return TRUE;
    }
    return FALSE;
}

//=============================================================================

//=============================================================================
