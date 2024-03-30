//==============================================================================
//
//  condition_line_of_sight.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  define condition for checking line of sight between 2 objects
//
//==============================================================================

//==========================================================================
// INCLUDE
//==========================================================================

#include "condition_line_of_sight.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"
#include "collisionmgr\collisionmgr.hpp"
#include "objects\player.hpp"
#include "characters\character.hpp"

//==========================================================================
// DEFINES
//==========================================================================

//==========================================================================
// GLOBAL
//==========================================================================

//==========================================================================
// FUNTIONS
//==========================================================================

condition_line_of_sight::condition_line_of_sight( conditional_affecter* pParent ):  conditional_ex_base(  pParent ),
m_CheckCode(CODE_CAN_SEE_OBJECT)
{
    m_CanSee = FALSE;
}

//=============================================================================

xbool condition_line_of_sight::Execute( guid TriggerGuid )
{    
    (void) TriggerGuid;

    object* pObject1 = m_Affecter1.GetObjectPtr();
    object* pObject2 = m_Affecter2.GetObjectPtr();

    if (!pObject1 || !pObject2)
        return FALSE;

    g_CollisionMgr.LineOfSightSetup( NULL_GUID, pObject1->GetPosition(), pObject2->GetPosition() );
    g_CollisionMgr.AddToIgnoreList ((pObject1->GetGuid ()));
    g_CollisionMgr.SetMaxCollisions(1);

    if( pObject1->IsKindOf(player::GetRTTI()) )
    {    
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_PLAYER_LOS );
    }
    else if( pObject1->IsKindOf(character::GetRTTI()) )
    {
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_CHARACTER_LOS );
    }
    else
    {
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_PLAYER_LOS | object::ATTR_BLOCKS_CHARACTER_LOS );
    }

    switch (m_CheckCode)
    {
    case CODE_CAN_SEE_OBJECT:
        if( g_CollisionMgr.m_nCollisions == 0 ) 
        {
            m_CanSee = TRUE;
            return TRUE;
        }
        else
        {
            m_CanSee = FALSE;
        }
        break;
    case CODE_CANT_SEE_OBJECT:
        if( g_CollisionMgr.m_nCollisions ) 
        {
            m_CanSee = FALSE;
            return TRUE;
        }
        else
        {
            m_CanSee = TRUE;
        }
        break;
    default:
        ASSERT(0);
        break;
    }
    return FALSE; 
}    

//=============================================================================

void condition_line_of_sight::OnEnumProp ( prop_enum& rPropList )
{ 
    //guid specific fields
    m_Affecter1.OnEnumProp( rPropList, "Object" );

    rPropList.PropEnumInt  ( "CheckCode" ,        "",  PROP_TYPE_DONT_SHOW  );
    rPropList.PropEnumEnum ( "Operation",         "Can See\0Cant See\0", "Are we check if we can or cant see the object from another object.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );

    m_Affecter2.OnEnumProp( rPropList, "Target" );

    conditional_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool condition_line_of_sight::OnProperty ( prop_query& rPropQuery )
{
    //guid specific fields
    if( m_Affecter1.OnProperty( rPropQuery, "Object" ) )
        return TRUE;

    if( m_Affecter2.OnProperty( rPropQuery, "Target" ) )
        return TRUE;

    if( conditional_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if ( rPropQuery.VarInt ( "CheckCode"  , m_CheckCode ) )
        return TRUE;

    if ( rPropQuery.IsVar  ( "Operation" ) )
    {

        if( rPropQuery.IsRead() )
        {
            switch ( m_CheckCode )
            {
            case CODE_CAN_SEE_OBJECT:     rPropQuery.SetVarEnum( "Can See" );      break;
            case CODE_CANT_SEE_OBJECT:     rPropQuery.SetVarEnum( "Cant See" );  break;
            default:
                ASSERT(0);
                break;
            }

            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            if( x_stricmp( pString, "Can See" )==0)    { m_CheckCode = CODE_CAN_SEE_OBJECT;}
            if( x_stricmp( pString, "Cant See" )==0)   { m_CheckCode = CODE_CANT_SEE_OBJECT;}

            return( TRUE );
        }
    }

    return FALSE;
}

//=============================================================================

#ifndef X_RETAIL
void condition_line_of_sight::OnDebugRender ( s32 Index )
{
    object* pObject1 = m_Affecter1.GetObjectPtr();
    object* pObject2 = m_Affecter2.GetObjectPtr();
    if (pObject2)
    {
        sml_string Info;
        switch (m_CheckCode)
        {
        case CODE_CAN_SEE_OBJECT:
            Info.Set("Can See");
            break;
        case CODE_CANT_SEE_OBJECT:
            Info.Set("Cant See");
            break;
        default:
            ASSERT(0);
            break;
        }

        if (pObject1)
        {
            if( !m_CanSee )
                draw_Line( pObject1->GetPosition(), pObject2->GetPosition(), XCOLOR_RED );
            else
                draw_Line( pObject1->GetPosition(), pObject2->GetPosition(), XCOLOR_GREEN );
        }

        if (!GetElse())
        {
            draw_Label( pObject2->GetPosition(), XCOLOR_PURPLE, xfs("[If %d]%s", Index, Info.Get()) );
        }
        else
        {
            draw_Label( pObject2->GetPosition(), XCOLOR_PURPLE, xfs("[Else If %d]%s", Index, Info.Get()) );
        }
    }
}
#endif // X_RETAIL

//=============================================================================

const char* condition_line_of_sight::GetDescription( void )
{
    static big_string   Info;
    static med_string   TargetData;
    TargetData.Set(m_Affecter2.GetObjectInfo());

    switch ( m_CheckCode )
    {
    case CODE_CAN_SEE_OBJECT:     
        Info.Set(xfs("If %s CAN see %s", m_Affecter1.GetObjectInfo(), TargetData.Get()));          
        break;
    case CODE_CANT_SEE_OBJECT: 
        Info.Set(xfs("If %s CAN NOT see %s", m_Affecter1.GetObjectInfo(), TargetData.Get())); 
        break;
    default:
        ASSERT(0);
        break;
    }

    return Info.Get();
}


