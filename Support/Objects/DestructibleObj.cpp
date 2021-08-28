//=============================================================================
//
// DestructibleObj.cpp
//
///=============================================================================

//=============================================================================
// INCLUDES
//=============================================================================

#include "DestructibleObj.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "Render\Render.hpp"
#include "Debris\Debris_mgr.hpp"
#include "Objects\ParticleEmiter.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "Decals\DecalMgr.hpp"

//=============================================================================
// CONSTANTS
//=============================================================================

#define MAX_PAIN_RESPONSES          16
#define NUM_FADING_RESPONSES        10

//=============================================================================
// SHARED LOCALS
//=============================================================================
 
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


//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

f32 g_StagerDestructionTime = 0.1f;

//=============================================================================
// TYPE
//=============================================================================
static struct destructible_obj_desc : public object_desc
{
    //-------------------------------------------------------------------------

    destructible_obj_desc( void ) : object_desc( 

            object::TYPE_DESTRUCTIBLE_OBJ, 
            "Destructible Object", 
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
            object::ATTR_SPACIAL_ENTRY      |
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_DESTRUCTABLE_OBJECT|
            object::ATTR_DAMAGEABLE, 

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC            |
            FLAGS_NO_ICON               |
            FLAGS_BURN_VERTEX_LIGHTING  ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void )
    {
        return new destructible_obj;
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
} s_Destructible_Desc;


//=============================================================================

const object_desc&  destructible_obj::GetTypeDesc     ( void ) const
{
    return s_Destructible_Desc;
}

//=============================================================================

const object_desc&  destructible_obj::GetObjectType   ( void )
{
    return s_Destructible_Desc;
}

//=============================================================================
// FUNCTIONS
//=============================================================================

destructible_obj::destructible_obj( void )
{
    m_Health            = 100.0f;
    m_Destroyed         = FALSE;
    m_DecalGroup        = 0;

    m_ParticleScale     = 1;
    m_PainRadius        = 100.0f;
    m_PainAmount        = 0;
    
    m_DestructionTime   = 0;
    m_DebrisCount       = 1.0f;
    m_MinDebrisVelocity = 500.0f;
    m_MaxDebrisVelocity = 600.0f;
    m_DebrisLife        = 6.0f;
    m_ActivateOnDestruction = NULL_GUID;
    m_SoundID               = -1;
    m_PainSoundID           = -1;

    m_PainParticleScaleMin = 0.75f;
    m_PainParticleScaleMax = 1.25f;    
}

//=============================================================================

destructible_obj::~destructible_obj( void )
{
    s32 i;
    for (i=0;i<MAX_PAIN_RESPONSES;i++)
    {
        if (s_PainResponseInfo[i].Owner == GetGuid())
        {
            g_AudioMgr.Release( s_PainResponseInfo[i].AudioVoice, 0.0f );
            s_PainResponseInfo[i].Owner = 0;
            s_PainResponseInfo[i].AudioVoice = 0;
            s_PainResponseInfo[i].InitialScale = 1.0f;

            if (SLOT_NULL != g_ObjMgr.GetSlotFromGuid( s_PainResponseInfo[ i ].Emitter ))
            {
                g_ObjMgr.DestroyObject( s_PainResponseInfo[ i ].Emitter );
            }

            s_PainResponseInfo[ i ].Emitter = 0;
        }
    }
}

//=============================================================================

bbox destructible_obj::GetLocalBBox( void ) const
{
    return play_surface::GetLocalBBox();
}      

//=============================================================================

void destructible_obj::OnColCheck( void )
{    
    play_surface::OnColCheck();   
}

//=============================================================================

xbool destructible_obj::IsDestroyed( void )
{
    return m_Destroyed;
}

//=============================================================================

void destructible_obj::OnColNotify( object& Object )
{
    play_surface::OnColNotify( Object );
}

//=============================================================================

#ifndef X_RETAIL
void destructible_obj::OnDebugRender ( void )
{
    draw_ClearL2W();
    draw_Sphere( GetPosition(), m_PainRadius, XCOLOR_RED );    
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

    vector3 Pos = GetPosition();
    x_DebugMsg( "X:[%f] Y:[%f] Z:[%f]\n", Pos.GetX(), Pos.GetY(), Pos.GetZ() );
}
#endif // X_RETAIL

//=============================================================================

void destructible_obj::OnRender( void )
{
    CONTEXT( "prop_surface::OnRender" );

    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();
    
    if( pRigidGeom )
    {
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
        
        if ( pRigidGeom->m_nBones > 1 )
        {
            x_throw( xfs( "Prop surface can't use multi-bone geometry (%s)", m_Inst.GetRigidGeomName() ) );
        }
        else
        {
            m_Inst.Render( &GetL2W(), Flags | GetRenderMode() );
        }
    }
    else
    {
#ifdef X_EDITOR
        draw_BBox( GetBBox() );
#endif // X_EDITOR
    }
}

//=============================================================================

void CreatePainResponse( guid               OwnerGuid,
                         const char*        pSoundName, 
                         const char*        pFXName,
                         f32                Scale,
                         const vector3&     Pos,
                         s32                ZoneID,
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
                    g_AudioMgr.Release( ThisVoice, 0.0f );
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
        g_AudioMgr.Release( s_PainResponseInfo[ s_iNextPainResponse ].AudioVoice, 0 );
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
            iVoiceForThisOwner = g_AudioMgr.PlayVolumeClipped( pSoundName, Pos, ZoneID, TRUE );
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


void destructible_obj::OnPain ( const pain& Pain )   // Tells object to recieve pain
{
    if( m_Destroyed )
        return;
    
    if( Pain.GetOriginRTTI().IsKindOf( destructible_obj::GetRTTI() )  )
        m_DestructionTime = g_StagerDestructionTime;
    else
        m_DestructionTime = 0.0f;

    health_handle Handle(GetLogicalName());
    if( !Pain.ComputeDamageAndForce( Handle, GetGuid(), GetBBox().GetCenter() ) )
        return;

    m_Health -= Pain.GetDamage();
    SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );

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
        if (m_PainSoundID != -1)
            pPainSound = g_StringMgr.GetString( m_PainSoundID );

        CreatePainResponse( GetGuid(), 
                            pPainSound,
                            m_hPainParticleEffect.GetName(),
                            x_frand( m_PainParticleScaleMin, m_PainParticleScaleMax ),
                            Pain.GetImpactPoint(),
                            GetZone1(),
                            Orient,
                            GetZone1() );
    }
}

//=============================================================================

void destructible_obj::SetDestroyed( void )
{
    m_Destroyed = TRUE;
    
    BroadcastPain();

    s32 i;

    guid Me = GetGuid();

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
                g_AudioMgr.Release( s_PainResponseInfo[ i ].AudioVoice, 0.0f );
            s_PainResponseInfo[ i ].AudioVoice = 0;

            s_PainResponseInfo[ i ].Owner   = 0;                    
        }
    }

    if( m_ActivateOnDestruction != NULL_GUID )
    {
        //activate this guid
        object* pObj = g_ObjMgr.GetObjectByGuid(m_ActivateOnDestruction);
        if (pObj)
        {
            pObj->OnActivate(TRUE);
        }
    }

    if( m_SoundID != -1 )
    {
        g_AudioMgr.PlayVolumeClipped( g_StringMgr.GetString( m_SoundID ), GetPosition(), GetZone1(), TRUE );
    }

    // Create the debris.
    const matrix4&  Mat = GetL2W();
//        radian3         Rad = Mat.GetRotation();
    vector3         ColX;
    vector3         ColY;
    vector3         ColZ;

    Mat.GetColumns( ColX, ColY, ColZ );

    for( i = 0; i < (s32)m_DebrisCount; i++ )
    {
        ColY.NormalizeAndScale( x_frand( m_MinDebrisVelocity, m_MaxDebrisVelocity) );
        f32 x = x_frand(-R_20,R_20);
        f32 y = x_frand(-R_20,R_20);
        f32 z = x_frand(-R_20,R_20);
        radian3 RandRot( x, y, z );

        u32 VMeshMask = 0;
        const geom* pGeom = m_DebrisInst.GetGeom();
        if (pGeom)
        {
            s32 nVMeshes = pGeom->m_nVirtualMeshes;

            if( nVMeshes > 0 )
            {
                VMeshMask |= (1<<x_irand(0,nVMeshes-1));
            }
            else 
            {
                VMeshMask |= 0xffffffff;
            }
            
            debris_mgr::GetDebrisMgr()->CreateDebris( Mat.GetTranslation(), GetZones(), ColY, RandRot, m_DebrisInst, m_DebrisLife, TRUE, VMeshMask );
        }
    }

    // Specific debris
    {
        const geom* pGeom = m_SpecificDebrisInst.GetGeom();
        if (pGeom)
        {
            s32 nVMeshes  = pGeom->m_nVirtualMeshes;
            u32 VMeshMask = 0;

            radian3 Rot(0,0,0);

            s32 i;
            for (i=0;i<nVMeshes;i++)
            {
                VMeshMask = (1<<i);
            
                ColY.NormalizeAndScale( x_frand( m_MinDebrisVelocity, m_MaxDebrisVelocity) );
                debris_mgr::GetDebrisMgr()->CreateDebris( Mat.GetTranslation(), GetZones(), ColY, Rot, m_SpecificDebrisInst, m_DebrisLife, TRUE, VMeshMask );
            }
        }
    }

    // create the decal
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
                
    // Create the particle effect.
    guid gPE;
    {
        matrix4 Mat = GetL2W();
        radian3 Rot = Mat.GetRotation();
        vector3 Dir(0,0,1);
        Dir.Rotate(Rot);
        
        gPE = particle_emitter::CreatePresetParticleAndOrient( m_hParticleEffect.GetName(), Dir, 
                                                        Mat.GetTranslation(), GetZone1() );
    }

    object* pObj = g_ObjMgr.GetObjectByGuid( gPE );
    if (pObj)
    {
        if (pObj->IsKindOf( particle_emitter::GetRTTI() ))
        {
            particle_emitter& PE = particle_emitter::GetSafeType( *pObj );

            PE.SetScale( m_ParticleScale );
        }
    }
    // Switch to the broken glass LOD.
    rigid_inst& RInst = GetRigidInst();
    const geom* pGeom = RInst.GetRigidGeom();
    if( pGeom && (pGeom->m_nVirtualMeshes == 2) )
    {
        u32 VMeshMask = RInst.GetVMeshMask().VMeshMask;
        VMeshMask ^= 0x3;   // swap bits one and two
        RInst.SetVMeshMask( VMeshMask );
    }

    //
    // Update PolyCache
    //
    {
        g_PolyCache.InvalidateCells( GetBBox(), GetGuid() );
    }
}

//=============================================================================

void destructible_obj::OnAdvanceLogic( f32 DeltaTime )
{
    if( m_DestructionTime > 0.0f )
    {
        m_DestructionTime -= DeltaTime;
        return;
    }

    if( m_Health <= 0.0f )
    {
        SetDestroyed();
    }

    SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );
}

//=============================================================================

void destructible_obj::BroadcastPain( void )
{
    //create pain area
    bbox Pain( GetPosition(), m_PainRadius );
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

        f32 DamageMax = m_PainAmount;
        f32 ForceMax = m_PainAmount;

        //for living objects (player, npc's) we will add in cover protection
        if (pObject->GetAttrBits() & ATTR_LIVING )
        {
                //calc LOS, since we don't want as much force or damage if they are behind cover
//                vector3 ObjPos = pObject->GetBBox().GetCenter();
            vector3 ColPos = GetPosition();
            ColPos.GetY() = GetBBox().Max.GetY();

            f32 nCoverAmount = 0;

            // Commented out because the HasLOS ends up calling the collision
            // manager which does an ObjectMgr.SelectBBox.  There is already
            // a SelectBBox in progress at the top of this function and so
            // an ASSERT kicks off.
/*
            //ok, calc first at 25% height
            ObjPos.Y -= (pObject->GetBBox().GetRadius()/2);
            if (!g_ObjMgr.HasLOS(pObject->GetGuid(), ObjPos,GetGuid(), ColPos ) )
            {
                //no line of sight, reduce impact
                nCoverAmount += 1.0f;
            }

            //ok, calc second at 75% height
            ObjPos.Y += pObject->GetBBox().GetRadius();
            if (!g_ObjMgr.HasLOS( pObject->GetGuid(), ObjPos, GetGuid(), ColPos ) )
            {
                //no line of sight, reduce impact
                nCoverAmount += 1.0f;
            }
*/
            //damage reduction of 1/3 per cover 
            DamageMax = DamageMax * (1.0f-(nCoverAmount/2.0f));
            //force reduction of 1/4 per cover (more concussion than damage)
            ForceMax = ForceMax * (1.0f-(nCoverAmount/3.0f));
        }

        pain_handle Handle("GENERIC_1");
        pain Pain;
        Pain.Setup( Handle, GetGuid(), pObject->GetBBox().GetCenter() );
        Pain.SetDirectHitGuid( pObject->GetGuid() );
        Pain.ApplyToObject( pObject );
    }
}

//=============================================================================

void destructible_obj::OnEnumProp( prop_enum&    List )
{
    play_surface::OnEnumProp( List );

    List.PropEnumHeader( "Destructible Obj", "Destructible Obj Properties", 0 );

    //
    // Generic Debris
    //

    List.PropEnumHeader( "Destructible Obj\\Debris", "This is the debris that is created.", 0 );

    s32 ID = List.PushPath( "Destructible Obj\\Debris\\" );
    m_DebrisInst.OnEnumProp( List );
    List.PopPath( ID );

    
    geom* pGeom = m_DebrisInst.GetGeom();
    if( pGeom )
    {
        for( s32 i = 0; i < pGeom->m_nMeshes; i++ )
        {
            List.PropEnumBool( xfs("Destructible Obj\\Debris\\%s[%d]", pGeom->GetMeshName( i ), i ), "Use this debris mesh?", 0 );
        }
    }
    List.PropEnumFloat( "Destructible Obj\\Debris\\Debris Count", "How many times should we spawn the debris", 0 );
    List.PropEnumFloat( "Destructible Obj\\Debris\\Debris Min Vertical Velocity", "Slowest velocity to shoot the deris out with on the objects World Y axis.", 0 );
    List.PropEnumFloat( "Destructible Obj\\Debris\\Debris Max Vertical Velocity", "Fastest velocity to shoot the deris out with on the objects World Y axis.", 0 );
    List.PropEnumFloat( "Destructible Obj\\Debris\\Debris Life", "How long is the debris going to live (seconds)", 0 );

    //
    //  Specific Debris
    //
    List.PropEnumHeader( "Destructible Obj\\Specific Debris", "This debris is specific to the object being destroyed.", 0 );
    ID = List.PushPath( "Destructible Obj\\Specific Debris\\" );
    m_SpecificDebrisInst.OnEnumProp( List );
    List.PopPath( ID );
    /*
    List.PropEnumFloat( "Destructible Obj\\Specific Debris\\Debris Min Vertical Velocity", "Slowest velocity to shoot the debris out with on the objects World Y axis." );
    List.PropEnumFloat( "Destructible Obj\\Specific Debris\\Debris Max Vertical Velocity", "Fastest velocity to shoot the debris out with on the objects World Y axis." );
    List.PropEnumFloat( "Destructible Obj\\Specific Debris\\Debris Life", "How long is the debris going to live (seconds)" );
    */


    List.PropEnumHeader( "Destructible Obj\\Particles", "This is the debris that is created.", 0 );

    List.PropEnumExternal( "Destructible Obj\\Particles\\Particles Resource",
                            "Resource\0fxo\0",
                            "Particle Resource for this item",
                            PROP_TYPE_MUST_ENUM );
    List.PropEnumFloat   ( "Destructible Obj\\Particles\\Scale", "The scale of the particle effect for this object.", 0 );

    List.PropEnumHeader  ( "Destructible Obj\\Pain Response", "How the object responds to pain.", 0 );
    List.PropEnumExternal( "Destructible Obj\\Pain Response\\Particles",
                            "Resource\0fxo\0",
                            "Particle Resource for when this object receives pain",
                            PROP_TYPE_MUST_ENUM );
    List.PropEnumFloat   ( "Destructible Obj\\Pain Response\\Min Particle Scale", "The minimum scale of the pain particle effect for this object.", 0 );
    List.PropEnumFloat   ( "Destructible Obj\\Pain Response\\Max Particle Scale", "The maximum scale of the pain particle effect for this object.", 0 );
    List.PropEnumExternal( "Destructible Obj\\Pain Response\\Sound",        "Sound\0soundexternal\0","The sound to play when this object receives pain", PROP_TYPE_MUST_ENUM );

    //s32 ID = List.PushPath( "Destructible Obj\\Debris\\" );
    //m_DebrisInst.OnEnumProp( List );
    //List.PopPath( ID );

    List.PropEnumExternal(   "Destructible Obj\\Audio Package",      "Resource\0audiopkg\0", "The audio package associated with this Glass object.", 0 );
    List.PropEnumExternal(   "Destructible Obj\\Char Decal Package", "Resource\0decalpkg\0", "The package of the char decal this object will leave behind.", 0 );
    List.PropEnumInt     (   "Destructible Obj\\Char Decal Group",   "The group index of the char decal this object will leave behind.", 0 );
    List.PropEnumFloat   (   "Destructible Obj\\Hit Points","Number of hit points for this object",PROP_TYPE_EXPOSE );
    List.PropEnumInt     (   "Destructible Obj\\Num Destruction Steps",   "The group index of the char decal this object will leave behind.", 0 );
    List.PropEnumFloat   (   "Destructible Obj\\Pain Radius", "The radius of the pain sphere.", 0 );
    List.PropEnumFloat   (   "Destructible Obj\\Pain Amount", "The amount of pain to send to the person colliding", 0 );
    List.PropEnumGuid    (   "Destructible Obj\\Activate OnDestroy", "Activate this guid on destruction of this object.", 0 );
    List.PropEnumExternal(   "Destructible Obj\\Destruction Sound",        "Sound\0soundexternal\0","The sound to play when this object is destroyed", PROP_TYPE_MUST_ENUM );
}

//=============================================================================

xbool destructible_obj::OnProperty( prop_query&   I    )
{
    if( play_surface::OnProperty( I ) )
        return TRUE;

    //
    // Generic Debris
    //
    s32 ID = I.PushPath( "Destructible Obj\\Debris\\" );
    if( m_DebrisInst.OnProperty( I ) )
    {
        return TRUE;
    }
    I.PopPath( ID );

    if( I.VarFloat( "Destructible Obj\\Debris\\Debris Count", m_DebrisCount ) )
    {
        // Keep the value as a whole number.
        m_DebrisCount = (f32)((s32)m_DebrisCount);

        return TRUE;
    }


    if( I.VarFloat( "Destructible Obj\\Debris\\Debris Min Vertical Velocity", m_MinDebrisVelocity ) )
    {
        return TRUE;
    }

    if( I.VarFloat( "Destructible Obj\\Debris\\Debris Max Vertical Velocity", m_MaxDebrisVelocity ) )
    {
        return TRUE;
    }


    if( I.VarFloat( "Destructible Obj\\Debris\\Debris Life", m_DebrisLife ) )
    {
        return TRUE;
    }

/*    // Get the blueprint file name.
    if( I.IsVar( "Destructible Obj\\Debris\\Debris Bluepint" ) )
    {   
        if( I.IsRead() )
        {
            if( m_DebrisBpxStrID > 0 )
            {
                const char* pStr = g_StringMgr.GetString( m_DebrisBpxStrID );
                I.SetVarFileName( pStr, x_strlen( pStr ) );
            }
            else
            {
                I.SetVarFileName( "", 1 );
            }
        }
        else
        {
            if( x_strlen( I.GetVarFileName() ) > 0 )
                m_DebrisBpxStrID = g_StringMgr.Add( I.GetVarFileName() );
        }

        return TRUE;
    }
*/




    //
    // Specific Debris
    //
    ID = I.PushPath( "Destructible Obj\\Specific Debris\\" );
    if( m_SpecificDebrisInst.OnProperty( I ) )
    {
        return TRUE;
    }
    I.PopPath( ID );



    // External Particle.
    if( I.IsVar( "Destructible Obj\\Particles\\Particles Resource" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hParticleEffect.GetName(), RESOURCE_NAME_SIZE );
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

    if( I.VarFloat( "Destructible Obj\\Particles\\Scale", m_ParticleScale ) )
        return TRUE;

    if( I.IsVar( "Destructible Obj\\Pain Response\\Particles" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hPainParticleEffect.GetName(), RESOURCE_NAME_SIZE );
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

    if( I.VarFloat( "Destructible Obj\\Pain Response\\Min Particle Scale", m_PainParticleScaleMin ) )
        return TRUE;
    if( I.VarFloat( "Destructible Obj\\Pain Response\\Max Particle Scale", m_PainParticleScaleMax ) )
        return TRUE;
    
        
    // External Audio.
    if( I.IsVar( "Destructible Obj\\Audio Package" ) )
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

    if ( I.IsVar( "Destructible Obj\\Char Decal Package" ) )
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

    if ( I.VarInt( "Destructible Obj\\Char Decal Group", m_DecalGroup ) )
    {
        return TRUE;
    }

    if( I.IsVar("Destructible Obj\\Hit Points" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_Health );
        }
        else
        {
            m_Health = I.GetVarFloat();
        }
        return TRUE;
    }
    
    if( I.VarFloat( "Destructible Obj\\Pain Radius", m_PainRadius ) )
        return TRUE;
    
    if( I.VarFloat( "Destructible Obj\\Pain Amount", m_PainAmount ) )
        return TRUE;

    if (I.VarGUID( "Destructible Obj\\Activate OnDestroy",m_ActivateOnDestruction))
        return TRUE;

    // External
    if( SMP_UTIL_IsAudioVar( I, "Destructible Obj\\Destruction Sound", m_hAudioPackage, m_SoundID ) )
    {
        if( (m_SoundID == -1) && (m_PainSoundID == -1) )
            m_hAudioPackage.SetName( "" );

        return( TRUE );
    }

    else if( SMP_UTIL_IsAudioVar( I, "Destructible Obj\\Pain Response\\Sound", m_hAudioPackage, m_PainSoundID ) )
    {
        if( (m_SoundID == -1) && (m_PainSoundID == -1) )
            m_hAudioPackage.SetName( "" );

        return( TRUE );
    }


    return FALSE;
}




//=============================================================================
