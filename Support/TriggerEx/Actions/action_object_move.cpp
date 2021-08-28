///////////////////////////////////////////////////////////////////////////
//
//  action_object_move.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_object_move.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "Characters\Character.hpp"
#include "Objects\Player.hpp"

static const xcolor s_MoveColor             (255,0,255);

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_object_move::action_object_move ( guid ParentGuid ) : actions_ex_base( ParentGuid ),
m_bAlignToMarker(TRUE)
{
}

//=============================================================================

xbool action_object_move::Execute ( f32 DeltaTime )
{
    (void) DeltaTime;

    if ( m_Marker != 0 )
    {
        object *pMarker = g_ObjMgr.GetObjectByGuid(m_Marker);
        object* pObject = m_ObjectAffecter.GetObjectPtr();
        if (pMarker && pObject)
        {
            matrix4 M;
            if (m_bAlignToMarker)
            {
                // Moving an NPC?
                if( pObject->IsKindOf( character::GetRTTI() ) )
                {
                    // To align with the marker, we need to adjust the Yaw by 180 -> See actor::OnTransform()
                    radian3 AdjustedRotation = pMarker->GetL2W().GetRotation();
                    AdjustedRotation.Yaw += R_180;
                    M.Setup( vector3(1.0f,1.0f,1.0f), AdjustedRotation, pMarker->GetPosition() );
                }
                else
                {
                    M.Setup( vector3(1.0f,1.0f,1.0f), pMarker->GetL2W().GetRotation(), pMarker->GetPosition() );
                }
            }
            else
            {
                M.Setup( vector3(1.0f,1.0f,1.0f), pObject->GetL2W().GetRotation(), pMarker->GetPosition() );
            }

            // Update polycache around the object
            g_PolyCache.InvalidateCells( pObject->GetBBox(), pObject->GetGuid() );

            //some objects require zones set first, some after, doing both to be safe
            pObject->SetZone1( pMarker->GetZone1() );
            pObject->SetZone2( pMarker->GetZone2() );   
            pObject->OnTriggerTransform( M );
            pObject->SetZone1( pMarker->GetZone1() );
            pObject->SetZone2( pMarker->GetZone2() );

            if( pObject->IsKindOf( player::GetRTTI() ) )
            {
                ((player*)pObject)->Teleport( pObject->GetPosition(), FALSE, FALSE );
            }
            else if( pObject->IsKindOf( actor::GetRTTI() ) )
            {
                actor* pActor = (actor*)pObject;
                pActor->InitZoneTracking();
            }

            // Update polycache around the object
            g_PolyCache.InvalidateCells( pObject->GetBBox(), pObject->GetGuid() );

            return TRUE;
        }
    }

    m_bErrorInExecute = TRUE;
    return (!RetryOnError());
}

//=============================================================================

#ifndef X_RETAIL
void action_object_move::OnDebugRender ( s32 Index )
{
    object* pObject = m_ObjectAffecter.GetObjectPtr();
    if (pObject)
    {
        draw_Line( GetPositionOwner(), pObject->GetPosition(), s_MoveColor );
        draw_BBox( bbox(pObject->GetPosition(), 100.0f), s_MoveColor );
        if (!GetElse())
        {
            draw_Label( pObject->GetPosition(), s_MoveColor, xfs("[%d]Move Object", Index) );
        }
        else
        {
            draw_Label( pObject->GetPosition(), s_MoveColor, xfs("[Else %d]Move Object", Index) );
        }

        if (m_Marker != 0)
        {
            object *pMarker = g_ObjMgr.GetObjectByGuid(m_Marker);
            if (pMarker)
            {
                draw_Line( pObject->GetPosition(), pMarker->GetPosition(), XCOLOR_WHITE );
                draw_BBox( bbox(pMarker->GetPosition(), 100.0f), XCOLOR_WHITE );
                if (!GetElse())
                {
                    draw_Label( pObject->GetPosition(), s_MoveColor, xfs("[%d]Move Here", Index) );
                }
                else
                {
                    draw_Label( pObject->GetPosition(), s_MoveColor, xfs("[Else %d]Move Here", Index) );
                }
            }
        }
    }
}
#endif // X_RETAIL

//=============================================================================

void action_object_move::OnEnumProp	( prop_enum& rPropList )
{
    //guid specific fields
    m_ObjectAffecter.OnEnumProp( rPropList, "Object" );

    rPropList.PropEnumGuid(  "Marker" ,       "Use a marker to move this object; gets position, rotation, and zone.", 0 );
    rPropList.PropEnumBool(  "AlignToMarker" ,"If true the markers rotation is used to rotate the moved object.", 0 );

    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_object_move::OnProperty	( prop_query& rPropQuery )
{
    //guid specific fields
    if( m_ObjectAffecter.OnProperty( rPropQuery, "Object" ) )
        return TRUE;

    if ( rPropQuery.VarGUID ( "Marker", m_Marker ) )
        return TRUE;

    if ( rPropQuery.VarBool ( "AlignToMarker", m_bAlignToMarker ) )
        return TRUE;

    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    return FALSE;
}

//=============================================================================

const char* action_object_move::GetDescription( void )
{
    static big_string   Info;
    if ( m_Marker == 0 )
    {
        Info.Set(xfs("Move %s Where?", m_ObjectAffecter.GetObjectInfo()));          
    }
    else
    {
        Info.Set(xfs("Move %s", m_ObjectAffecter.GetObjectInfo()));          
    }
    return Info.Get();
}


