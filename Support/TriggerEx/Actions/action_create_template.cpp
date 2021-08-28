///////////////////////////////////////////////////////////////////////////
//
//  action_create_template.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_create_template.hpp"
#include "..\Support\Templatemgr\TemplateMgr.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "Objects\Group.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

static const xcolor s_CreateTemplateColor   (255,255,255);

action_create_template::action_create_template ( guid ParentGuid ) : actions_ex_base( ParentGuid ),
m_StorageIndex(-1),
m_TemplateIndex(-1)
{
    m_Group  = NULL_GUID;
    m_Marker = NULL_GUID;
}

//=============================================================================

xbool action_create_template::Execute ( f32 DeltaTime )
{
    (void) DeltaTime;

    if ( (m_TemplateIndex >= 0) && (m_Marker != 0) )
    {
        object *pMarker = g_ObjMgr.GetObjectByGuid(m_Marker);
        if (pMarker)
        {
            if (m_StorageIndex >= 0)
            {
                guid GuidID = g_TemplateMgr.CreateSingleTemplate( 
                    g_TemplateStringMgr.GetString(m_TemplateIndex), 
                    pMarker->GetPosition(), 
                    pMarker->GetL2W().GetRotation(), 
                    pMarker->GetZone1(), 
                    pMarker->GetZone2() );

                xhandle rHandle;
                if ( g_VarMgr.GetGuidHandle( g_StringMgr.GetString(m_StorageIndex), &rHandle ) )
                {
                    g_VarMgr.SetGuid(rHandle, GuidID);

                    if (AddGuidToGroup( GuidID ))
                    {
                        if (GuidID != 0)
                            return TRUE;
                    }
                }
            }
            else
            {
                const char* pName = g_TemplateStringMgr.GetString(m_TemplateIndex);

                if (g_TemplateMgr.IsSingleTemplate( pName ))
                {
                    guid GuidID = g_TemplateMgr.CreateSingleTemplate(   pName, 
                                                                        pMarker->GetPosition(), 
                                                                        pMarker->GetL2W().GetRotation(), 
                                                                        pMarker->GetZone1(), 
                                                                        pMarker->GetZone2() );

                    if (AddGuidToGroup( GuidID ))
                    {
                        return TRUE;
                    }
                }
                else
                if (g_TemplateMgr.CreateTemplate( g_TemplateStringMgr.GetString(m_TemplateIndex), 
                                                  pMarker->GetPosition(), 
                                                  pMarker->GetL2W().GetRotation(), 
                                                  pMarker->GetZone1(), 
                                                  pMarker->GetZone2() ))
                {                    
                    return TRUE;
                }
            }
        }
    }

    m_bErrorInExecute = TRUE;
    return (!RetryOnError());
}

//=============================================================================

#ifndef X_RETAIL
void action_create_template::OnDebugRender ( s32 Index )
{
    if (m_Marker != 0)
    {
        object *pMarker = g_ObjMgr.GetObjectByGuid(m_Marker);
        if (pMarker)
        {
            vector3 MarkerPosition =  pMarker->GetPosition();

            draw_Line( GetPositionOwner(), MarkerPosition, s_CreateTemplateColor );
            draw_BBox( bbox(MarkerPosition, 100.0f), s_CreateTemplateColor );
            if (!GetElse())
            {
                draw_Label( MarkerPosition, s_CreateTemplateColor, xfs("[%d]Create Template", Index) );
            }
            else
            {
                draw_Label( MarkerPosition, s_CreateTemplateColor, xfs("[Else %d]Create Template", Index) );
            }
        }
    }
}
#endif // X_RETAIL

//=============================================================================

void action_create_template::OnEnumProp	( prop_enum& rPropList )
{
    rPropList.PropEnumFileName(  "TemplateBPX" ,  "template blueprints (*.bpx)|*.bpx|All Files (*.*)|*.*||", "Template Name, to determine what type of object to create.", 0 );  
    rPropList.PropEnumGuid    (  "Marker" ,       "Use a marker to place this object, gets position, rotation, and zone.", 0 );
    rPropList.PropEnumGuid    (  "Group",         "Group to place the spawned object into", 0 );

    rPropList.PropEnumExternal( "Storage Var", "global\0global_guid", 
                                "(Optional) You can store the guid of the object created in this global variable.", PROP_TYPE_MUST_ENUM );
    
    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_create_template::OnProperty	( prop_query& rPropQuery )
{ 
    if ( rPropQuery.IsVar( "TemplateBPX" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if ( m_TemplateIndex < 0 )
            {
                rPropQuery.SetVarFileName( "", 256 );
            }
            else
            {
                rPropQuery.SetVarFileName( g_TemplateStringMgr.GetString( m_TemplateIndex ), 256 );
            }
        }
        else
        {
            if ( x_strlen( rPropQuery.GetVarFileName() ) > 0 )
            {
                m_TemplateIndex = g_TemplateStringMgr.Add( rPropQuery.GetVarFileName() );
            }
        }

        return TRUE;
    }

    if ( rPropQuery.IsVar( "Storage Var" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if ( m_StorageIndex < 0 )
            {
                rPropQuery.SetVarExternal( "", 256 );
            }
            else
            {
                rPropQuery.SetVarExternal( g_StringMgr.GetString( m_StorageIndex ), 256 );
            }
        }
        else
        {
            if ( x_strlen( rPropQuery.GetVarExternal() ) > 0 )
            {
                m_StorageIndex = g_StringMgr.Add( rPropQuery.GetVarExternal() );
            }
        }

        return TRUE;
    }

    if ( rPropQuery.VarGUID ( "Marker", m_Marker ) )
        return TRUE;

    if ( rPropQuery.VarGUID( "Group", m_Group ))
    {
        return TRUE;
    }
    
    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    return FALSE;
}

//=============================================================================

const char* action_create_template::GetDescription( void )
{
    static big_string   Info;
    static big_string   Data;
    
    if ( m_TemplateIndex < 0 )
    {
        Info.Set("Create Unspecified Template");
    }
    else if ( m_Marker == 0 )
    {
        Info.Set("Create Template Where?");
    }   
    else
    {
        xstring templateString = g_TemplateStringMgr.GetString( m_TemplateIndex );
        s32 nStart = 0;
        s32 nNext = 0;
        while (nNext != -1)
        {
            //find the last slash
            nStart = nNext;
            nNext = templateString.Find('\\', nStart+1);
        }
        Data.Set(templateString.Right(templateString.GetLength() - nStart - 1));

        if ( m_StorageIndex < 0 )   
        {
            Info.Set(xfs("Create \"%s\"", Data.Get()));
        }
        else
        {
            Info.Set(xfs("Create \"%s\" as [%s]", Data.Get(), g_StringMgr.GetString( m_StorageIndex )));
        }
    }

    return Info.Get();
}

//===========================================================================

#ifdef X_EDITOR
void action_create_template::EditorPreGame( void )
{
    matrix4 rMat;
    rMat.Identity();

    if (m_TemplateIndex == -1)
        return;
    
    guid ObjectGuid = g_TemplateMgr.EditorCreateSingleTemplateFromPath( g_TemplateStringMgr.GetString( m_TemplateIndex ), rMat.GetTranslation(), rMat.GetRotation(), -1, -1 ); 
    
    object* pObject = g_ObjMgr.GetObjectByGuid( ObjectGuid );
    if ( pObject )
    {
        pObject->EditorPreGame();
        g_ObjMgr.DestroyObjectEx(ObjectGuid,TRUE);
    }
}
#endif // X_EDITOR

//===========================================================================

xbool action_create_template::AddGuidToGroup( const guid& Guid )
{
    if (NULL_GUID == m_Group)
        return TRUE;

    object* pObj = g_ObjMgr.GetObjectByGuid( m_Group );
    if (NULL == pObj)
        return FALSE;

    if (!pObj->IsKindOf( group::GetRTTI() ))
        return FALSE;

    group& Group = group::GetSafeType( *pObj );

    Group.AddGuid( Guid );

    return TRUE;
}

//===========================================================================
