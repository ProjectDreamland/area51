///////////////////////////////////////////////////////////////////////////
//
//  action_object_move_relative_relative.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_object_move_relative.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "Objects\Actor\Actor.hpp"
#include "Objects\Player.hpp"
#include "Loco\Loco.hpp"

static const xcolor s_MoveColor             (255,0,255);

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_object_move_relative::action_object_move_relative ( guid ParentGuid ) : actions_ex_base( ParentGuid )
{
}

//=============================================================================

xbool action_object_move_relative::Execute ( f32 DeltaTime )
{
    (void) DeltaTime;

    if (( m_DestMarker != 0 ) && ( m_RelativeMarker != 0 ))
    {
        object *pDestMarker = g_ObjMgr.GetObjectByGuid(m_DestMarker);
        object *pRelMarker  = g_ObjMgr.GetObjectByGuid(m_RelativeMarker);
        object* pObject     = m_ObjectAffecter.GetObjectPtr();
        if (pDestMarker && pRelMarker && pObject)
        {
            radian3 ObjRot(0,0,0);

            if (pObject->IsKindOf( player::GetRTTI() ))
            {
                player& Player = player::GetSafeType( *pObject );
                
                ObjRot.Set(0,Player.GetYaw(),0);                
            }
            else
            if (pObject->IsKindOf( actor::GetRTTI() ))
            {
                actor& Actor = actor::GetSafeType( *pObject );
                loco* pLoco = Actor.GetLocoPointer();

                ObjRot.Set(0,pLoco->GetYaw(),0);                
            }
            else
            {
                ObjRot = pObject->GetL2W().GetRotation();
            }

            vector3 Pos = pObject->GetPosition() - pRelMarker->GetPosition();
            radian3 Rot = ObjRot - pRelMarker->GetL2W().GetRotation();


            matrix4 M;

            M.Setup(    vector3(1.0f,1.0f,1.0f), 
                        pDestMarker->GetL2W().GetRotation() + Rot,
                        pDestMarker->GetPosition() + Pos );

            // Update polycache around the object
            g_PolyCache.InvalidateCells( pObject->GetBBox(), pObject->GetGuid() );

            //some objects require zones set first, some after, doing both to be safe
            pObject->SetZone1( pDestMarker->GetZone1() );
            pObject->SetZone2( pDestMarker->GetZone2() );   
            pObject->OnTriggerTransform( M );
            pObject->SetZone1( pDestMarker->GetZone1() );
            pObject->SetZone2( pDestMarker->GetZone2() );

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
void action_object_move_relative::OnDebugRender ( s32 Index )
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

        if ((m_DestMarker != 0) && (m_RelativeMarker != 0))
        {
            object *pDestMarker = g_ObjMgr.GetObjectByGuid(m_DestMarker);
            object *pRelMarker  = g_ObjMgr.GetObjectByGuid(m_RelativeMarker);
            if (pDestMarker)
            {
                draw_Line( pObject->GetPosition(), pDestMarker->GetPosition(), XCOLOR_WHITE );
                draw_BBox( bbox(pDestMarker->GetPosition(), 100.0f), XCOLOR_WHITE );
                if (!GetElse())
                {
                    draw_Label( pObject->GetPosition(), s_MoveColor, xfs("[%d]Move Here", Index) );
                }
                else
                {
                    draw_Label( pObject->GetPosition(), s_MoveColor, xfs("[Else %d]Move Here", Index) );
                }
            }

            if (pRelMarker)
            {
                draw_Line( pObject->GetPosition(), pRelMarker->GetPosition(), XCOLOR_WHITE );
                draw_BBox( bbox(pRelMarker->GetPosition(), 100.0f), XCOLOR_WHITE );
                if (!GetElse())
                {
                    draw_Label( pObject->GetPosition(), s_MoveColor, xfs("[%d]Relative To Here", Index) );
                }
                else
                {
                    draw_Label( pObject->GetPosition(), s_MoveColor, xfs("[Else %d]Relative To Here", Index) );
                }
            }
        }
    }
}
#endif // X_RETAIL

//=============================================================================

void action_object_move_relative::OnEnumProp	( prop_enum& rPropList )
{
    //guid specific fields
    m_ObjectAffecter.OnEnumProp( rPropList, "Object" );

    rPropList.PropEnumGuid(  "Destination Marker" ,  "Move object to this marker", 0 );
    rPropList.PropEnumGuid(  "Relative To Marker" ,  "Object's position and rotation relative to this marker will be added to the destination location.", 0 );

    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_object_move_relative::OnProperty	( prop_query& rPropQuery )
{
    //guid specific fields
    if( m_ObjectAffecter.OnProperty( rPropQuery, "Object" ) )
        return TRUE;

    if ( rPropQuery.VarGUID ( "Destination Marker", m_DestMarker ) )
        return TRUE;

    if ( rPropQuery.VarGUID ( "Relative To Marker", m_RelativeMarker ) )
        return TRUE;

    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    return FALSE;
}

//=============================================================================

const char* action_object_move_relative::GetDescription( void )
{
    static big_string   Info;
    if ( m_DestMarker == 0 )
    {
        Info.Set(xfs("Move %s Where?", m_ObjectAffecter.GetObjectInfo()));          
    }
    else if (m_RelativeMarker == 0)
    {
        Info.Set(xfs("Move %s Relative To What?", m_ObjectAffecter.GetObjectInfo()));          
    }
    else
    {
        Info.Set(xfs("Move %s Relatively", m_ObjectAffecter.GetObjectInfo()));          
    }
    return Info.Get();
}


