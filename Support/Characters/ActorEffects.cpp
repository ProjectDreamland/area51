//==============================================================================
//  
//  ActorEffects.cpp
//         
//==============================================================================

#include "ActorEffects.hpp"
#include "Obj_mgr\obj_mgr.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "objects\actor\actor.hpp"

#ifndef X_EDITOR
#include "objects\netghost.hpp"
#endif

#include "objects\Corpse.hpp"
#include "objects\NewWeapon.hpp"
#include "GameLib\RenderContext.hpp"
#include "objects\Player.hpp"
#include "characters\MutantTank\Mutant_Tank.hpp"

#include "..\Support\Sound\EventSoundEmitter.hpp"

//==============================================================================

actor_effects::actor_effects( void )
{
    for( s32 i = 0; i < FX_MAX; i++ )
    {
        m_bActive[i] = FALSE;
        m_AudioID[i] = 0;
    }

    for( s32 i = 0; i < MAX_FRY_POINTS; i++ )
        m_FryBone[i].iBone = -1;

    for( s32 i = 0; i < MAX_CLOAK_POINTS; i++ )
        m_CloakBone[i].iBone = -1;

    m_DeathTimer = 0.0f;
    m_ShockTimer = 0.0f;
}

//==============================================================================

actor_effects::~actor_effects( void )
{
    Kill();
}

//==============================================================================

void actor_effects::Init( void )
{
}

//==============================================================================

void actor_effects::Kill( void )
{
    for( s32 i = 0; i < FX_MAX; i++ )
    {
        KillEffect( (effect_type)i );
    }
}

//==============================================================================

void actor_effects::InitEffect( effect_type Type, object* pParent, s32 iBone )
{
    switch( Type )
    {
    //--------------------------------------------------------------------------
    case FX_FLASHLIGHT:
    //--------------------------------------------------------------------------
        ASSERT( pParent );
        InitBasicEffect( Type, 
                         pParent->GetPosition(), 
                         pParent->GetZone1(), 
                         PRELOAD_FILE( "FlashLight_000.fxo" ), 
                         NULL );
                         
        // Set correct position                         
        UpdateFlashLight( Type, pParent );
        break;

    //--------------------------------------------------------------------------
    case FX_FLAME:
    //--------------------------------------------------------------------------
        InitFryEffect( Type, 
                       pParent, 
                       PRELOAD_FILE( "jm_fire_002.fxo" ), 
                       "FIRE_NPC_LOOP" );
        break;

    //--------------------------------------------------------------------------
    case FX_SHOCK:
    //--------------------------------------------------------------------------
        InitFryEffect( Type, 
                       pParent, 
                       PRELOAD_FILE( "Actor_effect_Electric.fxo" ), 
                       "FIRE_NPC_LOOP" );               // TO DO - Fire sound?
        break;

    //--------------------------------------------------------------------------
    case FX_MUTATE:
        {
            //--------------------------------------------------------------------------
            ASSERT( pParent );
            xbool ParentIsNetGhost = FALSE;

#if !defined( X_EDITOR )
            ParentIsNetGhost = pParent->IsKindOf( net_ghost::GetRTTI() );
#endif

            InitBasicEffect( Type, 
                pParent->GetPosition(), 
                pParent->GetZone1(), 
                PRELOAD_FILE( "MP_MUTATION.fxo" ), 
                ParentIsNetGhost ? "MUT_Switch_To" : NULL );
            break;
            //--------------------------------------------------------------------------
        }

    case FX_UNMUTATE:
        {
            //--------------------------------------------------------------------------
            ASSERT( pParent );
            xbool ParentIsNetGhost = FALSE;

#if !defined( X_EDITOR )
            ParentIsNetGhost = pParent->IsKindOf( net_ghost::GetRTTI() );
#endif

            InitBasicEffect( Type, 
                pParent->GetPosition(), 
                pParent->GetZone1(), 
                PRELOAD_FILE( "MP_MUTATION.fxo" ), 
                ParentIsNetGhost ? "MUT_Switch_From" : NULL );
            break;

            //--------------------------------------------------------------------------
        }
    case FX_SPAWN:
    //--------------------------------------------------------------------------
        ASSERT( pParent );
        KillEffect( Type );
        InitBasicEffect( Type, 
                         pParent->GetPosition(), 
                         pParent->GetZone1(), 
                         PRELOAD_FILE( "MP_Spawn_Shield.fxo" ), 
                         "JumpPad_Launch" );
        break;

    //--------------------------------------------------------------------------
    case FX_CLOAK:
    //--------------------------------------------------------------------------
        InitCloakEffect( Type, 
                         pParent, 
                         PRELOAD_FILE( "blackops_cloak.fxo" ) );
        break;

    //--------------------------------------------------------------------------
    case FX_DECLOAK:
    //--------------------------------------------------------------------------
        InitCloakEffect( Type, 
                         pParent, 
                         PRELOAD_FILE( "blackops_decloak.fxo" ) );
        break;

    //--------------------------------------------------------------------------
    case FX_CLOAK_PAIN:
    //--------------------------------------------------------------------------
        InitCloakEffect( Type, 
                         pParent, 
                         PRELOAD_FILE( "blackops_impact.fxo" ), 
                         iBone );
        break;

    //--------------------------------------------------------------------------
    case FX_CONTAIGON:
    //--------------------------------------------------------------------------
        InitFryEffect( Type, 
            pParent, 
            PRELOAD_FILE( "Contagion_Actor.fxo" ), 
            "" );         // TO DO - Fire sound?
        break;        


    //--------------------------------------------------------------------------
    default:
    //--------------------------------------------------------------------------
        ASSERT( FALSE );
        break;
    }
}

//==============================================================================

void actor_effects::InitBasicEffect(       effect_type Type, 
                                     const vector3&    Position,
                                           s32         Zone,
                                     const char*       pEffectName,
                                     const char*       pAudioName )
{
    if( m_bActive[Type] )
        return;

    rhandle<char> Resource;
    Resource.SetName( pEffectName );

    char* pPtr = Resource.GetPointer();
    ASSERT( pPtr );
    if( !pPtr )
        return;

    m_FXHandle[Type].InitInstance( pPtr );
    m_FXHandle[Type].Restart();
    m_FXHandle[Type].SetTranslation( Position );

    if( pAudioName )
    {
        if( m_AudioID[Type] != 0 )
        {
            g_AudioMgr.Release( m_AudioID[Type], 0.0f );
            m_AudioID[Type] = 0;
        }

        m_AudioID[Type] = g_AudioMgr.Play( pAudioName, 
                                           Position, 
                                           Zone, 
                                           TRUE );
    }

    m_bActive[Type] = TRUE;
}

//==============================================================================

void actor_effects::InitFryEffect(       effect_type Type,
                                         object*     pParent, 
                                   const char*       pEffectName, 
                                   const char*       pAudioName )
{
    KillFryEffect();

    if( pParent && pParent->IsKindOf( actor::GetRTTI() ) )
    {
        actor* pActor = (actor*)pParent;

        rhandle<char> Resource;
        Resource.SetName( pEffectName );

        // Need the parent actor loco player.
        ASSERT( pActor->GetLocoPointer() );
        loco_char_anim_player& Player = pActor->GetLocoPointer()->m_Player;
        
        // Need a range of bones on which to add flames.
        s32 nBones   = MIN( Player.GetNBones(), 16 );
        m_nFryPoints = MIN( nBones, MAX_FRY_POINTS );

        // Fire it up!  Literally.
        for( s32 i = 0; i < m_nFryPoints; i++ )
        {
            m_FryBone[i].iBone = x_irand( 0, nBones-1 );
            m_FryBone[i].FXHandle.InitInstance( Resource.GetPointer() );

            if( Type == FX_SHOCK )
            {
                f32 Scale = x_frand( 0.4f, 0.7f );
                m_FryBone[i].FXHandle.SetScale( vector3( Scale, Scale, Scale ) );
            }
            if( Type == FX_FLAME )
            {
                m_FryBone[i].FXHandle.SetScale( vector3( 0.4f, 0.4f, 0.4f ) );
            }

            if (Type == FX_CONTAIGON)
            {
                // Randomize rotation for contaigon
                radian P,Y;
                P = x_frand(0,R_360);
                Y = x_frand(0,R_360);
                m_FryBone[i].FXHandle.SetRotation( radian3(P,Y,0) );

                if (pParent->IsKindOf( mutant_tank::GetRTTI() ))
                {
                    m_FryBone[i].FXHandle.SetScale( vector3(2,2,2) );
                }
            }
        }
    }

    if( m_AudioID[Type] != 0 )
    {
        g_AudioMgr.Release( m_AudioID[Type], 0.0f );
        m_AudioID[Type] = 0;
    }

    if( pAudioName && pParent )
    {
        m_AudioID[Type] = g_AudioMgr.PlayVolumeClipped( pAudioName, 
                                                        pParent->GetPosition(), 
                                                        pParent->GetZone1(), 
                                                        TRUE );
    }

    m_bActive[Type] = TRUE;
}

//==============================================================================

void actor_effects::InitCloakEffect(       effect_type Type,
                                           object*     pParent, 
                                     const char*       pEffectName, 
                                           s32         iBone )
{
    // Shut down any active cloaking effect.
    if( m_bActive[FX_CLOAK]      )  KillEffect( FX_CLOAK      );
    if( m_bActive[FX_DECLOAK]    )  KillEffect( FX_DECLOAK    );
    if( m_bActive[FX_CLOAK_PAIN] )  KillEffect( FX_CLOAK_PAIN );

    rhandle<char> Resource;
    Resource.SetName( pEffectName );

    ASSERT( pParent && pParent->IsKindOf( actor::GetRTTI() ) );
    actor* pActor = (actor*)pParent;
    const anim_group* pAnimGroup = pActor->GetAnimGroupPtr();

    if( iBone == -1 )
    {
        // Apply this cloaking effect to all bones.

        for( s32 i = 0; i < MAX_CLOAK_POINTS; i++ )
            m_CloakBone[i].FXHandle.InitInstance( Resource.GetPointer() );

        if( pAnimGroup )
        {
            m_CloakBone[ 0].iBone = pAnimGroup->GetBoneIndex( "B_01_Arm_R_UpperArm" );
            m_CloakBone[ 1].iBone = pAnimGroup->GetBoneIndex( "B_01_Arm_R_ForeArm"  );
            m_CloakBone[ 2].iBone = pAnimGroup->GetBoneIndex( "B_01_Leg_R_Thigh"    );
            m_CloakBone[ 3].iBone = pAnimGroup->GetBoneIndex( "B_01_Leg_R_Calf"     );
            m_CloakBone[ 4].iBone = pAnimGroup->GetBoneIndex( "B_01_Arm_L_UpperArm" );
            m_CloakBone[ 5].iBone = pAnimGroup->GetBoneIndex( "B_01_Arm_L_ForeArm"  );
            m_CloakBone[ 6].iBone = pAnimGroup->GetBoneIndex( "B_01_Leg_L_Thigh"    );
            m_CloakBone[ 7].iBone = pAnimGroup->GetBoneIndex( "B_01_Leg_L_Calf"     );
            m_CloakBone[ 8].iBone = pAnimGroup->GetBoneIndex( "B_01_Spine01"        );
            m_CloakBone[ 9].iBone = pAnimGroup->GetBoneIndex( "B_01_Spine02"        );
            m_CloakBone[10].iBone = pAnimGroup->GetBoneIndex( "B_01_Head"           );
        }
    }
    else
    {
        // Apply this cloaking effect to ONE bone.

        m_CloakBone[0].FXHandle.InitInstance( Resource.GetPointer() );

        if( pAnimGroup )
        {
            ASSERT( (iBone >= 0) && (iBone < pAnimGroup->GetNBones()) );
            m_CloakBone[0].iBone = iBone;
        }
    }

    m_bActive[Type] = TRUE;
}

//==============================================================================

void actor_effects::KillEffect( effect_type Type )
{
    switch( Type )
    {
    //--------------------------------------------------------------------------
    case FX_FLASHLIGHT:
    case FX_MUTATE:
    case FX_UNMUTATE:
    case FX_SPAWN:
    //--------------------------------------------------------------------------
        if( m_bActive[Type] )
        {
            m_FXHandle[Type].KillInstance();
            m_bActive[Type] = FALSE;
            if( m_AudioID[Type] != 0 )
                g_AudioMgr.Release( m_AudioID[Type], 0.25f );
        }
        break;

    //--------------------------------------------------------------------------
    case FX_FLAME:
    case FX_SHOCK:
    case FX_CONTAIGON:
    //--------------------------------------------------------------------------
        KillFryEffect();
        break;

    //--------------------------------------------------------------------------
    case FX_CLOAK:
    case FX_DECLOAK:
    case FX_CLOAK_PAIN:
    //--------------------------------------------------------------------------
        if( m_bActive[Type] )
        {
            for( s32 i = 0; i < MAX_CLOAK_POINTS; i++ )
            {
                if( m_CloakBone[i].iBone != -1 )
                {
                    m_CloakBone[i].FXHandle.KillInstance();
                    m_CloakBone[i].iBone = -1;
                }
            }
            m_bActive[Type] = FALSE;
        }
        break;

    //--------------------------------------------------------------------------
    default:
    //--------------------------------------------------------------------------
        break;
    }
}

//==============================================================================

void actor_effects::KillFryEffect( void )
{
    effect_type Type = FX_MAX;

    if( m_bActive[FX_FLAME] )       Type = FX_FLAME;
    if( m_bActive[FX_SHOCK] )       Type = FX_SHOCK;
    if( m_bActive[FX_CONTAIGON] )   Type = FX_CONTAIGON;

    if( Type != FX_MAX )
    {
        for( s32 i = 0; i < m_nFryPoints; i++ )
            m_FryBone[i].FXHandle.KillInstance();
        m_nFryPoints = 0;
        g_AudioMgr.Release( m_AudioID[Type], 0.5f );
        m_AudioID[Type] = 0;
        m_bActive[Type] = FALSE;
    }
}

//==============================================================================
                
xbool actor_effects::IsEffectOn( effect_type Type )
{
    switch( Type )
    {
    //--------------------------------------------------------------------------
    case FX_FLASHLIGHT:
    case FX_FLAME:
    case FX_SHOCK:
    case FX_MUTATE:
    case FX_UNMUTATE:
    case FX_SPAWN:
    case FX_CLOAK:
    case FX_DECLOAK:
    case FX_CLOAK_PAIN:
    case FX_CONTAIGON:
    //--------------------------------------------------------------------------
        return( m_bActive[Type] );
        break;

    //--------------------------------------------------------------------------
    default:
    //--------------------------------------------------------------------------
        ASSERT( FALSE );
        break;
    }

    return( FALSE );
}

//==============================================================================

void actor_effects::UpdateFlashLight( effect_type Type, object* pParent )
{
    // Attached to an actor?
    if( pParent && pParent->IsKindOf( actor::GetRTTI() ) )
    {
        actor*      pActor  = (actor*)pParent;
        new_weapon* pWeapon = pActor->GetCurrentWeaponPtr();
        matrix4     L2W;
        vector3     Offset;

        // Transform the flashlight effect based on the weapon.
        if( pWeapon )
        {
            new_weapon::render_state OldRenderState = pWeapon->GetRenderState();
            pWeapon->SetRenderState( new_weapon::RENDER_STATE_NPC );
            if ( pWeapon->GetFlashlightTransformInfo( L2W, Offset ) )
            {
                L2W.PreTranslate( Offset );
                m_FXHandle[Type].SetRotation( L2W.GetRotation() );
                m_FXHandle[Type].SetTranslation( L2W.GetTranslation() );
            }
            pWeapon->SetRenderState( OldRenderState );
        }
        else
        {
            // Kill flashlight if no weapon to follow.
            KillEffect( FX_FLASHLIGHT );
        }
    }
    else
    {
        // Kill flashlight if no actor to follow.
        KillEffect( FX_FLASHLIGHT );
    }       
}

//==============================================================================

void actor_effects::Update( object* pParent, f32 DeltaTime )
{
    for( s32 Type = 0; Type < FX_MAX; Type++ )
    {
        if( !m_bActive[Type] )
            continue;

        switch( Type )
        {
        //----------------------------------------------------------------------
        case FX_FLASHLIGHT:
        //----------------------------------------------------------------------
        
            UpdateFlashLight( (effect_type)Type, pParent );
            break;

        //----------------------------------------------------------------------
        case FX_MUTATE:
        case FX_UNMUTATE:
        case FX_SPAWN:
        //----------------------------------------------------------------------
            {
                ASSERT( pParent );
                if( pParent->IsKindOf( actor::GetRTTI() ) )
                {
                    f32 I = ((actor*)pParent)->GetFloorIntensity();
                    I /= 255.0f;
                    I *=   0.8f;    // This...
                    I +=   0.2f;    //   ...plus this should sum to 1.0.
                    I *= 255.0f;
                    s32 i = (s32)I;

                    m_FXHandle[Type].SetColor( xcolor(i,i,i) );
                }

                m_FXHandle[Type].AdvanceLogic( DeltaTime );
                m_FXHandle[Type].SetTranslation( pParent->GetPosition() );

                if( m_FXHandle[Type].IsFinished() )
                    m_bActive[Type] = FALSE;
                if( m_AudioID[Type] != 0 )
                    g_AudioMgr.SetPosition( m_AudioID[Type], 
                                            pParent->GetPosition(), 
                                            pParent->GetZone1() );
            } 
            break;

        //----------------------------------------------------------------------
        case FX_FLAME:
        case FX_SHOCK:
        case FX_CONTAIGON:
        //----------------------------------------------------------------------
            for( s32 i = 0; i < m_nFryPoints; i++ )
            {
                ASSERT( m_FryBone[i].iBone != -1 );
                {
                    vector3 Position = GetBonePosition( pParent, m_FryBone[i].iBone );
                    m_FryBone[i].FXHandle.SetTranslation( Position );
                    m_FryBone[i].FXHandle.AdvanceLogic( DeltaTime );
                }
            }
            break;

        //----------------------------------------------------------------------
        case FX_CLOAK:
        case FX_DECLOAK:
        case FX_CLOAK_PAIN:
        //----------------------------------------------------------------------
            for( s32 i = 0; i < MAX_CLOAK_POINTS; i++ )
            {
                xbool Finished = TRUE;  // Assume true, disprove below.
                if( m_CloakBone[i].iBone != -1 )
                {
                    matrix4 L2W;
                    GetBoneL2W( pParent, m_CloakBone[i].iBone, L2W );
                    m_CloakBone[i].FXHandle.SetTranslation( L2W.GetTranslation() );
                    m_CloakBone[i].FXHandle.SetRotation   ( L2W.GetRotation() );
                    m_CloakBone[i].FXHandle.AdvanceLogic  ( DeltaTime );
                    if( !m_CloakBone[i].FXHandle.IsFinished() )
                        Finished = FALSE;
                }
                if( Finished )
                    KillEffect( (effect_type)Type );
            }
            break;

        //----------------------------------------------------------------------
        default:
        //----------------------------------------------------------------------
            ASSERT( FALSE );
            break;
        }
    }

    if( m_DeathTimer != 0.0f )
    {
        m_DeathTimer -= DeltaTime;
        if( m_DeathTimer <= 0.0f )
        {
            if( pParent )
            {
                pain Pain;
                Pain.Setup( "GENERIC_LETHAL", 0, pParent->GetColBBox().GetCenter() );
                Pain.SetDirectHitGuid( pParent->GetGuid() );
                Pain.ApplyToObject( pParent );
            }
            m_DeathTimer = 0.0f;
        }
    }

    if( m_ShockTimer != 0.0f )
    {
        m_ShockTimer -= DeltaTime;
        if( m_ShockTimer <= 0.0f )
        {
            KillEffect( FX_SHOCK );
            m_ShockTimer = 0.0f;
        }
    }
}

//==============================================================================

const vector3 actor_effects::GetBonePosition( object* pParent, s32 iBone )
{
    if( pParent->IsKindOf( actor::GetRTTI() ) )
    {
        actor* pActor = (actor*)pParent;
        if( pActor->GetLocoPointer() )
        {
            loco_char_anim_player& Player = pActor->GetLocoPointer()->m_Player;
            return( Player.GetBonePosition( iBone ) );
        }
    }
    else if( pParent->IsKindOf( corpse::GetRTTI() ) )
    {
        corpse*       pCorpse = (corpse*)pParent;
        physics_inst& PhysicsInst = pCorpse->GetPhysicsInst();
        return( PhysicsInst.GetBoneWorldPosition( iBone ) );
    }

    ASSERT( FALSE );
    return( vector3( 0.0f, 0.0f, 0.0f ) );
}

//==============================================================================

void actor_effects::GetBoneL2W( object* pParent, s32 iBone, matrix4& L2W )
{
    if( pParent->IsKindOf( actor::GetRTTI() ) )
    {
        actor* pActor = (actor*)pParent;
        if( pActor->GetLocoPointer() )
        {
            // Get animation player from actor.
            loco_char_anim_player& Player = pActor->GetLocoPointer()->m_Player;

            // Get the bone L2W making sure to do the proper conversion to take
            // out the extra skin offset.
            L2W = Player.GetBoneL2W( iBone );
            L2W.PreTranslate( Player.GetBoneBindPosition( iBone ) );
        }
    }
    else if( pParent->IsKindOf( corpse::GetRTTI() ) )
    {
        // Snag the L2W from the ragdoll.
        corpse*       pCorpse = (corpse*)pParent;
        physics_inst& PhysicsInst = pCorpse->GetPhysicsInst();
        L2W = PhysicsInst.GetBoneWorldTransform( iBone );
    }
    else
    {
        ASSERT( FALSE );
    }
}

//==============================================================================

void actor_effects::Render( object* pParent )
{
    (void)pParent;
}

//==============================================================================

void actor_effects::RenderTransparent( object* pParent, f32 Alpha )
{
    for( s32 Type = 0; Type < FX_MAX; Type++ )
    {
        if( !m_bActive[Type] )
            continue;

        switch( Type )
        {
        //----------------------------------------------------------------------
        case FX_FLASHLIGHT:
        case FX_MUTATE:
        case FX_UNMUTATE:
        //----------------------------------------------------------------------
            if( pParent->IsKindOf( actor::GetRTTI() ) )
            {
                actor* pActor = (actor*)pParent;
                if( (g_RenderContext.NetPlayerSlot != pActor->net_GetSlot()) )
                    m_FXHandle[Type].Render();
            }
            else
            {
                m_FXHandle[Type].Render();
            }
            break;

        //----------------------------------------------------------------------
        case FX_SPAWN:
        //----------------------------------------------------------------------
            m_FXHandle[Type].Render();
            break;            
            
        //----------------------------------------------------------------------
        case FX_FLAME:
        case FX_SHOCK:
        case FX_CONTAIGON:
        //----------------------------------------------------------------------
            {
                xbool bRender = TRUE;
                object_ptr<actor>pActor(pParent);
                
                if (pActor.IsValid())                
                {                    
                    if (g_RenderContext.NetPlayerSlot == pActor->net_GetSlot() )
                    {
                        bRender = FALSE;
                    }
                }

                if (bRender)
                {
                    for( s32 i = 0; i < m_nFryPoints; i++ )
                    {
                        xcolor Color( 255, 255, 255, (u8)(Alpha * 255) );
                        m_FryBone[i].FXHandle.SetColor( Color );
                        m_FryBone[i].FXHandle.Render();
                    }
                }
            }
            break;

        //----------------------------------------------------------------------
        case FX_CLOAK:
        case FX_DECLOAK:
        case FX_CLOAK_PAIN:
        //----------------------------------------------------------------------
            for( s32 i = 0; i < MAX_CLOAK_POINTS; i++ )
            {
                if( m_CloakBone[i].iBone >= 0 )
                    m_CloakBone[i].FXHandle.Render();
            }
            break;
        //----------------------------------------------------------------------
        default:
        //----------------------------------------------------------------------
            ASSERT( FALSE );
            break;
        }
    }
}

//==============================================================================

void actor_effects::SetDeathTimer( f32 DeathTimer )
{
    if( DeathTimer > 10000.0f )
        m_DeathTimer = 0.0f;
    else
        m_DeathTimer = DeathTimer;
}

//==============================================================================

void actor_effects::SetShockTimer( f32 Timer )
{
    if( Timer > 10000.0f )
        m_ShockTimer = 0.0f;
    else
        m_ShockTimer = Timer;
}

//==============================================================================

xbool actor_effects::IsActive( void )
{
    for( s32 i = 0; i < FX_MAX; i++ )
        if( m_bActive[i] )
            return( TRUE );

    return( FALSE );
}

//==============================================================================
