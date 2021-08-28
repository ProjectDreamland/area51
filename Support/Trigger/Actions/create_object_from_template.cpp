///////////////////////////////////////////////////////////////////////////
//
//  create_object_from_template.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\create_object_from_template.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "..\Support\Templatemgr\TemplateMgr.hpp"

#include "Dictionary\global_dictionary.hpp"

#include "Entropy.hpp"

static const xcolor s_CreateTemplateColor   (255,255,255);

//=========================================================================
// CREATE_OBJECT_FROM_TEMPLATE
//=========================================================================

create_object_from_template::create_object_from_template ( guid ParentGuid ) : actions_base( ParentGuid ),
m_Position(0.0f,0.0f,0.0f),
m_Orientation(0.0f,0.0f,0.0f),
m_RandomVectorExtent(0.0f,0.0f,0.0f)
{

}

//=============================================================================

void create_object_from_template::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * create_object_from_template::Execute" );
        
    (void) pParent;

    if ( m_TemplateID.IsEmpty() )
        return;

    vector3 FinalPos = m_Position;

    if ( m_RandomVectorExtent.Length() > 0.0f )
    {
        FinalPos.X += x_frand(-m_RandomVectorExtent.X,  m_RandomVectorExtent.X);
        FinalPos.Y += x_frand(-m_RandomVectorExtent.Y,  m_RandomVectorExtent.Y);
        FinalPos.Z += x_frand(-m_RandomVectorExtent.Z,  m_RandomVectorExtent.Z);
    }
    
    g_TemplateMgr.CreateTemplate( m_TemplateID.Get(), FinalPos, m_Orientation, pParent->GetZone1(), pParent->GetZone2() );
#ifdef TARGET_PC
    x_DebugMsg(xfs("Trigger %s creating template %s\n",guid_ToString(pParent->GetGuid()), m_TemplateID.Get()));
#endif
}

//=============================================================================

void  create_object_from_template::OnRender ( void )
{
#ifdef TARGET_PC
    vector3 MyPosition =  GetPositionOwner() + SMP_UTIL_RandomVector(k_rand_draw_displace_amt);

    draw_Line( MyPosition, m_Position, s_CreateTemplateColor );
    draw_BBox( bbox(m_Position, 100.0f), s_CreateTemplateColor );
    draw_Label( m_Position, s_CreateTemplateColor, GetTypeName() );
#endif
}

//=============================================================================

void create_object_from_template::OnEnumProp ( prop_enum& rPropList )
{
    //object info
     
//    rPropList.AddExternal( "Template" ,  "File\0blueprint", "Template Name, to determine what type of object to create." );
    rPropList.AddFileName( "TemplateBPX" ,  "template blueprints (*.bpx)|*.bpx|All Files (*.*)|*.*||", "Template Name, to determine what type of object to create." );
   
    rPropList.AddVector3(  "Position" , "Position of object." );
     
    rPropList.AddRotation( "Orientation" , "Rotation of the object." );
 
    rPropList.AddVector3(  "Random Vector Extent" , "Amount of random varation in terms of position. Leave at 0,0,0 for no variation." );
    
    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool create_object_from_template::OnProperty ( prop_query& rPropQuery )
{  
    if ( rPropQuery.VarVector3 ( "Position"  , m_Position ) )
        return TRUE;
     
    if ( rPropQuery.VarVector3 ( "Random Vector Extent"  , m_RandomVectorExtent ) )
        return TRUE;
    
    if ( rPropQuery.VarRotation ( "Orientation"  , m_Orientation ) )
        return TRUE;
    
//for editor build external handling
/*
    if( rPropQuery.IsVar( "Template" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_TemplateID.Get(),  m_TemplateID.MaxLen() );
        }
        else
        {
            if( rPropQuery.GetVarExternal()[0] )
            {
                //remove any reference to the old template
                g_TemplateMgr.EditorRemoveBlueprintRef(m_TemplateID.Get());

                m_TemplateID.Set( rPropQuery.GetVarExternal() );

               //add new blueprint
                g_TemplateMgr.EditorAddBlueprintRef(m_TemplateID.Get());

                //clean blueprint list : Due to order of operataion and syncrhronization issues with the
                //blueprint system and the property system, the blue prints should manage its states independent
                //of the property system, otehrwise its possible to delete blueprints still refrencned by the property
                //system..

                ///g_TemplateMgr.EditorReconcileBlueprints();
            }
        }
        return( TRUE );
    }
*/

    if ( rPropQuery.VarFileName ( "TemplateBPX"  , m_TemplateID.Get(),  m_TemplateID.MaxLen()) )
    {
        if( !rPropQuery.IsRead() && x_strlen( rPropQuery.GetVarFileName() ) > 0)
        {
            g_TemplateStringMgr.Add( rPropQuery.GetVarFileName() );
        }

        return TRUE;
    }
    
    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;
    
    return FALSE;
}

//===========================================================================

#ifdef WIN32
void create_object_from_template::EditorPreGame( void )
{
    matrix4 rMat;
    rMat.Identity();

    guid ObjectGuid = g_TemplateMgr.EditorCreateSingleTemplateFromPath( m_TemplateID.Get(), rMat.GetTranslation(), rMat.GetRotation(), -1, -1 ); 
    
    // Make sure that the item we just created was an 'inventory item'
    object* pObject = g_ObjMgr.GetObjectByGuid( ObjectGuid );
    if ( pObject )
    {
        g_TemplateStringMgr.Add(m_TemplateID.Get());
        pObject->EditorPreGame();
        g_ObjMgr.DestroyObject(ObjectGuid);
    }

}
#endif
