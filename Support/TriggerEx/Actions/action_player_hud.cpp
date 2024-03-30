///////////////////////////////////////////////////////////////////////////////
//
//  action_player_hud.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_player_hud.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "..\Support\Zonemgr\ZoneMgr.hpp"
#include "..\Support\Objects\HudObject.hpp"


//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_player_hud::action_player_hud( guid ParentGuid ):  actions_ex_base(  ParentGuid )
{
    m_ElementID = HUD_ELEMENT_HEALTH_BAR;
    m_DoPulse   = FALSE;
}

//=============================================================================

xbool action_player_hud::Execute( f32 DeltaTime )
{    
    (void) DeltaTime;

    // find the HUD object slot
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );
    
    if( SlotID != SLOT_NULL )
    {
        // Get the HUD object
        object* pHudObject = g_ObjMgr.GetObjectBySlot( SlotID );

        // set the HUD element pulse state
        ((hud_object*)pHudObject)->SetElementPulseState( m_ElementID, m_DoPulse );
    }

    return TRUE;
}    
    
//=============================================================================

#ifndef X_RETAIL
void action_player_hud::OnDebugRender ( s32 Index )
{
    (void)Index;
}
#endif // X_RETAIL

//=============================================================================

void action_player_hud::OnEnumProp ( prop_enum& rPropList )
{ 
    //guid specific fields
    rPropList.PropEnumEnum( "Element ID" ,        "Health Bar\0Ammo Bar\0Text Box\0Reticle\0Pain Sensor\0", "Select HUD element", 0  );
    rPropList.PropEnumBool( "Pulse On",           "Pulse this element", 0 );

    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_player_hud::OnProperty ( prop_query& rPropQuery )
{
    //guid specific fields
    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    if( rPropQuery.IsVar( "Element ID" ) )
    {
        if( rPropQuery.IsRead () )
        {
            switch( m_ElementID )
            {
                case HUD_ELEMENT_HEALTH_BAR         : rPropQuery.SetVarEnum( "Health Bar"   ); break;
                case HUD_ELEMENT_AMMO_BAR           : rPropQuery.SetVarEnum( "Ammo Bar"     ); break;
                case HUD_ELEMENT_TEXT_BOX           : rPropQuery.SetVarEnum( "Text Box"     ); break;
                case HUD_ELEMENT_RETICLE            : rPropQuery.SetVarEnum( "Reticle"      ); break;
                case HUD_ELEMENT_DAMAGE             : rPropQuery.SetVarEnum( "Pain Sensor"  ); break;
                default:    
                {
                ASSERTS( FALSE, "Didn't set the element ID"  );
                }
            } 
        }
        else
        {
            if( !x_stricmp( "Health Bar", rPropQuery.GetVarEnum()) )
            {
                m_ElementID = HUD_ELEMENT_HEALTH_BAR;
            }
            else if( !x_stricmp( "Ammo Bar", rPropQuery.GetVarEnum() ) )
            {
                m_ElementID = HUD_ELEMENT_AMMO_BAR;
            }
            else if( !x_stricmp( "Text Box", rPropQuery.GetVarEnum() ) )
            {
                m_ElementID = HUD_ELEMENT_TEXT_BOX;
            }
            else if( !x_stricmp( "Reticle", rPropQuery.GetVarEnum() ) )
            {
                m_ElementID = HUD_ELEMENT_RETICLE;
            }
            else if( !x_stricmp( "Pain Sensor", rPropQuery.GetVarEnum() ) )
            {
                m_ElementID = HUD_ELEMENT_DAMAGE;
            }
        }

        return TRUE;
    }

    if ( rPropQuery.VarBool (  "Pulse On"  , m_DoPulse ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* action_player_hud::GetDescription( void )
{
    static big_string   Info;

    xstring OnOffString;

    if( m_DoPulse )
    {
        OnOffString = "ON";
    }
    else
    {
        OnOffString = "OFF";
    }

    switch( m_ElementID )
    {
        case HUD_ELEMENT_HEALTH_BAR: 
        {
            xstring ItemString("Set HUD health bar pulse ");
            ItemString += OnOffString;
            Info.Set(ItemString);          
        }
        break;

        case HUD_ELEMENT_AMMO_BAR: 
        {
            xstring ItemString("Set HUD ammo bar pulse ");
            ItemString += OnOffString;
            Info.Set(ItemString);          
        }
        break;

        case HUD_ELEMENT_TEXT_BOX: 
        {
            xstring ItemString("Set HUD text box pulse ");
            ItemString += OnOffString;
            Info.Set(ItemString);          
        }
        break;

        case HUD_ELEMENT_RETICLE: 
        {
            xstring ItemString("Set HUD reticle pulse ");
            ItemString += OnOffString;
            Info.Set(ItemString);          
        }
        break;

        case HUD_ELEMENT_DAMAGE: 
        {
            xstring ItemString("Set HUD pain sensor pulse ");
            ItemString += OnOffString;
            Info.Set(ItemString);          
        }
        break;

        default:
            ASSERT(0);
        break;
    }

    return Info.Get();
}
