#include "..\Support\Trigger\Actions\change_player_strain.hpp"
#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "Objects\Player.hpp"
#include "Entropy.hpp"

//=========================================================================
// CHANGE_PLAYER_STRAIN
//=========================================================================

change_player_strain::change_player_strain( guid ParentGuid ) : 
    actions_base( ParentGuid ),
    m_StrainToSet( player::STRAIN_HUMAN )
{
}

//=============================================================================

void change_player_strain::Execute ( trigger_object* pParent )
{ 
    TRIGGER_CONTEXT( "ACTION * change_player_strain::Execute" );

    ( void ) pParent;

    // Let's walk through the players until we find an 'all strains' player.
    slot_id PlayerSlot = g_ObjMgr.GetFirst( object::TYPE_PLAYER ) ;

    while ( PlayerSlot != SLOT_NULL )
    {
        player* pCandidatePlayer = (player*)g_ObjMgr.GetObjectBySlot( PlayerSlot ) ;
        if ( pCandidatePlayer && pCandidatePlayer->IsKindOf( player::GetRTTI() ) )
        {
            player* pPlayer = ( player* )pCandidatePlayer;
            pPlayer->SetupStrain( m_StrainToSet );
            return;
        }

        PlayerSlot = g_ObjMgr.GetNext( PlayerSlot ) ;
    }
}

//===========================================================================

void change_player_strain::OnEnumProp ( prop_enum& rPropList )
{
    rPropList.AddEnum (  "Strain", player::GetStrainEnum(),
                         "Strain to set." ) ;
    
    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool change_player_strain::OnProperty ( prop_query& rPropQuery )
{
    if( actions_base::OnProperty( rPropQuery ) )
    {
        return TRUE;
    }

    if( rPropQuery.IsVar( "Strain" ) )
    {
        if ( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarEnum( player::GetStrainName( m_StrainToSet ) );
        }
        else
        {
            m_StrainToSet = player::GetStrainByName( rPropQuery.GetVarEnum() );
        }
        return TRUE;
    }
   
    return FALSE;
}
