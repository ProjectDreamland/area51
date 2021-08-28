//==============================================================================
//
//  SuperDestructible.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  define HERE
//
//==============================================================================

//==========================================================================
// INCLUDE
//==========================================================================
#include "Obj_mgr\obj_mgr.hpp"
#include "SuperDestructible.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "GameLib\RigidGeomCollision.hpp"
#include "Render\Render.hpp"
#include "Debris\Debris_mgr.hpp"
#include "Objects\ParticleEmiter.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Decals\DecalMgr.hpp"
#include "EventMgr\EventMgr.hpp"
#include "Object.hpp"
#include "Actor/actor.hpp"

                                       
//==========================================================================
// DEFINES
//==========================================================================
#define MAX_PAIN_RESPONSES          16
#define NUM_FADING_RESPONSES        10

#define SUPER_D_DATA_VERSION        1001

xstring g_SuperDestructibleStringList;
xstring g_SuperDestructiblePlayAnimStringList;

// Anim list
struct namelist
{
    char Name[64][256];
};
 
// Pain
struct pain_response_info
{
    pain_response_info()
    {
        Emitter         = 0;
        Owner           = 0;
        AudioVoice      = 0;
        InitialScale    = 1.0f;
        Pos.Set(0,0,0);
    }
    guid        Emitter;
    guid        Owner;
    voice_id    AudioVoice;
    f32         InitialScale;
    vector3     Pos;
};

static pain_response_info       s_PainResponseInfo[ MAX_PAIN_RESPONSES ];
static s32                      s_iNextPainResponse = 0;

//==========================================================================
// GLOBAL
//==========================================================================

static struct super_destructible_obj_desc : public object_desc
{
    //-------------------------------------------------------------------------

    super_destructible_obj_desc( void ) : object_desc( 

            object::TYPE_SUPER_DESTRUCTIBLE_OBJ, 
            "Super Destructible Object", 
            "PROPS",

            object::ATTR_COLLIDABLE         | 
            object::ATTR_BLOCKS_ALL_PROJECTILES | 
            object::ATTR_BLOCKS_ALL_ACTORS | 
            object::ATTR_BLOCKS_RAGDOLL | 
            object::ATTR_BLOCKS_CHARACTER_LOS | 
            object::ATTR_BLOCKS_PLAYER_LOS | 
            object::ATTR_BLOCKS_PAIN_LOS | 
            object::ATTR_BLOCKS_SMALL_DEBRIS | 
            object::ATTR_RENDERABLE         |
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_SPACIAL_ENTRY      |
            object::ATTR_DESTRUCTABLE_OBJECT|
            object::ATTR_DAMAGEABLE, 

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC            |
            FLAGS_NO_ICON               |
            FLAGS_BURN_VERTEX_LIGHTING  ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void )
    {
        return new super_destructible_obj;
    }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR
    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        
        if( (Object. GetAttrBits() & object::ATTR_EDITOR_SELECTED) || 
            (Object.GetAttrBits() & object::ATTR_EDITOR_PLACEMENT_OBJECT) )
            Object.OnDebugRender();
        
        return -1;
    }
#endif // X_EDITOR

    //-------------------------------------------------------------------------

    virtual const char* QuickResourceName( void ) const
    {
        return "RigidGeom";
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourcePropertyName( void ) const 
    {
        return "RenderInst\\File";
    }

} s_Super_Destructible_Obj_Desc;

//=============================================================================

const object_desc&  super_destructible_obj::GetTypeDesc     ( void ) const
{
    return s_Super_Destructible_Obj_Desc;
}

//=============================================================================

const object_desc&  super_destructible_obj::GetObjectType   ( void )
{
    return s_Super_Destructible_Obj_Desc;
}

//==========================================================================

void BuildNamesList( const geom& Geom, namelist& List )
{
    for( s32 i=0; i<Geom.m_nMeshes; i++ )
    {
        x_strcpy( List.Name[i], Geom.GetMeshName( i ) );
    }
}

//==========================================================================
// FUNTIONS
//==========================================================================

super_destructible_obj::super_destructible_obj(void)
{
    m_Destroyed         = FALSE;
    m_DecalGroup        = 0;
    m_DestructionTime   = 0;
    m_CurrentStage      = 0;
    m_AcceptActorPain   = TRUE;
    m_AcceptObjectPain  = TRUE;
    m_IgnorePainGuid    = 0;
    m_AnimIndex         = -1;
    m_Invulnerable      = FALSE;
    m_KillCurrentStage  = FALSE;
}

//==========================================================================

super_destructible_obj::~super_destructible_obj(void)
{
    s32 i;
    for (i = 0 ; i < MAX_PAIN_RESPONSES ; i++ ) 
    {
        if (s_PainResponseInfo[i].Owner == GetGuid())
        {
            g_AudioMgr.Release( s_PainResponseInfo[i].AudioVoice, 0 );
            s_PainResponseInfo[i].Owner             = 0;
            s_PainResponseInfo[i].AudioVoice        = 0;
            s_PainResponseInfo[i].InitialScale      = 1.0f;

            if (SLOT_NULL != g_ObjMgr.GetSlotFromGuid( s_PainResponseInfo[ i ].Emitter ))
            {
                g_ObjMgr.DestroyObject( s_PainResponseInfo[ i ].Emitter );
            }

            s_PainResponseInfo[ i ].Emitter = 0;
        }
    }
}

//=========================================================================

void super_destructible_obj::UpdateZoneTrack ( void )
{ 
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, GetPosition() );
}

//==========================================================================

bbox super_destructible_obj::GetLocalBBox( void ) const
{
    // Anim data?
    if (m_hAnimGroup.GetPointer())
    {
        return m_hAnimGroup.GetPointer()->GetBBox();
    }
    else
    {
        return play_surface::GetLocalBBox();
    }
}   
   
//==========================================================================

void super_destructible_obj::OnPain( const pain& Pain )
{
    // no pain for destoryed or ourselfs
    if( m_Destroyed || (Pain.GetOriginGuid() == GetGuid()) || (Pain.GetOriginGuid() == m_IgnorePainGuid))
        return;

    // if invulnerable then don't take damage.
    if( m_Invulnerable )
        return;

    // Single Stage.. dont get the goods
    if( m_Stages.GetCount() <= 1 )
        return;
    
    // Compute force and pain
    health_handle HealthHandle(GetLogicalName());
    if( !Pain.ComputeDamageAndForce( HealthHandle, GetGuid(), GetBBox().GetCenter() ) )
        return;

    // Lookup info
    f32 Damage    = Pain.GetDamage();
    s32 LastStage = m_Stages.GetCount() - 1;
    
    // HACK : jhowa - When we have SuperD vs SuperD pain crazy things happen..
    //                Lets just make sure we don't kill a stage that is already dead.
    //                Will have to try and come back to this later.
    if( m_CurrentStage >= LastStage )
    {
        m_Destroyed = TRUE;
        return;
    }
        
    //ASSERT( m_CurrentStage < LastStage );
    
    // Apply all damage
    while( Damage > 0 )
    {
        // Lookup current stage
        stage& Stage = m_Stages[ m_CurrentStage ];
        
        // Decrease health and update damage
        f32 DamageTaken = x_min( Damage, Stage.m_Health );
        Stage.m_Health -= DamageTaken;
        Damage -= DamageTaken;

        // Has stage died?
        if( Stage.m_Health <= 0.0f )
        {
            // Are we going to be destroyed?
            s32 Stage = m_CurrentStage + 1;
            if( Stage >= m_Stages.GetCount() )
            {
                // Yes -> Mark ourself as Invulnerable so we avoid receiving any more pain
                m_Invulnerable = TRUE;        
            }

            // This stage has died .. Update and spawn necessary particles sounds etc. 
            KillStage(Pain);

            // Go to the next stage
            m_CurrentStage++;

            // If we are at the last stage, flag as destroyed and we are done
            if( m_CurrentStage == LastStage )
            {
                m_Destroyed = TRUE;                
            }

            // Broadcast pain from the stage we were on when we were hurt
            BroadcastPain( m_CurrentStage-1 );

            if (m_Destroyed)
                return;
        }
    }
    
    // Create pain response?
    if( Pain.HasCollision() )
    {
        radian3 Orient;
        Orient.Zero();
        vector3 Normal = Pain.GetCollision().Plane.Normal;
        Normal *= -1;
        Normal.GetPitchYaw( Orient.Pitch, Orient.Yaw );
        Orient.Roll = x_frand( 0,R_360 );

        // Scale down the old ones    
        const char* pPainSound = NULL;
        if (m_Stages[m_CurrentStage].m_PainSoundID != -1)
            pPainSound = g_StringMgr.GetString( m_Stages[m_CurrentStage].m_PainSoundID );

        object* pObject = g_ObjMgr.GetObjectByGuid( Pain.GetOriginGuid() );
        if( pObject )
        {
            // Filter out any actor originated pain
            if( pObject->IsKindOf( actor::GetRTTI() ) )
            {
                if( m_AcceptActorPain == FALSE)
                    return;
            }
            else
            {
                if( m_AcceptObjectPain == FALSE )
                    return;
            }

            CreateSuperPainResponse(    GetGuid(), 
                                        pPainSound,
                                        m_Stages[m_CurrentStage].m_hPainParticleEffect.GetName(),
                                        x_frand( m_Stages[m_CurrentStage].m_PainParticleScaleMin, m_Stages[m_CurrentStage].m_PainParticleScaleMax ),
                                        Pain.GetImpactPoint(),
                                        Orient,
                                        GetZone1() );
        }
    }
}    

//==========================================================================

void super_destructible_obj::KillStage( const pain& Pain )
{
    s32 i;    

    //
    // Zero health for this stage
    //
    m_Stages[ m_CurrentStage ].m_Health = 0.0f;

    guid Me = GetGuid();

    //
    // Oh the Pain
    //
    for (i=0;i<MAX_PAIN_RESPONSES;i++)
    {   
        if (s_PainResponseInfo[ i ].Owner == Me)
        {
            if (SLOT_NULL != g_ObjMgr.GetSlotFromGuid( s_PainResponseInfo[ i ].Emitter ))
            {
                g_ObjMgr.DestroyObject( s_PainResponseInfo[ i ].Emitter );
                s_PainResponseInfo[ i ].Emitter = 0;                    
            }
            if (g_AudioMgr.IsValidVoiceId( s_PainResponseInfo[ i ].AudioVoice ))
                g_AudioMgr.Release( s_PainResponseInfo[ i ].AudioVoice, 0 );
            s_PainResponseInfo[ i ].AudioVoice = 0;

            s_PainResponseInfo[ i ].Owner   = 0;                    
        }
    }

    //
    // Activate ?
    //
    if( m_Stages[m_CurrentStage].m_ActivateOnDestruction != NULL_GUID )
    {
        //activate this guid
        object* pObj = g_ObjMgr.GetObjectByGuid(m_Stages[m_CurrentStage].m_ActivateOnDestruction);
        if (pObj)
        {
            pObj->OnActivate(TRUE);
        }
    }

    //
    // Sound
    //
    if( m_Stages[m_CurrentStage].m_SoundID != -1 )
    {
        g_AudioMgr.PlayVolumeClipped( g_StringMgr.GetString( m_Stages[m_CurrentStage].m_SoundID ), GetPosition(), GetZone1(), TRUE );
    }

    //
    // Create the debris. ( Glass Only Tagged )
    //
    if( m_Stages[m_CurrentStage].GetGlassOnly() )
    {
        debris_mgr::GetDebrisMgr()->CreateGlassFromRigidGeom( (play_surface*)this, &Pain );
    }

    //
    // Create the debris.
    //
    const matrix4&  Mat = GetL2W();
    vector3         ColX;
    vector3         ColY;
    vector3         ColZ;

    Mat.GetColumns( ColX, ColY, ColZ );

    vector3 WorldStartPos = GetLocalBBox().GetCenter();
    vector3 LeftPos =   WorldStartPos + (vector3(-1, 0, 0) * m_Stages[m_CurrentStage].m_SpawnWidth);
    vector3 RightPos =  WorldStartPos + (vector3( 1, 0, 0) * m_Stages[m_CurrentStage].m_SpawnWidth);
    vector3 TopPos =    WorldStartPos + (vector3( 0, 1, 0) * m_Stages[m_CurrentStage].m_SpawnHeight);
    vector3 BottomPos = WorldStartPos + (vector3( 0,-1, 0) * m_Stages[m_CurrentStage].m_SpawnHeight);

    u32     CreateMask = 0;

    for( i = 0; i < (s32)m_Stages[m_CurrentStage].m_DebrisCount; i++ )
    {
        ColY.NormalizeAndScale( x_frand( m_Stages[m_CurrentStage].m_MinDebrisVelocity, m_Stages[m_CurrentStage].m_MaxDebrisVelocity) );

        f32 x = x_frand(-R_20,R_20);
        f32 y = x_frand(-R_20,R_20);
        f32 z = x_frand(-R_20,R_20);

        radian3 RandRot( x, y, z );

        u32 VMeshMask = 0;
        const geom* pGeom = m_Stages[m_CurrentStage].m_DebrisInst.GetGeom();
        if (pGeom && (pGeom->m_nVirtualMeshes > 1))
        {
            s32 nVMeshes = pGeom->m_nVirtualMeshes;
            s32 meshIndex = x_irand(0,nVMeshes-1);

            if( (m_Stages[m_CurrentStage].m_SingleCreateVMeshMask&(1<<meshIndex)) && !(CreateMask&(1<<meshIndex)) ||
                ((m_Stages[m_CurrentStage].m_SingleCreateVMeshMask&(1<<meshIndex)) == 0) )
            {
                //-- Mark that we have created this bit of debris.
                CreateMask |= (1<<meshIndex);
                VMeshMask  |= (1<<meshIndex);
            
                f32 Rx = x_frand( LeftPos.GetX() , RightPos .GetX() );
                f32 Ry = x_frand( TopPos .GetY() , BottomPos.GetY() );

                vector3 randPos = vector3( Rx, Ry, 0 );
                randPos.GetZ() = GetLocalBBox().GetCenter().GetZ();
                randPos = Mat.Transform( randPos );

                debris_mgr::GetDebrisMgr()->CreateDebris( randPos, GetZones(), ColY, RandRot, m_Stages[m_CurrentStage].m_DebrisInst, m_Stages[m_CurrentStage].m_DebrisLife, FALSE, VMeshMask );
            }
        }
    }

    //
    // Specific debris
    //
    {
        const geom* pGeom = m_Stages[m_CurrentStage].m_SpecificDebrisInst.GetGeom();
        if (pGeom)
        {
            s32 nVMeshes = pGeom->m_nVirtualMeshes;
            u32 VMeshMask = 0;

            radian3 Rot(0,0,0);

            s32 i;
            for (i=0;i<nVMeshes;i++)
            {
                VMeshMask = (1<<i);
            
                ColY.NormalizeAndScale( x_frand( m_Stages[m_CurrentStage].m_MinDebrisVelocity, m_Stages[m_CurrentStage].m_MaxDebrisVelocity) );
                               
                debris_mgr::GetDebrisMgr()->CreateDebris( Mat.GetTranslation(), GetZones(), ColY, Rot, m_Stages[m_CurrentStage].m_SpecificDebrisInst, m_Stages[m_CurrentStage].m_DebrisLife, TRUE, VMeshMask );
            }
        }
    }

    //
    // create the decal
    //
    decal_package*    pPackage = m_hDecalPackage.GetPointer();
    if ( pPackage &&
         (m_DecalGroup>=0) &&
         (m_DecalGroup<pPackage->GetNGroups()) &&
         (pPackage->GetNDecalDefs(m_DecalGroup)>0) )
    {
        decal_definition& Def = pPackage->GetDecalDef( m_DecalGroup, 0 );
        
        const bbox&    BBox     = GetBBox();
        const vector3& RayStart = BBox.GetCenter();
        const f32      Radius   = BBox.GetRadius();
        const vector3  RayEnd( RayStart.GetX(), -2.0f*(BBox.Max.GetY()-BBox.Min.GetY()), RayStart.GetZ() );
        g_DecalMgr.CreateDecalFromRayCast( Def, RayStart, RayEnd, vector2( Radius, Radius ), Def.RandomRoll() );
    }
    
    //
    // Create the particle effect.
    //
    guid gPE;
    {
        matrix4 Mat = GetL2W();
        radian3 Rot = Mat.GetRotation();
        vector3 Dir(0,0,1);
        Dir.Rotate(Rot);
        
        gPE = particle_emitter::CreatePresetParticleAndOrient( m_Stages[m_CurrentStage].m_hParticleEffect.GetName(), Dir, 
                                                        Mat.GetTranslation(), GetZone1() );
    }

    object* pObj = g_ObjMgr.GetObjectByGuid( gPE );
    if (pObj)
    {
        if (pObj->IsKindOf( particle_emitter::GetRTTI() ))
        {
            particle_emitter& PE = particle_emitter::GetSafeType( *pObj );

            PE.SetScale( m_Stages[m_CurrentStage].m_ParticleScale );
        }
    }

    //
    // Anim
    //
    if( m_hAnimGroup.GetPointer() && ( m_hAnimGroup.IsLoaded() == TRUE ) )
    {
        s32 Index = m_AnimPlayer.GetAnimIndex( g_StringMgr.GetString( m_Stages[m_CurrentStage].m_AnimIndex ) );
        if( Index != -1 )
            m_AnimPlayer.SetSimpleAnim( Index );
    }

    //
    // Update PolyCache
    //
    {
        g_PolyCache.InvalidateCells( GetBBox(), GetGuid() );
    }
}
  
//==========================================================================

void  super_destructible_obj::OnRender( void )
{
    if( m_Stages.GetCount() > 0 )
    {
        CONTEXT( "super_destructible_obj::OnRender" );

        rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    
        if( pRigidGeom && (m_hAnimGroup.IsLoaded() == TRUE) )
        {
            // Compute bones
            const matrix4* pBone = GetBoneL2Ws() ;
            if (!pBone)
                return ;

            u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;

            m_Inst.Render( pBone, Flags | GetRenderMode() );
        }
        else if( pRigidGeom && pRigidGeom->m_nBones <= 1 )
        {
            u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
        
            // Render using the LOD mask that we have setup.
            m_Inst.Render( &GetL2W(), Flags | GetRenderMode() );
        }
        else
        {
#ifdef X_EDITOR
            draw_BBox( GetBBox() );
#endif
        }
    }
    else
    {
#ifdef X_EDITOR        
        draw_BBox( GetBBox() );
#endif
    }
}

//=============================================================================

void super_destructible_obj::OnMove( const vector3& NewPos )
{
    bbox BBox = GetBBox() ;
 
    object::OnMove( NewPos );
    m_AnimPlayer.SetL2W( GetL2W() );
 
    BBox += GetBBox() ;

    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    if( pRigidGeom && pRigidGeom->m_Collision.nLowClusters > 0 )
        g_PolyCache.InvalidateCells( BBox, GetGuid() );

    // Make sure to track the object across zones
    UpdateZoneTrack();
}
 
//=============================================================================
 
void super_destructible_obj::OnTransform( const matrix4& L2W )
{
    bbox BBox = GetBBox() ;
 
    object::OnTransform( L2W );
    m_AnimPlayer.SetL2W( GetL2W() );
 
    BBox += GetBBox() ;
    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    if( pRigidGeom && pRigidGeom->m_Collision.nLowClusters > 0 )
        g_PolyCache.InvalidateCells( BBox, GetGuid() );

    // Make sure to track the object across zones
    UpdateZoneTrack();
}

//=============================================================================

const matrix4* super_destructible_obj::GetBoneL2Ws( void )
{
    return m_AnimPlayer.GetBoneL2Ws() ;
}

//=============================================================================

void super_destructible_obj::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "super_destructible_obj::OnAdvanceLogic" );

    // SB.
    // Since the LOD mask for super destructibles is stored in each stage,
    // we have to copy this into the regular render instance lod mask
    // since it is used by the collision system when building the polycache.
    // If it needs updating, then we need to re-build the polycache

    // It is possible for there to be no stages so we need to survive
    if( m_Stages.GetCount() )
    {
        ASSERTS( ((m_CurrentStage >= 0) && (m_CurrentStage < m_Stages.GetCount())), xfs("Super destructible '%s' at stage %d of %d", GetName(), m_CurrentStage, m_Stages.GetCount()) );
        u32 VMeshMask = m_Stages[m_CurrentStage].m_VMeshMask;
        if ( m_Inst.GetVMeshMask().VMeshMask != VMeshMask )
        {        
            m_Inst.SetVMeshMask( VMeshMask );
            g_PolyCache.InvalidateCells( GetBBox(), GetGuid() );
        }
    }

    //-- check for kill current stage.
    if( m_KillCurrentStage )
        KillCurrentStage ();

    // Update animation?
    if( m_hAnimGroup.IsLoaded() )
    {
        m_AnimPlayer.Advance( DeltaTime );
        g_EventMgr.HandleSuperEvents( m_AnimPlayer, this );

        //
        // Consider updating polycache
        //
        {
            if( (m_AnimPlayer.GetFrame() != m_AnimPlayer.GetPrevFrame()) ||
                (m_AnimPlayer.GetCycle() != m_AnimPlayer.GetPrevCycle()) )
            {
                rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
                if( pRigidGeom && pRigidGeom->m_Collision.nLowClusters > 0 )
                    g_PolyCache.InvalidateCells( GetBBox(), GetGuid() );
            }
        }
    }
}
    
//==========================================================================

void  super_destructible_obj::OnColCheck( void )
{
    CONTEXT("super_destructible_obj::OnColCheck");
    
    // Compute the bone matrices, then let the play_surface code handle it.

    if( m_Inst.GetRigidGeom() && (m_hAnimGroup.IsLoaded() == TRUE) )
    {
        // Compute bones
        const matrix4* pBone = GetBoneL2Ws() ;
        if (!pBone)
            return ;
        play_surface::DoColCheck( pBone );
    }
    else
    {
        play_surface::OnColCheck();
//        g_CollisionMgr.StartApply( GetGuid() );
//        g_CollisionMgr.ApplyAABBox( GetBBox() );    
//        g_CollisionMgr.EndApply();    
    }
}      

//==========================================================================

void  super_destructible_obj::OnColNotify( object& Object )
{
    play_surface::OnColNotify( Object );
} 

//==========================================================================

void super_destructible_obj::OnPolyCacheGather( void )
{
    // Use animation?
    if( ( m_Inst.GetRigidGeom() ) && ( m_hAnimGroup.IsLoaded() == TRUE ) )
    {
        // Compute bones
        const matrix4* pBoneL2Ws = GetBoneL2Ws();
        if( !pBoneL2Ws )
            return;
    
        // Gather info
        RigidGeom_GatherToPolyCache( GetGuid(), 
                                     GetBBox(), 
                                     m_Inst.GetLODMask( U16_MAX ),
                                     pBoneL2Ws,
                                     m_Inst.GetRigidGeom() );
    }
    else
    {    
        // No animation so just call base class
        play_surface::OnPolyCacheGather();
    }        
}

//==========================================================================
     
f32 super_destructible_obj::GetHealth( void )
{
    // Loop over all, but the last stage
    f32 TotalHealth = 0.0f;
    for( s32 i = 0; i < m_Stages.GetCount()-1; i++ )
        TotalHealth += m_Stages[ i ].m_Health;
    
    return TotalHealth;
}

//==========================================================================

f32 super_destructible_obj::GetMaxHealth( void )
{
    // Loop over all, but the last stage
    f32 TotalMaxHealth = 0.0f;
    for( s32 i = 0; i < m_Stages.GetCount()-1; i++ )
        TotalMaxHealth += m_Stages[ i ].m_MaxHealth;

    return TotalMaxHealth;
}

//==========================================================================

void super_destructible_obj::SetInitHealth( f32 Health )
{
    // Only apply if more than one stage
    s32 nStages = m_Stages.GetCount();
    if( nStages > 1 )
    {
        // Divide over all stages but the last (health is always zero)
        f32 HealthPerStage = Health / (f32)( nStages - 1 );
        for( s32 i = 0; i < ( nStages - 1 ); i++ )
        {
            stage& Stage = m_Stages[ i ];
            Stage.m_Health    = HealthPerStage;
            Stage.m_MaxHealth = HealthPerStage;
        }
        
        // Set to start stage
        m_CurrentStage = 0;
        m_Destroyed    = FALSE;
    }
}
     
//==========================================================================

#ifndef X_RETAIL
void  super_destructible_obj::OnDebugRender( void )
{
    if( m_Stages.GetCount() <= 0 )
        return;

    draw_ClearL2W();
    draw_Sphere( GetPosition(), m_Stages[m_CurrentStage].m_PainRadius, XCOLOR_RED );    
    draw_Marker( GetPosition(), XCOLOR_BLUE );
    draw_Point( GetPosition(), XCOLOR_BLUE );
    
    s32 i;
    for (i=0;i<MAX_PAIN_RESPONSES;i++)
    {
        if (s_PainResponseInfo[i].AudioVoice != 0)
        {
            draw_Point( s_PainResponseInfo[i].Pos );
            draw_Label( s_PainResponseInfo[i].Pos, XCOLOR_RED, "Voice %d",s_PainResponseInfo[i].AudioVoice );
        }        
    }

    vector3 WorldStartPos = GetLocalBBox().GetCenter();
    vector3 LeftPos =   WorldStartPos + (vector3(-1, 0, 0) * m_Stages[m_CurrentStage].m_SpawnWidth*0.5f);
    vector3 RightPos =  WorldStartPos + (vector3( 1, 0, 0) * m_Stages[m_CurrentStage].m_SpawnWidth*0.5f);
    vector3 TopPos =    WorldStartPos + (vector3( 0, 1, 0) * m_Stages[m_CurrentStage].m_SpawnHeight*0.5f);
    vector3 BottomPos = WorldStartPos + (vector3( 0,-1, 0) * m_Stages[m_CurrentStage].m_SpawnHeight*0.5f);

    bbox BBOX;
    BBOX.Set( vector3( LeftPos.GetX(), TopPos.GetY(), WorldStartPos.GetZ() ), vector3( RightPos.GetX(), BottomPos.GetY(), WorldStartPos.GetZ()+2.0f ) );

    draw_SetL2W( GetL2W() );
    draw_BBox( BBOX, XCOLOR_GREEN );
} 
#endif // X_RETAIL

//==========================================================================

anim_group::handle* super_destructible_obj::GetAnimGroupHandlePtr( void )
{
    // Only provide handle if group is present since super destructibles do not need anims
    if( m_hAnimGroup.GetPointer() )
        return &m_hAnimGroup;
    else
        return NULL;
}

//==========================================================================

void super_destructible_obj::CreateSuperPainResponse( guid          OwnerGuid,
                                                      const char*        pSoundName, 
                                                      const char*        pFXName,
                                                      f32                Scale,
                                                      const vector3&     Pos,
                                                      const radian3&     Rot,
                                                      u16                Zone )
{


    //
    //  Scale down all old responses
    //
    s32 i;
    for (i=0;i<NUM_FADING_RESPONSES;i++)
    {
        s32     iCur = (s_iNextPainResponse + i) % MAX_PAIN_RESPONSES;
        f32     T    = (f32)i / (f32)NUM_FADING_RESPONSES;

        object* pObj = g_ObjMgr.GetObjectByGuid( s_PainResponseInfo[ iCur ].Emitter );
        if (pObj)
        {
            if (pObj->IsKindOf( particle_emitter::GetRTTI() ))
            {
                particle_emitter& PE = particle_emitter::GetSafeType( *pObj );

                f32 NewScale = MAX(0.1f, s_PainResponseInfo[ iCur ].InitialScale * T );
                PE.SetScale( NewScale );
            }
        }
    }

    //
    //  Count how many active responses are owned by the previous owner.
    //
    //  Also, release any inactive voices belonging to OwnerGuid, and find 
    //  an active voice for the current OwnerGuid if there is one
    //
    s32         nOwned = 0;
    voice_id    iVoiceForThisOwner = 0;
    guid        PreviousOwner = s_PainResponseInfo[ s_iNextPainResponse ].Owner;

    for (i=0;i<MAX_PAIN_RESPONSES;i++)
    {
        if (PreviousOwner != 0)
        {
            if ( s_PainResponseInfo[ i ].Owner == PreviousOwner )
            {
                nOwned++;
            }
        }

        if ( s_PainResponseInfo[ i ].Owner == OwnerGuid )
        {
            // Relase the audio voice if it has stopped playing
            voice_id ThisVoice = s_PainResponseInfo[ i ].AudioVoice;

            if (ThisVoice != 0)
            {
                if (!g_AudioMgr.IsValidVoiceId( ThisVoice ))
                {
                    g_AudioMgr.Release( ThisVoice, 0 );
                    s_PainResponseInfo[ i ].AudioVoice = 0;
                }
            }

            ThisVoice = s_PainResponseInfo[ i ].AudioVoice;
           
            if (ThisVoice != 0)
            {
                iVoiceForThisOwner = ThisVoice;
            }
        }
    }
    
    //
    //  If the previous owner is about to lose his only audio event,
    //  make sure we kill it off.
    //
    if (nOwned == 1)
    {
        g_AudioMgr.Release( s_PainResponseInfo[ s_iNextPainResponse ].AudioVoice, 0.0f );
        s_PainResponseInfo[ s_iNextPainResponse ].AudioVoice = 0;
    }

    //
    //  Kill current one
    //
    if (s_PainResponseInfo[ s_iNextPainResponse ].Emitter != 0)
    {
        if (SLOT_NULL != g_ObjMgr.GetSlotFromGuid( s_PainResponseInfo[ s_iNextPainResponse ].Emitter ))
        {
            g_ObjMgr.DestroyObject( s_PainResponseInfo[ s_iNextPainResponse ].Emitter );
        }
        s_PainResponseInfo[ s_iNextPainResponse ].Emitter = 0;
    }

    //
    //  Create particle
    //
    s_PainResponseInfo[ s_iNextPainResponse ].Emitter = particle_emitter::CreatePresetParticleAndOrient( pFXName, 
                                                                                                         Rot, 
                                                                                                         Pos,
                                                                                                         Zone );
    s_PainResponseInfo[ s_iNextPainResponse ].Owner = OwnerGuid;
    s_PainResponseInfo[ s_iNextPainResponse ].InitialScale = Scale;

    object* pObj = g_ObjMgr.GetObjectByGuid( s_PainResponseInfo[ s_iNextPainResponse ].Emitter );
    if (pObj)
    {
        if (pObj->IsKindOf( particle_emitter::GetRTTI() ))
        {
            particle_emitter& PE = particle_emitter::GetSafeType( *pObj );

            PE.SetScale( s_PainResponseInfo[ s_iNextPainResponse ].InitialScale );
        }
    }

    //
    //  Create sound if required
    //
    if (iVoiceForThisOwner == 0)
    {
        if( NULL != pSoundName )
        {
            iVoiceForThisOwner = g_AudioMgr.PlayVolumeClipped( pSoundName, Pos, GetZone1(), TRUE );
            if ( iVoiceForThisOwner != 0 )
            {
                g_AudioMgr.SetFalloff( iVoiceForThisOwner, 0.5f, 0.5f );
            }
        }
    }

    s_PainResponseInfo[ s_iNextPainResponse ].AudioVoice = iVoiceForThisOwner;
    s_PainResponseInfo[ s_iNextPainResponse ].Pos = Pos;

    s_iNextPainResponse = (s_iNextPainResponse+1) % MAX_PAIN_RESPONSES;
}

//==========================================================================

void super_destructible_obj::BroadcastPain( s32 iUsePainFromThisStage )
{
    ASSERT( iUsePainFromThisStage >= 0);
    ASSERT( iUsePainFromThisStage < m_Stages.GetCount() );

    // If no pain. then done broadcast.
    if( !x_strcmp( m_Stages[iUsePainFromThisStage].m_PainType , "NONE") )
        return;

    //create pain area
    bbox Pain( GetPosition(), m_Stages[iUsePainFromThisStage].m_PainRadius );
    g_ObjMgr.SelectBBox( ATTR_DAMAGEABLE , Pain ,object::TYPE_ALL_TYPES );    
    xarray<slot_id> ObjectSlots;
    ObjectSlots.SetGrowAmount( 10 );

    vector3 ColX;
    vector3 ColY;
    vector3 ColZ;

    GetL2W().GetColumns( ColX, ColY, ColZ );

    slot_id SlotID = g_ObjMgr.StartLoop();
    while( SlotID != SLOT_NULL )
    {
        ObjectSlots.Append() = SlotID;
        SlotID = g_ObjMgr.GetNextResult( SlotID );
    }
    g_ObjMgr.EndLoop();
    
    for( s32 i = 0; i < ObjectSlots.GetCount(); i++ )
    {
        slot_id& SlotID = ObjectSlots[i];
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        if( pObject->GetAttrBits() & ATTR_DAMAGEABLE )
        {
            pain Pain;
            Pain.Setup( m_Stages[iUsePainFromThisStage].m_PainType, GetGuid(),GetPosition());
            Pain.SetDirectHitGuid( pObject->GetGuid() );
            Pain.ApplyToObject( pObject );
        }
    }
}      

//==========================================================================

void super_destructible_obj::OnEnumProp( prop_enum& List )
{
    s32 i;

    // Call base class
    play_surface::OnEnumProp( List );

    // Audio
    List.PropEnumHeader  ("Super Destructible",                          "Properties for the Destructible object", 0 );
    List.PropEnumExternal("Super Destructible\\Audio Package",           "Resource\0audiopkg\0", "The audio package associated with this Glass object.", 0 );

    // Anim
    List.PropEnumExternal("Super Destructible\\Anim Package",            "Resource\0anim\0",     "Resource File", PROP_TYPE_MUST_ENUM );
    //List.AddHeader  ( "Super Destructible\\Anim",                                                       "Stage Anim.");
    if( m_hAnimGroup.GetPointer() )
    {
        anim_group* pAnimGroup = m_hAnimGroup.GetPointer();

        g_SuperDestructibleStringList.Clear();

        s32 i;
        for( i=0; i<pAnimGroup->GetNAnims(); i++ )
        {
            const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( i );
            g_SuperDestructibleStringList += AnimInfo.GetName();
            g_SuperDestructibleStringList += "~";
        }

        for( i=0; g_SuperDestructibleStringList[i]; i++ )
        {
            if( g_SuperDestructibleStringList[i] == '~' )
                g_SuperDestructibleStringList[i] = 0;
        }
        List.PropEnumEnum( "Super Destructible\\PlayAnim", g_SuperDestructibleStringList, "Only for triggers. Select an animation what you want the object to play", PROP_TYPE_EXPOSE | PROP_TYPE_MUST_ENUM);
    }
    else
    {
        List.PropEnumEnum( "Super Destructible\\PlayAnim", "\0\0", "Only for triggers. Select an animation what you want the object to play", PROP_TYPE_EXPOSE );
    }

    // Decal
    List.PropEnumExternal("Super Destructible\\Char Decal Package",      "Resource\0decalpkg\0", "The package of the char decal this object will leave behind.", 0 );
    List.PropEnumInt     ("Super Destructible\\Char Decal Group",        "The group index of the char decal this object will leave behind.", 0 );
    
    // Pain
    List.PropEnumBool    ("Super Destructible\\AcceptActorPain",         "Accept pain response events from Actor", 0 );
    List.PropEnumBool    ("Super Destructible\\AcceptObjectPain",        "Accept pain response events from objects", 0 );
    List.PropEnumGuid    ("Super Destructible\\IgnorePainGuid",          "Ignore Pain from guid", 0 );

    // Stage
    List.PropEnumInt     ("Super Destructible\\StageCount",              "Number of Stages in this destructible.",PROP_TYPE_MUST_ENUM);
    List.PropEnumInt     ("Super Destructible\\CurrentStage",            "Current stage the destructible is in.", PROP_TYPE_DONT_SHOW | PROP_TYPE_EXPOSE );
    List.PropEnumBool    ("Super Destructible\\Destroyed",               "Are we dead yet", PROP_TYPE_DONT_SHOW | PROP_TYPE_EXPOSE );
    List.PropEnumBool    ("Super Destructible\\Invulnerable",            "destructible will not game damage if set to true", PROP_TYPE_DONT_SHOW | PROP_TYPE_EXPOSE );
    List.PropEnumBool    ("Super Destructible\\KillCurrentStage",        "Kills current stage",PROP_TYPE_DONT_SHOW|PROP_TYPE_EXPOSE );

    // Add the individual stage properties
    for(i = 0 ; i < m_Stages.GetCount() ; i++ )
    {
        // Add common properties
        List.PropEnumHeader(xfs("Super Destructible\\Stage[%d]", i), "Stage", PROP_TYPE_EXPOSE );
        s32 iHeader = List.PushPath(xfs("Super Destructible\\Stage[%d]\\", i)) ;

        // Enum Stage
        m_Stages[i].OnEnumProp( List, *this, i) ;

        List.PopPath(iHeader) ;
    }
}      

//==========================================================================

void super_destructible_obj::KillCurrentStage (void)
{
    if( m_Destroyed )
        return;

    pain P;
    P.Setup("Generic_1", GetGuid (), GetPosition () );
    P.ComputeDamageAndForce( "Super_Destructible_Object", GetGuid(), GetPosition() );

    // Are we going to be destroyed?
    s32 Stage = m_CurrentStage + 1;
    if( Stage >= m_Stages.GetCount() )
    {
        // Yes -> Mark ourself as Invulnerable so we avoid receiving any more pain
        m_Invulnerable = TRUE;        
    }
    
    KillStage(P);    
    
    m_CurrentStage++;
    
    if( m_CurrentStage >= m_Stages.GetCount() )
    {
        m_CurrentStage--;
        m_Destroyed = TRUE;        
    }

    // Broadcast pain from the stage we were on when we were hurt
    BroadcastPain( m_CurrentStage-1 );

    if (m_Destroyed)
        return;

    m_KillCurrentStage = FALSE;
}

//==========================================================================

void super_destructible_obj::SetCurrentStage( s32 NewCurrentStage, xbool bGiveFullHealth )
{
    // Do some bounds checking on it.
    s32 LastStage = m_Stages.GetCount() - 1;
    NewCurrentStage = x_min( NewCurrentStage, LastStage );
    NewCurrentStage = x_max( NewCurrentStage, 0 );

    // Set stage
    m_CurrentStage = NewCurrentStage;

    // Update destroyed flag
    m_Destroyed = ( m_CurrentStage == LastStage );

    // Loop thru the stages now and reset the hit points
    if( bGiveFullHealth )
    {
        for( s32 i = LastStage; i >= m_CurrentStage; i-- )
        {
            m_Stages[ i ].m_Health = m_Stages[ i ].m_MaxHealth;
        }
    }
}

//==========================================================================

xbool super_destructible_obj::OnProperty( prop_query& I )
{
    //
    // Call base class
    //
    if( play_surface::OnProperty( I ) )
    {
        // Initialize the zone tracker
        if( I.IsVar( "Base\\Position" )) 
        {
            g_ZoneMgr.InitZoneTracking( *this, m_ZoneTracker );
        }

        return( TRUE );
    }

    //
    // External Audio.
    //
    if( I.IsVar( "Super Destructible\\Audio Package" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                if( xstring(pString) == "<null>" )
                {
                    m_hAudioPackage.SetName( "" );
                }
                else
                {
                    m_hAudioPackage.SetName( pString );                

                    // Load the audio package.
                    if( m_hAudioPackage.IsLoaded() == FALSE )
                        m_hAudioPackage.GetPointer();
                }
            }
        }
        return( TRUE );
    }

    //
    // Decal
    //
    if ( I.IsVar( "Super Destructible\\Char Decal Package" ) )
    {
        if ( I.IsRead() )
        {
            I.SetVarExternal( m_hDecalPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            m_hDecalPackage.SetName( I.GetVarExternal() );
            m_hDecalPackage.GetPointer();
        }

        return TRUE;
    }

    if ( I.VarInt( "Super Destructible\\Char Decal Group", m_DecalGroup ) )
    {
        return TRUE;
    }


    //
    // Pain
    //
    xbool bAcceptActorPain = m_AcceptActorPain;
    if ( I.VarBool( "Super Destructible\\AcceptActorPain", bAcceptActorPain ) )
    {
        m_AcceptActorPain = bAcceptActorPain;
        return TRUE;
    }

    xbool bAcceptObjectPain = m_AcceptObjectPain;
    if ( I.VarBool( "Super Destructible\\AcceptObjectPain", bAcceptObjectPain ) )
    {
        m_AcceptObjectPain = bAcceptObjectPain;
        return TRUE;
    }

    if ( I.VarGUID ("Super Destructible\\IgnorePainGuid", m_IgnorePainGuid ) )
        return TRUE;

    //
    // Anim      
    //
    if( I.IsVar( "Super Destructible\\Anim Package" ) )
    {
        MEMORY_OWNER("Anim Package");
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hAnimGroup.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();
            if( ( pString[0] ) && ( x_strcmp( pString, "<null>" ) != 0 ) )
            {
                m_hAnimGroup.SetName( pString );
                anim_group* pAnimGroup = m_hAnimGroup.GetPointer();
                
                if( pAnimGroup )
                {
                    m_AnimPlayer.SetAnimGroup( m_hAnimGroup );
                    m_AnimPlayer.SetAnim( 0, TRUE );
                }
            }
            else
            {
                // Clear anim group and anim
                m_hAnimGroup.SetName( "" );
                m_AnimIndex = -1;
            }
        }
        return( TRUE );
    }

    //
    // Stage count?
    //
    if( I.IsVar("Super Destructible\\StageCount" ) )
    {
        MEMORY_OWNER( "StageCount" );
        if( I.IsRead() )
            I.SetVarInt( m_Stages.GetCount() ) ;
        else
        {
            // Get new count
            s32 Count = I.GetVarInt() ;
            
            // Add new keys on the end
            while(Count > m_Stages.GetCount())
                CreateStage(m_Stages.GetCount()-1, TRUE) ;

            // Set count incase of deleting keys
            m_Stages.SetCount(Count) ;
        }

        return TRUE ;
    }
    
    // This property is hidden so it can only be set from a trigger
    if( I.IsVar ("Super Destructible\\CurrentStage"))
    {
        if( I.IsRead () )
            I.SetVarInt( m_CurrentStage );
        else
        {
            SetCurrentStage( I.GetVarInt(), TRUE );
        }
        return TRUE;
    }

    // Invulnerable
    xbool bInvulnerable = m_Invulnerable;
    if( I.VarBool( "Super Destructible\\Invulnerable", bInvulnerable ) )
    {
        m_Invulnerable = bInvulnerable;
        return TRUE;
    }

    // Kill current stage
    xbool bKillCurrentStage = m_KillCurrentStage;
    if( I.VarBool("Super Destructible\\KillCurrentStage", bKillCurrentStage ) ) 
    {
        m_KillCurrentStage = bKillCurrentStage;
        return TRUE;
    }

    //-- Is this Super D Destroyed
    if( I.IsVar( "Super Destructible\\Destroyed") )
    {
        if( I.IsRead () )
            I.SetVarBool( m_Destroyed );
        else
            m_Destroyed = I.GetVarBool();
        return TRUE;
    }

    // Play Anim
    if( I.IsVar( "Super Destructible\\PlayAnim" ) )
    {
        MEMORY_OWNER( "PlayAnim" );
        if( I.IsRead() )
        {
            if ( m_AnimIndex > -1 )
            {
                const char* pString = g_StringMgr.GetString( m_AnimIndex );

                if ( pString )
                {
                    I.SetVarEnum( pString );                
                }
            }
        }
        else
        {
            if( m_hAnimGroup.GetPointer() )
            {

                s32 Index = m_AnimPlayer.GetAnimIndex( I.GetVarEnum() );
                if( Index != -1 )
                {
                    m_AnimIndex = g_StringMgr.Add( I.GetVarEnum() );
                    //m_AnimPlayer.SetSimpleAnim( Index );
                }
                else
                {
                    LOG_ERROR( "GAMEPLAY", "No animation found with name (%s) in anim group (%s)",
                        I.GetVarEnum(), m_hAnimGroup.GetName() );                        
                }
            }
            else
            {
                m_AnimIndex = g_StringMgr.Add( I.GetVarEnum() );
            }
        }

        return TRUE;
    }

    //
    // Stage[
    //
    if (I.IsSimilarPath("Super Destructible\\Stage["))
    {
        // Lookup stage
        s32  iStage = I.GetIndex(0) ;

        xbool bFound = FALSE;

        if( (iStage>=0) && (iStage < m_Stages.GetCount()) )
        {
            stage& Stage  = m_Stages[iStage] ;

            s32 iHeader = I.PushPath("Super Destructible\\Stage[]\\") ; 

            // Read/Write stage?
            bFound = Stage.OnProperty(I, *this, iStage) ;

            I.PopPath(iHeader) ;
        }

        if (bFound)
            return TRUE ;
    }

    return FALSE;
}   

//=========================================================================

void super_destructible_obj::CreateStage( s32 iRefStage /*= -1*/, xbool bAfterRefStage /*= TRUE*/ )
{
    // Create new key
    m_Stages.Insert(iRefStage+bAfterRefStage);
}

//=========================================================================

void super_destructible_obj::DeleteStage( s32 Index )
{
    // Delete the key
    if( m_Stages.GetCount() > 1 )
        m_Stages.Delete(Index);
}

//=========================================================================

void super_destructible_obj::Regenerate( f32 DeltaTime, f32 TotalTime )
{
    s32 i;
    
    // Skip if no stages
    if( m_Stages.GetCount() == 0 )
        return;
        
    // Put to previous stage if destroyed
    if( m_Destroyed )
        SetCurrentStage( m_CurrentStage - 1, FALSE );
    
    // Compute total health of all stages (skip last stage since health is assumed to be 0)
    f32 TotalHealth = 0.0f;
    for( i = 0; i < m_Stages.GetCount() - 1; i++ )
        TotalHealth += m_Stages[ i ].m_MaxHealth;
    
    // Compute health increase
    f32 DeltaHealth = TotalHealth * DeltaTime / TotalTime;
    
    // Increase health of current stage
    stage& Stage = m_Stages[ m_CurrentStage ];
    Stage.m_Health += DeltaHealth;
    
    // Go to previous stage?
    if( Stage.m_Health > Stage.m_MaxHealth )
    {
        // Cap health
        Stage.m_Health = Stage.m_MaxHealth;
        
        // Goto previous stage
        if( m_CurrentStage > 0 )
            SetCurrentStage( m_CurrentStage - 1, FALSE );
    }
}

//=========================================================================

#ifdef X_EDITOR
s32 super_destructible_obj::OnValidateProperties( xstring& ErrorMsg )
{
    s32 errors = 0;
    errors = object::OnValidateProperties( ErrorMsg );

    if( m_Stages.GetCount() == 0 )
    {
        ErrorMsg += "\nSuper Destructible has 0 stages\n";
        errors += 1;
    }

    return errors;
}
#endif

//==========================================================================
// STAGE 
//==========================================================================

super_destructible_obj::stage::stage( )
{
    // Debris
    m_DebrisCount           = 10;
    m_MinDebrisVelocity     = 100;
    m_MaxDebrisVelocity     = 100;
    m_DebrisLife            = 3;
    m_SpawnWidth            = 100;
    m_SpawnHeight           = 100;

    // Particles
    m_ParticleScale         = 1;
    m_PainParticleScaleMin  = 0.75f;
    m_PainParticleScaleMax  = 1.25f;

    // Pain
    m_PainRadius            = 100.0f;
    m_PainAmount            = 0;
    x_strcpy( m_PainType, "NONE");
    m_PainForce             = 0;
    m_PainSoundID           = -1;
    m_SoundID               = -1;
    
    // Settings 
    m_Health                = 0.0f;
    m_MaxHealth             = 0.0f;
    m_ActivateOnDestruction = NULL_GUID;
    m_GlassOnly             = FALSE;
    m_AnimIndex = g_StringMgr.Add( "None" );


}

//==========================================================================

void super_destructible_obj::stage::OnEnumProp  ( prop_enum& List, 
                                                  super_destructible_obj& SuperDestructibleObj, 
                                                  s32 iStage )
{
    xbool LastStage = (( iStage == SuperDestructibleObj.GetNumStages()-1 ) ? TRUE : FALSE);
    u32   Flags = (LastStage) ? PROP_TYPE_DONT_SHOW : PROP_TYPE_EXPOSE;

    u32 CreateBtnFlags = PROP_TYPE_MUST_ENUM;
    if ((iStage == -1) && (SuperDestructibleObj.GetNumStages() > 1))
        CreateBtnFlags |= PROP_TYPE_READ_ONLY;

    s32 ID;

    //
    // Settings
    //
    if( !LastStage )
    {
        List.PropEnumHeader  ( "Settings" ,                              "Stage Settings", 0 );
        List.PropEnumFloat   ( "Settings\\Hit Points",                   "Number of hit points for this stage",PROP_TYPE_EXPOSE | Flags );
        List.PropEnumBool    ( "Settings\\Glass Only",                   "Break glass portion only.", Flags );
        List.PropEnumExternal( "Settings\\Stage Destruction Sound",      "Sound\0soundexternal\0","The sound to play when this stage is destroyed", PROP_TYPE_MUST_ENUM | Flags );
        List.PropEnumGuid    ( "Settings\\Activate OnDestroy",           "Activate this guid on destruction of this object.", Flags );
    }

    //
    // LoD Meshs
    //
    geom* pGeom = SuperDestructibleObj.GetRigidInst().GetGeom();
    m_VMeshMask.OnEnumProp( List, pGeom );

    //
    // Pain
    //
    if( !LastStage )
    {
        List.PropEnumHeader  ( "Pain",                                   "Pain Event", 0 );
        List.PropEnumFloat   ( "Pain\\Pain Radius",                      "The radius of the pain sphere." , Flags);
        List.PropEnumString  ( "Pain\\Pain Type",                        "The Pain Type as of the tweek table Set to NONE for no pain" , Flags);
        List.PropEnumFloat   ( "Pain\\Pain Force",                       "The amount of force the pain event has." , Flags);
    }
    
    //
    // Debris
    //
    if( !LastStage )
    {
        List.PropEnumHeader  ( "Debris",                                 "This is the debris that is created.",PROP_TYPE_MUST_ENUM );
        ID = List.PushPath( "Debris\\" );
        m_DebrisInst.OnEnumProp( List );
        List.PopPath( ID );

        List.PropEnumHeader( "SingleCreate",                        "This is debris that only gets created once.", PROP_TYPE_MUST_ENUM );
        ID = List.PushPath( "SingleCreate\\" );
        pGeom = m_DebrisInst.GetGeom();
        m_SingleCreateVMeshMask.OnEnumProp( List, pGeom );
        List.PopPath( ID );

        List.PropEnumFloat   ( "Debris\\Debris Count",                                       "How many times should we spawn the debris", 0 );
        List.PropEnumFloat   ( "Debris\\Debris Min Vertical Velocity",                       "Slowest velocity to shoot the deris out with on the objects World Y axis.", 0 );
        List.PropEnumFloat   ( "Debris\\Debris Max Vertical Velocity",                       "Fastest velocity to shoot the deris out with on the objects World Y axis.", 0 );
        List.PropEnumFloat   ( "Debris\\Debris Life",                                        "How long is the debris going to live (seconds)", 0 );
        List.PropEnumFloat   ( "Debris\\Debris Width",                                       "Width of spawn point", 0 );
        List.PropEnumFloat   ( "Debris\\Debris Height",                                      "Height of spawn point", 0 );
    }

    //
    // Specific Debris
    //
    if( !LastStage )
    {
        List.PropEnumHeader  ( "Specific Debris",                                            "This debris is specific to the object being destroyed.", 0 );
        ID = List.PushPath( "Specific Debris\\" );
        m_SpecificDebrisInst.OnEnumProp( List );
        List.PopPath( ID );
    }

    //
    // Particles
    //
    if( !LastStage )
    {
        List.PropEnumHeader  ( "Particles",                                                  "This is the debris that is created.", 0 );
        List.PropEnumExternal( "Particles\\Particles Resource",    "Resource\0fxo\0",        "Particle Resource for this item", PROP_TYPE_MUST_ENUM );
        List.PropEnumFloat   ( "Particles\\Scale",                                           "The scale of the particle effect for this object.", 0 );
    }
    //
    // Pain Response
    //
    if( !LastStage )
    {
        List.PropEnumHeader  ( "Pain Response",                                              "How the object responds to pain.", 0 );
        List.PropEnumExternal( "Pain Response\\Particles",         "Resource\0fxo\0",        "Particle Resource for when this object receives pain", PROP_TYPE_MUST_ENUM );
        List.PropEnumFloat   ( "Pain Response\\Min Particle Scale",                          "The minimum scale of the pain particle effect for this object.", 0 );
        List.PropEnumFloat   ( "Pain Response\\Max Particle Scale",                          "The maximum scale of the pain particle effect for this object.", 0 );
        List.PropEnumExternal( "Pain Response\\Sound",             "Sound\0soundexternal\0", "The sound to play when this object receives pain", PROP_TYPE_MUST_ENUM );
    }
    //
    // Anim
    //
    if( !LastStage )
    {
        List.PropEnumHeader  ( "Anim",                                                       "Stage Anim.", 0 );
        if( SuperDestructibleObj.m_hAnimGroup.GetPointer() )
        {
            anim_group* pAnimGroup = SuperDestructibleObj.m_hAnimGroup.GetPointer();

            g_SuperDestructiblePlayAnimStringList.Clear();

            s32 i;
            for( i=0; i<pAnimGroup->GetNAnims(); i++ )
            {
                const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( i );

                g_SuperDestructiblePlayAnimStringList += AnimInfo.GetName();
                g_SuperDestructiblePlayAnimStringList += "~";
            }

            for( i=0; g_SuperDestructiblePlayAnimStringList[i]; i++ )
            {
                if( g_SuperDestructiblePlayAnimStringList[i] == '~' )
                    g_SuperDestructiblePlayAnimStringList[i] = 0;
            }

            List.PropEnumEnum( "Anim\\PlayAnim", g_SuperDestructiblePlayAnimStringList, "Only for triggers. Select an animation what you want the object to play", PROP_TYPE_EXPOSE | PROP_TYPE_MUST_ENUM);
        }
        else
        {
            List.PropEnumEnum( "Anim\\PlayAnim", "\0\0", "Only for triggers. Select an animation what you want the object to play", PROP_TYPE_EXPOSE );
        }
    }

    //
    // Buttons ( add / remove )
    //
    List.PropEnumButton ("Create Stage Before",  "Inserts a new Stage before this one."  , CreateBtnFlags);
    List.PropEnumButton ("Create Stage After",   "Inserts a new Stage after  this one."  , CreateBtnFlags);
    List.PropEnumButton ("Delete Stage",         "Deletes the selected Stage"            , PROP_TYPE_MUST_ENUM);
}


//==========================================================================

xbool super_destructible_obj::stage::OnProperty  ( prop_query& I, 
                                                   super_destructible_obj& SuperDestructibleObj, 
                                                   s32 iStage )
{
    geom* pGeom = SuperDestructibleObj.GetRigidInst().GetGeom();

    //
    // LoD Meshs
    //
    if( m_VMeshMask.OnProperty( I, pGeom ) )
    {
        return TRUE;
    }

    // 
    // Pain
    //
    if( I.VarFloat( "Pain\\Pain Radius", m_PainRadius ) )
        return TRUE;
    
    if( I.VarString( "Pain\\Pain Type", m_PainType,64 ) )
        return TRUE;

    if( I.VarFloat( "Pain\\Pain Force", m_PainForce ) ) 
        return TRUE;

    //
    // Settings
    //
    if( I.IsVar("Settings\\Hit Points" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_Health );
        }
        else
        {
            m_Health = I.GetVarFloat();
            m_MaxHealth = m_Health;
        }
        return TRUE;
    }

    if( I.VarBool( "Settings\\Glass Only", m_GlassOnly ) )
        return TRUE;

    if( SMP_UTIL_IsAudioVar( I, "Settings\\Stage Destruction Sound", SuperDestructibleObj.m_hAudioPackage, m_SoundID ) )
    {
        if( (m_SoundID == -1) && (m_PainSoundID == -1) )
            SuperDestructibleObj.m_hAudioPackage.SetName( "" );

        return( TRUE );
    }

    if (I.VarGUID ( "Settings\\Activate OnDestroy",m_ActivateOnDestruction))
        return TRUE;
    
    //
    // Debris
    //
    s32 ID = I.PushPath( "Debris\\" );
    if( m_DebrisInst.OnProperty( I ) )
    {
        I.PopPath( ID );
        return TRUE;
    }
    I.PopPath( ID );

    //
    // Debris that only gets created once
    //
    ID = I.PushPath( "SingleCreate\\" );
    if( m_SingleCreateVMeshMask.OnProperty( I, pGeom ) )
    {
        I.PopPath( ID );
        return TRUE;
    }
    I.PopPath( ID );
   
    if( I.VarFloat( "Debris\\Debris Count", m_DebrisCount ) )
    {
        // Keep the value as a whole number.
        m_DebrisCount = (f32)((s32)m_DebrisCount);

        return TRUE;
    }

    if( I.VarFloat( "Debris\\Debris Min Vertical Velocity", m_MinDebrisVelocity ) )
        return TRUE;
    if( I.VarFloat( "Debris\\Debris Max Vertical Velocity", m_MaxDebrisVelocity ) )
        return TRUE;
    if( I.VarFloat( "Debris\\Debris Life", m_DebrisLife ) )
        return TRUE;
    if( I.VarFloat( "Debris\\Debris Width", m_SpawnWidth ) )
        return TRUE;
    if( I.VarFloat( "Debris\\Debris Height", m_SpawnHeight ) )
        return TRUE;

    //
    // Specific Debris
    //
    ID = I.PushPath( "Destructible Obj\\Specific Debris\\" );
    if( m_SpecificDebrisInst.OnProperty( I ) )
    {
        I.PopPath( ID );
        return TRUE;
    }
    I.PopPath( ID );

    //
    // Particle.
    //
    if( I.IsVar( "Particles\\Particles Resource" ) )
    {
        if( I.IsRead() )
        {
            if( x_strcmp(m_hParticleEffect.GetName(),"<null>") != 0)
                I.SetVarExternal( m_hParticleEffect.GetName(), RESOURCE_NAME_SIZE );
            else
            {
                m_hParticleEffect.SetName("");
            }
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                m_hParticleEffect.SetName( pString );                

                // Load the particle effect.
                if( m_hParticleEffect.IsLoaded() == FALSE )
                    m_hParticleEffect.GetPointer();
            }
        }
        return( TRUE );
    }

    if( I.VarFloat( "Particles\\Scale", m_ParticleScale ) )
        return TRUE;

    //
    // Pain Response
    //
    if( I.IsVar( "Pain Response\\Particles" ) )
    {
        if( I.IsRead() )
        {
            if( x_strcmp(m_hPainParticleEffect.GetName(),"<null>") != 0)
            {
                I.SetVarExternal( m_hPainParticleEffect.GetName(), RESOURCE_NAME_SIZE );
            }
            else
            {
                m_hPainParticleEffect.SetName("");
            }
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                m_hPainParticleEffect.SetName( pString );                

                // Load the particle effect.
                if( m_hPainParticleEffect.IsLoaded() == FALSE )
                    m_hPainParticleEffect.GetPointer();
            }
        }
        return( TRUE );
    }

    if( I.VarFloat( "Pain Response\\Min Particle Scale", m_PainParticleScaleMin ) )
        return TRUE;
    if( I.VarFloat( "Pain Response\\Max Particle Scale", m_PainParticleScaleMax ) )
        return TRUE;

    if( SMP_UTIL_IsAudioVar( I, "Pain Response\\Sound", SuperDestructibleObj.m_hAudioPackage, m_PainSoundID ) )
    {
        if( (m_SoundID == -1) && (m_PainSoundID == -1) )
            SuperDestructibleObj.m_hAudioPackage.SetName( "" );

        return( TRUE );
    }

    //
    // Anim
    //
    if( I.IsVar( "Anim\\PlayAnim" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarEnum( g_StringMgr.GetString( m_AnimIndex ) );                
        }
        else
        {
            // this is done when the stage dies.. ( or if we are the first stage )
            if( SuperDestructibleObj.m_hAnimGroup.GetPointer() )
            {

                s32 Index = SuperDestructibleObj.m_AnimPlayer.GetAnimIndex( I.GetVarEnum() );
                if( Index != -1 )
                {
                    m_AnimIndex = g_StringMgr.Add( I.GetVarEnum() );
                    //SuperDestructibleObj.m_AnimPlayer.SetSimpleAnim( Index ); 
                }
                else
                {
                    LOG_ERROR( "GAMEPLAY", "No animation found with name (%s) in anim group (%s) GUID %08X:%08X",
                                            I.GetVarEnum(), SuperDestructibleObj.m_hAnimGroup.GetName(), 
                                            SuperDestructibleObj.GetGuid().GetHigh(), 
                                            SuperDestructibleObj.GetGuid().GetLow() );                        
                }
                
            }
            else
            {
                m_AnimIndex = g_StringMgr.Add( I.GetVarEnum() );
            }
        }

        return TRUE;
    }

    //
    // Buttons
    //
    // Create stage after?
    if( I.IsVar("Create Stage After"))
    {
        if (I.IsRead())
            I.SetVarButton( "Create Stage After" );
        else
            SuperDestructibleObj.CreateStage(iStage, TRUE) ;

        return TRUE ;
    }

    // Create stage before?
    if( I.IsVar("Create Stage Before"))
    {
        if (I.IsRead())
            I.SetVarButton( "Create Stage Before" );
        else
            SuperDestructibleObj.CreateStage(iStage, FALSE) ;

        return TRUE ;
    }

    // Delete stage?
    if( I.IsVar("Delete Stage"))
    {
        if (I.IsRead())
        {
            I.SetVarButton( "Delete Stage" );
        }
        else
        {
            ASSERT(iStage != -1) ;
            SuperDestructibleObj.DeleteStage(iStage) ;
        }

        return TRUE ;
    }

    // Not found...
    return FALSE;
}



