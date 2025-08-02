//=============================================================================
//
//  Force Linker to include all object types
//
//=============================================================================

#include "Render\Texture.hpp"
#include "Objects\LightObject.hpp"
#include "Objects\PlaySurface.hpp"
#include "Objects\SkinPropSurface.hpp"
#include "Objects\AnimSurface.hpp"
#include "Objects\Player.hpp"
#ifndef X_EDITOR
#include "Objects\NetGhost.hpp"
#endif
#include "Sound\SoundEmitter.hpp"
#include "Sound\SimpleSoundEmitter.hpp"
#include "Sound\EventSoundEmitter.hpp"
#include "Objects\PropSurface.hpp"
#include "Objects\Pickup.hpp"
#include "Objects\Door.hpp"
#include "Objects\NewWeapon.hpp"
#include "Objects\WeaponShotgun.hpp"
#include "Objects\WeaponDualShotgun.hpp"
#include "Objects\WeaponSMP.hpp"
#include "Objects\WeaponDualSMP.hpp"
#include "Objects\WeaponGauss.hpp"
#include "Objects\WeaponSniper.hpp"
#include "Objects\WeaponDesertEagle.hpp"
#include "Objects\WeaponMHG.hpp"
#include "Objects\WeaponMSN.hpp"
#include "Objects\WeaponBBG.hpp"
#include "Objects\WeaponTRA.hpp"
#include "Objects\WeaponScanner.hpp"
#include "Objects\WeaponMutation.hpp"
#include "Objects\ProjectileBullett.hpp"
#include "Objects\Corpse.hpp"
#include "Objects\ParticleEmiter.hpp"
#include "Objects\Portal.hpp"
#include "Objects\Projector.hpp"
#include "Objects\ClothObject.hpp"
#include "Objects\marker_object.hpp"
#include "Objects\InputSetting.hpp"
#include "Objects\LevelSettings.hpp"
#include "Objects\ProxyPlaySurface.hpp"
#include "Objects\Cinema.hpp"
#include "Characters\TaskSystem\character_task_set.hpp"
#include "..\Support\TriggerEx\TriggerEx_Object.hpp"
#include "Characters\Soldiers\Soldier.hpp"
#include "Characters\God.hpp"
#include "Characters\Gray\Gray.hpp"
#include "Characters\MutantTank\Mutant_Tank.hpp"
#include "characters\genericNPC\GenericNPC.hpp"
#include "characters\grunt\grunt.hpp"
#include "characters\scientist\friendlyscientist.hpp"
#include "Navigation\CoverNode.hpp"
#include "Navigation\AlarmNode.hpp"
#include "Debris\debris.hpp"
#include "Debris\debris_rigid.hpp"
#include "Objects\ParticleEventEmitter.hpp"
#include "Objects\Manipulator.hpp"
#include "Objects\animation_obj.hpp"

#include "Objects\GlassSurface.hpp"
#include "Objects\Ladders\Ladder_Field.hpp"
#include "Objects\Spawner\SpawnerObject.hpp"
#include "Objects\FocusObject.hpp"
#include "Objects\LoreObject.hpp"
#include "Objects\HudObject.hpp"
#include "Objects\DamageField.hpp"
#include "Objects\DestructibleObj.hpp"
#include "Objects\SpawnPoint.hpp"
#include "Objects\notepad_object.hpp"
#include "Objects\LensFilter.hpp"

#include "Objects\CokeCan.hpp"
#include "Objects\SuperDestructible.hpp"


#include "Objects\VideoWall.hpp"
#include "Objects\Path.hpp"
#include "Objects\Tracker.hpp"
#include "Objects\Camera.hpp"
#include "Objects\Pip.hpp"
#include "Objects\Controller.hpp"
#include "Objects\Turret.hpp"
#include "Objects\NavPoint.hpp"
#include "Objects\ProjectileExplosiveBullett.hpp"
#include "Objects\Coupler.hpp"
#include "Objects\ProjectileHoming.hpp"
#include "Objects\ProjectileMutantParasite2.hpp"
#include "Objects\ProjectileMutantContagion.hpp"
#include "Objects\ProjectileMutantTendril.hpp"
#include "Objects\ProjectileEnergy.hpp"
#include "Objects\ProjectileMesonSeeker.hpp"
#include "Objects\InvisWall.hpp"
#include "Objects\VolumetricLight.hpp"
#include "Objects\ProjectileAlienTurret.hpp"
#include "Objects\AlienOrb.hpp"
#include "Objects\AlienSpawnTube.hpp"
#include "Objects\AlienGlob.hpp"
#include "Objects\AlienShield.hpp"

#include "Objects\GZCoreObj.hpp"
#include "Objects\feedbackemitter.hpp"

#include "Objects\Group.hpp"

#include "Debris\debris_glass_cluster.hpp"

#include "Objects\GameProp.hpp"
#include "Objects\Flag.hpp"
#include "Objects\BluePrintBag.hpp"
#include "Objects\MP_Settings.hpp"
#include "Objects\JumpPad.hpp"
#include "Objects\Teleporter.hpp"
#include "Objects\TeamProp.hpp"
#include "Objects\TeamLight.hpp"
#include "Objects\ForceField.hpp"
#include "Objects\FlagBase.hpp"

#include "Objects\MutagenReservoir.hpp"
#include "Objects\ReactiveSurface.hpp"

//=============================================================================

volatile xbool NoOpt  = FALSE;
volatile void* pDummy = NULL;

//=============================================================================

void ForceLink( void )
{
    if( NoOpt )
    {
        pDummy = new texture;
//      pDummy = new render_update;
        pDummy = new light_obj;
        pDummy = new play_surface;
        pDummy = new anim_surface;
		pDummy = new door;
        pDummy = new sound_emitter;
        pDummy = new prop_surface;
        pDummy = new pickup;
		pDummy = new corpse;
        pDummy = new notepad_object;
        pDummy = new lens_filter;

        pDummy = new player;
#ifndef X_EDITOR
        pDummy = new net_ghost;
#endif

        pDummy = new new_weapon;
		pDummy = new weapon_smp;
        pDummy = new weapon_dual_smp;
		pDummy = new weapon_shotgun;
        pDummy = new weapon_dual_shotgun;
		pDummy = new weapon_sniper_rifle;
        pDummy = new weapon_desert_eagle;
        pDummy = new weapon_msn;
        pDummy = new weapon_bbg;
        pDummy = new weapon_tra;
        pDummy = new weapon_scanner;
        pDummy = new weapon_mutation;
		pDummy = new bullet_projectile;
        pDummy = new homing_projectile;
        pDummy = new mutant_parasite_projectile;
        pDummy = new mutant_contagion_projectile;
        pDummy = new mutant_tendril_projectile;
        pDummy = new energy_projectile;
        pDummy = new mesonseeker_projectile;

        pDummy = new particle_emitter;
        pDummy = new zone_portal;
 		pDummy = new trigger_ex_object;
        // DECAL TODO: Replace this once the new decal manager is finished
    //  pDummy = new static_decal_object;

        pDummy = new character_task_set;
        pDummy = new soldier;
        pDummy = new hazmat;
        pDummy = new god;
        pDummy = new gray;
        pDummy = new grunt;
        pDummy = new genericNPC;
        pDummy = new mutant_tank;

        pDummy = new cover_node;
        pDummy = new alarm_node;
        pDummy = new debris;
        pDummy = new debris_rigid;
        pDummy = new skin_prop_surface;
        pDummy = new projector_obj;
        pDummy = new friendly_scientist;
        pDummy = new simple_sound_emitter;
        pDummy = new event_sound_emitter;
        pDummy = new cloth_object ;
        pDummy = new particle_event_emitter;
        pDummy = new ladder_field;
        pDummy = new video_wall;

        pDummy = new marker_object;
        pDummy = new input_setting;
        pDummy = new level_settings;
        pDummy = new glass_surface;
        pDummy = new manipulator;
        pDummy = new animation_obj;
        pDummy = new spawn_point;

        pDummy = new spawner_object;
        pDummy = new focus_object;
        pDummy = new lore_object;
        pDummy = new hud_object;
        pDummy = new proxy_playsurface;
        pDummy = new reactive_surface;

        pDummy = new damage_field;    
        pDummy = new destructible_obj;

        pDummy = new path;
        pDummy = new tracker;
        pDummy = new camera;
        pDummy = new pip;
        pDummy = new controller;
		pDummy = new turret;
        pDummy = new nav_point;
        pDummy = new explosive_bullet_projectile;

        pDummy = new coupler;
    //  pDummy = new alien_turret_projectile;
	    pDummy = new alien_orb;
        pDummy = new alien_spawn_tube;
        pDummy = new alien_glob;
        pDummy = new alien_shield;

        pDummy = new ground_zero_core_obj;
        pDummy = new feedback_emitter;
                     
        pDummy = new volumetric_light_obj;
        pDummy = new coke_can;
        pDummy = new super_destructible_obj;
        pDummy = new invisible_wall_obj;

        pDummy = new debris_glass_cluster;

        pDummy = new group;

        pDummy = new mutagen_reservoir;
        pDummy = new cinema_object;
        // BEGIN - Multiplayer stuff.
        pDummy = new game_prop;
        pDummy = new flag;
        pDummy = new blueprint_bag;
        pDummy = new mp_settings;
        pDummy = new jump_pad;
        pDummy = new teleporter;
        pDummy = new force_field;
        pDummy = new team_prop;
        pDummy = new team_light;
        pDummy = new flag_base;
        // END   - Multiplayer stuff.
    }
}

//=============================================================================
