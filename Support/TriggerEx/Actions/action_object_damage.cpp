///////////////////////////////////////////////////////////////////////////////
//
//  action_object_damage.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_object_damage.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "objects\actor\actor.hpp"
#include "painmgr\painmgr.hpp"

static const xcolor s_DamageColor          (255,64,64);

//=========================================================================
// ENUMS
//=========================================================================


typedef enum_pair<action_object_damage::hit_location_type> hit_location_type_enum_pair;
static hit_location_type_enum_pair s_HitLocationTypeList[] = 
{
    hit_location_type_enum_pair("Head_Hit",                 action_object_damage::HIT_HEAD),
        hit_location_type_enum_pair("Body_Hit",             action_object_damage::HIT_CENTER),
        hit_location_type_enum_pair("Feet_Hit",             action_object_damage::HIT_FEET),
        hit_location_type_enum_pair( k_EnumEndStringConst,  action_object_damage::HIT_HEAD)
};
enum_table<action_object_damage::hit_location_type>  g_HitLocationTypeList( s_HitLocationTypeList );              

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_object_damage::action_object_damage( guid ParentGuid ):  actions_ex_base(  ParentGuid )
{
    m_GenericPainType = TYPE_GENERIC_1;
    m_HitLocationType = HIT_HEAD;
}

//=============================================================================

xbool action_object_damage::Execute( f32 DeltaTime )
{    
    (void) DeltaTime;

    object* pObject = m_ObjectAffecter.GetObjectPtr();
    if (pObject)
    {
        object *triggerObject = g_ObjMgr.GetObjectByGuid( m_TriggerGuid );
        pain_handle PainHandle = GetPainHandleForGenericPain( m_GenericPainType );

        //Do Damage
        pain Pain;

        // special case for actors on the position
        if( pObject->IsKindOf(actor::GetRTTI()) )
        {
            actor& rActor = actor::GetSafeType(*pObject);
            switch( m_HitLocationType )
            {
            case HIT_HEAD:
                Pain.Setup( PainHandle, m_TriggerGuid, rActor.GetPositionWithOffset(actor::OFFSET_EYES) );
                break;
            case HIT_CENTER:
                Pain.Setup( PainHandle, m_TriggerGuid, rActor.GetPositionWithOffset(actor::OFFSET_CENTER) );
                break;
            case HIT_FEET:
                Pain.Setup( PainHandle, m_TriggerGuid, rActor.GetPosition() );
                break;
            }
        }
        else
        {        
            Pain.Setup( PainHandle, m_TriggerGuid, pObject->GetPosition() );
        }
        if( triggerObject )
            Pain.SetDirection( pObject->GetPosition() - triggerObject->GetPosition() );
        Pain.SetDirectHitGuid( pObject->GetGuid() );
        Pain.ApplyToObject( pObject );

    }

    return TRUE;
}    

//=============================================================================

#ifndef X_RETAIL
void action_object_damage::OnDebugRender ( s32 Index )
{
    object* pObject = m_ObjectAffecter.GetObjectPtr();
    if (pObject)
    {
        draw_Line( GetPositionOwner(), pObject->GetPosition(), s_DamageColor );
        draw_BBox( pObject->GetBBox(), s_DamageColor );

        if (!GetElse())
        {
            draw_Label( pObject->GetPosition(), s_DamageColor, xfs("[%d]Damage Object", Index) );
        }
        else
        {
            draw_Label( pObject->GetPosition(), s_DamageColor, xfs("[Else %d]Damage Object", Index) );
        }
    }
}
#endif // X_RETAIL

//=============================================================================

void action_object_damage::OnEnumProp ( prop_enum& rPropList )
{ 
    //guid specific fields
    m_ObjectAffecter.OnEnumProp( rPropList, "Object" );

    rPropList.PropEnumEnum    ( "PainType",    g_GenericPainTypeList.BuildString(), "What type of pain this action causes.", PROP_TYPE_MUST_ENUM );
    rPropList.PropEnumEnum    ( "HitLocation",    g_HitLocationTypeList.BuildString(), "Where the pain is applied to the actor.", PROP_TYPE_MUST_ENUM );
    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_object_damage::OnProperty ( prop_query& rPropQuery )
{
    //guid specific fields
    if( m_ObjectAffecter.OnProperty( rPropQuery, "Object" ) )
        return TRUE;

    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if ( rPropQuery.IsVar( "PainType") )
    {
        if( rPropQuery.IsRead() )
        {
            if ( g_GenericPainTypeList.DoesValueExist( m_GenericPainType ) )
            {
                rPropQuery.SetVarEnum( g_GenericPainTypeList.GetString( m_GenericPainType ) );
            }
            else
            {
                rPropQuery.SetVarEnum( "Generic_1" );
            } 
        }
        else
        {
            generic_pain_type PainType;

            if( g_GenericPainTypeList.GetValue( rPropQuery.GetVarEnum(), PainType ) )
            {
                m_GenericPainType = PainType;
            }
        }
        
        return( TRUE );
    }
    else if ( rPropQuery.IsVar( "HitLocation") )
    {
        if( rPropQuery.IsRead() )
        {
            if ( g_HitLocationTypeList.DoesValueExist( m_HitLocationType ) )
            {
                rPropQuery.SetVarEnum( g_HitLocationTypeList.GetString( m_HitLocationType ) );
            }
            else
            {
                rPropQuery.SetVarEnum( "HEAD_HIT" );
            } 
        }
        else
        {
            hit_location_type HitType;

            if( g_HitLocationTypeList.GetValue( rPropQuery.GetVarEnum(), HitType ) )
            {
                m_HitLocationType = HitType;
            }
        }
        return( TRUE );
    }

    return FALSE;
}

//=============================================================================

const char* action_object_damage::GetDescription( void )
{
    static big_string   Info;
    if ( g_GenericPainTypeList.DoesValueExist( m_GenericPainType ) )
    {
        Info.Set(xfs("Damage %s with %s pain", m_ObjectAffecter.GetObjectInfo(), g_GenericPainTypeList.GetString( m_GenericPainType ) ) );
    }
    else
    {
        Info.Set(xfs("Damage %s", m_ObjectAffecter.GetObjectInfo()));          
    }
    return Info.Get();
}
