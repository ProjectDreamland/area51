#include "CheckPointMgr.hpp"
#include "GameLib\LevelLoader.hpp"
#include "Objects\Render\PostEffectMgr.hpp"
#include "..\Support\TriggerEx\TriggerEx_Object.hpp"
#include "Objects\LevelSettings.hpp"
#include "Sound\SoundEmitter.hpp"
#include "objects\hudobject.hpp"
#include "StateMgr/PlayerProfile.hpp"
#include "StateMgr/StateMgr.hpp"

check_point_mgr g_CheckPointMgr;

//==============================================================================

void ActivateSoundEmitters( const vector3& Position )
{
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_SND_EMITTER );
    object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );

    while( pObject )
    {
        sound_emitter& Emitter = sound_emitter::GetSafeType( *pObject );
        Emitter.OnTeleportActivate( Position );

        SlotID = g_ObjMgr.GetNext( SlotID );
        pObject = g_ObjMgr.GetObjectBySlot( SlotID );
    }
}

//==============================================================================

s32 check_point_mgr::GetCheckPointIndex( void )
{
    return m_Level.iCurrentCheckPoint;
}

//==============================================================================

void check_point_mgr::SetCheckPointIndex( s32 iCheckPoint )
{
    if( m_Level.nValidCheckPoints > 0 )
    {
        ASSERT( (iCheckPoint >= 0) && (iCheckPoint < MAX_CHECKPOINTS) );
        ASSERT( iCheckPoint < m_Level.nValidCheckPoints );
        ASSERT( m_Level.CheckPoints[iCheckPoint].bIsValid );
        m_Level.iCurrentCheckPoint = iCheckPoint;
    }
}

//==============================================================================

void check_point_mgr::Reinit( s32 MapID )
{
#ifndef X_EDITOR
    player_profile& Profile = g_StateMgr.GetActiveProfile(0);
    level_check_points* pCP = Profile.GetCheckpointByMapID( MapID );
    if( pCP )
    {
        m_Level = *pCP;
        m_Level.iCurrentCheckPoint = -1;
    }
    else
#endif
    {
        m_Level.Init( MapID );
    }
}

//==============================================================================

xbool check_point_mgr::Restore( xbool bIsDebugAdvance )
{
    (void)bIsDebugAdvance;
    object* pObject;
    xbool   Result = FALSE;

    // Stop all audio.
    g_AudioMgr.ReleaseAll();

    LOG_MESSAGE( "check_point_mgr::Restore", "Restoring checkpoint" );

    // Find the damn player.
    slot_id ID      = g_ObjMgr.GetFirst( object::TYPE_PLAYER );
    player* pPlayer = (player*)g_ObjMgr.GetObjectBySlot( ID );
    ASSERT( pPlayer );
    if( pPlayer )
    {
        // Gotta be in range!
        ASSERT( (m_Level.iCurrentCheckPoint >= 0) && (m_Level.iCurrentCheckPoint < MAX_CHECKPOINTS) );
        if( (m_Level.iCurrentCheckPoint >= 0) && (m_Level.iCurrentCheckPoint < MAX_CHECKPOINTS) )
        {
            // Checkpoint *has* to be valid.
            check_point& CP = m_Level.CheckPoints[ m_Level.iCurrentCheckPoint ];
            ASSERT( CP.bIsValid );
            if( CP.bIsValid )
            {
                // Now that the inventory is correct, redo the inventory.
                pPlayer->ReInitInventory();

                // Update player data.
                pPlayer->m_CurrentWeaponItem             = (inven_item)CP.CurrWeapon;
                pPlayer->m_PrevWeaponItem                = (inven_item)CP.PrevWeapon;
                pPlayer->m_NextWeaponItem                = (inven_item)CP.NextWeapon; 
                pPlayer->m_bMutationMeleeEnabled         = CP.MutantMelee;
                pPlayer->m_bPrimaryMutationFireEnabled   = CP.MutantPrimary;
                pPlayer->m_bSecondaryMutationFireEnabled = CP.MutantSecondary;
                pPlayer->m_Mutagen                       = CP.Mutagen;
                pPlayer->ResetHealth( CP.Health );
                pPlayer->SetMaxHealth( CP.MaxHealth );

                // Set up the inventory.
                for( s32 i=0 ; i<INVEN_COUNT ; i++ )
                {
                    inven_item Item = (inven_item)i;
                    pPlayer->m_Inventory2.SetAmount( Item, CP.Inventory[i] );
                }

                // Update all the ammo.
                for( s32 i=0 ; i<INVEN_NUM_WEAPONS ; i++ )
                {
                    new_weapon* pWeapon = pPlayer->GetWeaponPtr( inventory2::WeaponIndexToItem(i) );
                    if( pWeapon )
                    {
                        pWeapon->SetAmmoState2( new_weapon::AMMO_PRIMARY,   CP.Ammo[i*2].Amount,   CP.Ammo[i*2].CurrentClip   );
                        pWeapon->SetAmmoState2( new_weapon::AMMO_SECONDARY, CP.Ammo[i*2+1].Amount, CP.Ammo[i*2+1].CurrentClip );
                    }
                }

                // Switch to the correct weapon.
                pPlayer->SetNextWeapon2( pPlayer->m_CurrentWeaponItem, TRUE );

#if !defined(X_RETAIL)
                // Only do the debug advance trigger if its from debug menu.
                if( bIsDebugAdvance )
                {
                    // Activate the debug advance trigger.
                    pObject = g_ObjMgr.GetObjectByGuid( CP.AdvanceGUID );
                    if( pObject && pObject->IsKindOf(trigger_ex_object::GetRTTI()) )
                    {
                        trigger_ex_object &Trigger = trigger_ex_object::GetSafeType( *pObject );

                        // Force it to be active and such!
                        Trigger.ForceStartTrigger();

                        // Run the debug advance trigger logic once.
                        Trigger.OnAdvanceLogic( 0.033f );
                    }
                }
#endif    

                // Haveing the HUD object init itself in the onRender we need to do
                // some hacking here to get the hud object ready for the restore.
                // So the first thing we do is find the hud object in the object mgr.
                slot_id ID = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );
                hud_object* pHudObject = (hud_object*)g_ObjMgr.GetObjectBySlot( ID );
                if( pHudObject )
                {
                    // Now that we have the hud object let get the player.
                    slot_id PlayerID      = g_ObjMgr.GetFirst( object::TYPE_PLAYER );
                    player* pPlayer = (player*)g_ObjMgr.GetObjectBySlot( PlayerID );
                    if( pPlayer )
                    {
                        // We need to compute the view before we init the hud so that
                        // the safe zones for hud rendering are correct.
                        s32     XRes, YRes;
                        eng_GetRes( XRes, YRes );
                        view& rView0 = pPlayer->GetView();
                        rView0.SetViewport( 0, 0, XRes, YRes );

                        // Finally init the hud.
                        pHudObject->InitHud();
                    }
                }

                // Find the level settings.
                ID                        = g_ObjMgr.GetFirst( object::TYPE_LEVEL_SETTINGS );
                level_settings* pSettings = (level_settings*)g_ObjMgr.GetObjectBySlot( ID );

                // All good?
                if( pSettings && pSettings->IsKindOf(level_settings::GetRTTI()) )
                {
                    // Get the startup trigger.
                    guid    GUID    = pSettings->GetStartupGuid();
                    object* pObject = g_ObjMgr.GetObjectByGuid( GUID );

                    // All good?
                    if( pObject && pObject->IsKindOf(trigger_ex_object::GetRTTI()) )
                    {
                        trigger_ex_object &Trigger = trigger_ex_object::GetSafeType( *pObject );

                        // Force it to be active and such!
                        Trigger.ForceStartTrigger();

                        // Run the trigger logic once.
                        Trigger.OnAdvanceLogic( 0.033f );
                        
                        // Now NUKE it!
                        g_ObjMgr.DestroyObject( GUID );

                        // Now flush any deleted objects.
                        g_ObjMgr.EmptyDeleteObjectList();
                    }
                }

                // Activate the respawn trigger.
                pObject = g_ObjMgr.GetObjectByGuid( CP.RespawnGUID );
                if( pObject && pObject->IsKindOf(trigger_ex_object::GetRTTI()) )
                {
                    trigger_ex_object &Trigger = trigger_ex_object::GetSafeType( *pObject );
                    
                    // Force it to be active and such!
                    Trigger.ForceStartTrigger();
             
                    // Run the respawn trigger logic once.
                    Trigger.OnAdvanceLogic( 0.033f );
                    
                    // Now NUKE it!
                    g_ObjMgr.DestroyObject( CP.RespawnGUID );

                    // Now flush any deleted objects.
                    g_ObjMgr.EmptyDeleteObjectList();
                }

                // Fade in.
                g_PostEffectMgr.StartScreenFade( xcolor(0,0,0,0), 3.0f );

                // Its all good!
                Result = TRUE;
            }
        }
    }

    // Tell the world.
    return Result;
}

//==============================================================================

xbool check_point_mgr::SetCheckPoint( guid RespawnGUID, 
                                      guid DebugAdvanceGUID, 
                                      s32  TableName,
                                      s32  TitleName )
{
    xbool Result = FALSE;

    // Find the damn player.
    LOG_MESSAGE( "check_point_mgr::SetCheckPoint","Checkpoint set RespawnGUID:%08x, DebugAdvanceGUID:%08x", RespawnGUID, DebugAdvanceGUID );
    slot_id ID = g_ObjMgr.GetFirst( object::TYPE_PLAYER );
    player* pPlayer = (player*)g_ObjMgr.GetObjectBySlot( ID );
    ASSERT( pPlayer );
    if( pPlayer )
    {
        // Woot! Past another checkpoint! (has to be in range)
        m_Level.iCurrentCheckPoint++;
        ASSERT( (m_Level.iCurrentCheckPoint >= 0) && (m_Level.iCurrentCheckPoint < MAX_CHECKPOINTS) );
        if( (m_Level.iCurrentCheckPoint >= 0) && (m_Level.iCurrentCheckPoint < MAX_CHECKPOINTS) )
        {
            check_point& CP = m_Level.CheckPoints[ m_Level.iCurrentCheckPoint ];

            // Set the guids.
            CP.TableName   = TableName;
            CP.TitleName   = TitleName;
            CP.RespawnGUID = RespawnGUID;
            CP.AdvanceGUID = DebugAdvanceGUID;

            // Save off player data
            if( pPlayer->IsMutated() )
                CP.CurrWeapon      = pPlayer->m_PreMutationWeapon2;
            else
                CP.CurrWeapon      = pPlayer->m_CurrentWeaponItem;
            CP.PrevWeapon      = pPlayer->m_PrevWeaponItem;
            CP.NextWeapon      = pPlayer->m_NextWeaponItem;
            CP.MutantMelee     = pPlayer->m_bMutationMeleeEnabled;
            CP.MutantPrimary   = pPlayer->m_bPrimaryMutationFireEnabled;
            CP.MutantSecondary = pPlayer->m_bSecondaryMutationFireEnabled;
            CP.Mutagen         = pPlayer->m_Mutagen;
            CP.Health          = pPlayer->GetHealth();
            CP.MaxHealth       = pPlayer->GetMaxHealth();

            // Snag the player inventory.
            for( s32 i=0 ; i<INVEN_COUNT ; i++ )
            {
                inven_item Item = (inven_item)i;
                CP.Inventory[i] = pPlayer->m_Inventory2.GetAmount( Item );
            }

            // Snag the ammo.
            for( s32 i=0 ; i<INVEN_NUM_WEAPONS ; i++ )
            {
                new_weapon* pWeapon = pPlayer->GetWeaponPtr( inventory2::WeaponIndexToItem(i) );
                if( pWeapon )
                {
                    s32 AmmoAmount, AmmoMax, AmmoPerClip, AmmoInCurrentClip;
                    pWeapon->GetAmmoState(   new_weapon::AMMO_PRIMARY, AmmoAmount, AmmoMax, AmmoPerClip, AmmoInCurrentClip );
                    CP.Ammo[i*2].Amount      = AmmoAmount;
                    CP.Ammo[i*2].CurrentClip = AmmoInCurrentClip;
                    pWeapon->GetAmmoState( new_weapon::AMMO_SECONDARY, AmmoAmount, AmmoMax, AmmoPerClip, AmmoInCurrentClip );
                    CP.Ammo[i*2+1].Amount      = AmmoAmount;
                    CP.Ammo[i*2+1].CurrentClip = AmmoInCurrentClip;
                }
                else
                {
                    CP.Ammo[i*2].Amount        = 0;
                    CP.Ammo[i*2].CurrentClip   = 0;
                    CP.Ammo[i*2+1].Amount      = 0;
                    CP.Ammo[i*2+1].CurrentClip = 0;
                }
            }

            // One more checkpoint is valid!
            if( (m_Level.iCurrentCheckPoint+1) > m_Level.nValidCheckPoints )
                m_Level.nValidCheckPoints = m_Level.iCurrentCheckPoint+1;
            
            // Ok, its valid now!
            CP.bIsValid = TRUE;

            // Its all good!
            Result = TRUE;
        }
    }

    // Tell the world.
    return Result;
}

//==============================================================================

s32 check_point_mgr::GetMapID( void )
{
    return m_Level.MapID;
}

//==============================================================================

s32 check_point_mgr::Read( bitstream& in )
{
    (void)in;
    return 0;
}

//==============================================================================

s32 check_point_mgr::Write( bitstream& out )
{
    (void)out;
    return 0;
}
