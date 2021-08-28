
#include "AlienSpawnTube.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "GameLib\RigidGeomCollision.hpp"
#include "Render\Render.hpp"
#include "EventMgr\EventMgr.hpp"
#include "alienorb.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "GameLib\RenderContext.hpp"
#include "Loco\LocoUtil.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "Characters\Soldiers\Soldier.hpp"
#include "Characters\CharacterState.hpp"
#include "EventMgr\EventMgr.hpp"
#include "Objects\Group.hpp"

//=============================================================================
// CONSTANTS
//=============================================================================

//=============================================================================
// SHARED
//=============================================================================
xstring g_AlienSpawnTubeStringList;

//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

//=============================================================================

static struct alien_spawn_tube_desc : public object_desc
{
    alien_spawn_tube_desc( void ) : object_desc( 
        object::TYPE_ALIEN_SPAWN_TUBE, 
        "Alien Spawn Tube", 
        "AI",
        object::ATTR_COLLIDABLE             |         
        object::ATTR_BLOCKS_ALL_PROJECTILES |         
        object::ATTR_BLOCKS_PLAYER          |         
        object::ATTR_BLOCKS_RAGDOLL         |         
        object::ATTR_BLOCKS_CHARACTER_LOS   |         
        object::ATTR_BLOCKS_PLAYER_LOS      |         
        object::ATTR_BLOCKS_PAIN_LOS        |         
        object::ATTR_BLOCKS_SMALL_DEBRIS    |         
        object::ATTR_RENDERABLE             |
        object::ATTR_NEEDS_LOGIC_TIME       |
        object::ATTR_SPACIAL_ENTRY,

        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_IS_DYNAMIC            |
        FLAGS_NO_ICON               |
        FLAGS_BURN_VERTEX_LIGHTING ) {}

        //-------------------------------------------------------------------------

        virtual object* Create( void ) { return new alien_spawn_tube; }

} s_AlienSpawnTube_Desc;

//=============================================================================

alien_spawn_tube::alien_spawn_tube()
{
    m_ReservedBy                = NULL_GUID;
    m_TemplateIDToSpawn         = -1;
    
    m_AnimName                  = -1;
    m_AnimGroupName             = -1;
    m_AnimPlayTime              = 1.0f;
    m_AnimFlags                 = 0;

    m_iSpawnAnimName            = g_StringMgr.Add( "None" );

    m_State                     = STATE_IDLE;

    m_SpawnedVarName            = -1;
    m_SpawnedVarHandle          = HNULL;

    m_SpawnGroup                = NULL_GUID;

    m_gLastSpawnedNPC           = NULL_GUID;

    // Disable logic.  We'll turn it on when an orb enters
    SetAttrBits( GetAttrBits() & (~(object::ATTR_NEEDS_LOGIC_TIME) ));
}

//=============================================================================

alien_spawn_tube::~alien_spawn_tube()
{
    
}

//=============================================================================

const object_desc& alien_spawn_tube::GetTypeDesc( void ) const
{
    return s_AlienSpawnTube_Desc;
}

//=============================================================================

const object_desc& alien_spawn_tube::GetObjectType( void )
{
    return s_AlienSpawnTube_Desc;
}

//=============================================================================


void alien_spawn_tube::OnEnumProp      ( prop_enum&    List )
{
    anim_surface::OnEnumProp( List );

    List.PropEnumHeader  ( "Alien Spawn Tube",           "Alien Spawn Tube Properties", 0 );
    List.PropEnumFileName( "Alien Spawn Tube\\Blueprint To Spawn",
                      "Area51 blueprints (*.bpx)|*.bpx|All Files (*.*)|*.*||",
                      "Resource for this item",
                      PROP_TYPE_MUST_ENUM );

    List.PropEnumExternal( "Alien Spawn Tube\\Spawned Object", "global\0global_guid", 
        "(Optional) Global Variable (type guid) where we can store the guid of the object that was spawned; leave blank to not store this info.", PROP_TYPE_MUST_ENUM );

    List.PropEnumGuid    ( "Alien Spawn Tube\\Spawn Group", "Guid of group object that should store the spawned object guid", PROP_TYPE_MUST_ENUM );

    s32 iPath = List.PushPath("Alien Spawn Tube\\");
    m_ActivateOnSpawn.OnEnumProp( List, "ActivateOnSpawn" );
    List.PopPath( iPath );

    if( m_hAnimGroup.GetPointer() )
    {
        //
        // TODO: HACK: We need a better way to do this in the future
        //
        anim_group* pAnimGroup = m_hAnimGroup.GetPointer();

        g_AlienSpawnTubeStringList.Clear();

        s32 i;
        for( i=0; i<pAnimGroup->GetNAnims(); i++ )
        {
            const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( i );

            g_AlienSpawnTubeStringList += AnimInfo.GetName();
            g_AlienSpawnTubeStringList += "~";
        }

        for( i=0; g_AlienSpawnTubeStringList[i]; i++ )
        {
            if( g_AlienSpawnTubeStringList[i] == '~' )
                g_AlienSpawnTubeStringList[i] = 0;
        }

        List.PropEnumEnum( "Alien Spawn Tube\\SpawnAnim", g_AlienSpawnTubeStringList, "What anim to play when spawning a blueprint", PROP_TYPE_EXPOSE );
    }

    List.PropEnumHeader  ( "Spawned Object Anim", "Animation to be played on the spawned object", 0 );
    iPath = List.PushPath( "Spawned Object Anim\\" );

    // Add loco animation properties
    LocoUtil_OnEnumPropAnimFlags(   List,
                                    loco::ANIM_FLAG_PLAY_TYPE_ALL        | 
                                    loco::ANIM_FLAG_END_STATE_ALL        |
                                    loco::ANIM_FLAG_INTERRUPT_BLEND      |
                                    loco::ANIM_FLAG_TURN_OFF_AIMER       |
                                    loco::ANIM_FLAG_RESTART_IF_SAME_ANIM,
                                    m_AnimFlags );
    m_TargetAffecter.OnEnumProp( List, "Target" ); 

    List.PopPath(iPath);
}

//=============================================================================

xbool alien_spawn_tube::OnProperty( prop_query&   I    )
{
    if (anim_surface::OnProperty( I ))
    {
        return TRUE;
    }
    
    if (I.IsSimilarPath("Alien Spawn Tube\\ActivateOnSpawn"))
    {
        s32 iPath = I.PushPath("Alien Spawn Tube\\");
        if (m_ActivateOnSpawn.OnProperty( I, "ActivateOnSpawn" ))
        {
            I.PopPath( iPath );
            return TRUE;
        }
        I.PopPath( iPath );
    }
    else
    if ( I.IsVar  ( "Alien Spawn Tube\\Spawned Object" ))
    {
        if( I.IsRead() )
        {
            if ( m_SpawnedVarName >= 0 )
                I.SetVarExternal( g_StringMgr.GetString(m_SpawnedVarName), 256 );
            else
                I.SetVarExternal("", 256);
            return TRUE;
        }
        else
        {
            if (x_strlen(I.GetVarExternal()) > 0)
            {
                m_SpawnedVarName = g_StringMgr.Add( I.GetVarExternal() );

                if ( !g_VarMgr.GetGuidHandle( g_StringMgr.GetString(m_SpawnedVarName), &m_SpawnedVarHandle ) )
                {
                    m_SpawnedVarHandle = HNULL;
                }
                return TRUE;
            }
        }
    }
    else
    if (I.IsSimilarPath( "Spawned Object Anim" ))
    {        
        s32 iPath = I.PushPath( "Spawned Object Anim\\" );
        if (LocoUtil_OnPropertyAnimFlags(I, 
                                        m_AnimGroupName, 
                                        m_AnimName, 
                                        m_AnimFlags, 
                                        m_AnimPlayTime))
        {
            I.PopPath(iPath);
            return TRUE;
        }
        else
        if( m_TargetAffecter.OnProperty( I, "Target" ) )
        {
            I.PopPath(iPath);
            return TRUE;
        }
        I.PopPath(iPath);
        return FALSE;
    }
    else if( I.IsVar( "Alien Spawn Tube\\Blueprint To Spawn" ) )
    {
        if( I.IsRead() )
        {
            if( m_TemplateIDToSpawn < 0 )
            {
                I.SetVarFileName("",256);
            }
            else
            {
                I.SetVarFileName( g_TemplateStringMgr.GetString( m_TemplateIDToSpawn ), 256 );
            }            
        }
        else
        {
            if ( x_strlen( I.GetVarFileName() ) > 0 )
            {
                m_TemplateIDToSpawn = g_TemplateStringMgr.Add( I.GetVarFileName() );
            }
        }     
        return TRUE;
    }    
    if( I.IsVar( "Alien Spawn Tube\\SpawnAnim" ) )
    {
        if( I.IsRead() )
        {
            if( m_hAnimGroup.GetPointer() )
            {
                anim_group* pAnimGroup = m_hAnimGroup.GetPointer();                

                s32 Index = m_AnimPlayer.GetAnimIndex( g_StringMgr.GetString( m_iSpawnAnimName ) );
                if( Index != -1 )
                {
                    I.SetVarEnum( pAnimGroup->GetAnimInfo(Index).GetName() );
                }
                else
                {
                    I.SetVarEnum( g_StringMgr.GetString( m_iBackupAnimString ) );
                }
            }
            else
            {
                I.SetVarEnum( g_StringMgr.GetString( m_iBackupAnimString ) );
            }
        }
        else
        {
            if( m_hAnimGroup.GetPointer() )
            {
                s32 Index = m_AnimPlayer.GetAnimIndex( I.GetVarEnum() );
                if( Index != -1 )
                {
                    m_iSpawnAnimName = g_StringMgr.Add( I.GetVarEnum() );                    
                }
                else
                {
                    LOG_ERROR( "GAMEPLAY", "Alien Spawntube: No animation found with name (%s) in anim group (%s)",
                        I.GetVarEnum(), m_hAnimGroup.GetName() );                        
                }
            }
            else
            {
                m_iSpawnAnimName = g_StringMgr.Add( I.GetVarEnum() );
            }
        }

        return TRUE;
    }
    else if (I.VarGUID("Alien Spawn Tube\\Spawn Group", m_SpawnGroup ))
    {
        return TRUE;
    }
    return FALSE;
}


void alien_spawn_tube::OnAdvanceLogic( f32 DeltaTime )
{
    anim_surface::OnAdvanceLogic( DeltaTime );

    if (m_State == STATE_SPAWNING_PREWAIT)
    {
        g_EventMgr.HandleSuperEvents( m_AnimPlayer, this );
    }
    else if (m_State == STATE_SPAWNING_POSTWAIT)
    {
        if (m_AnimPlayer.IsAtEnd())
        {
            m_State = STATE_IDLE;
            if( m_hAnimGroup.GetPointer() )
            {
                s32 Index = m_AnimPlayer.GetAnimIndex( g_StringMgr.GetString( m_iBackupAnimString ) );
                if (Index != -1)
                {
                    m_AnimPlayer.SetSimpleAnim( Index );
                }
            }
        }
    }   
}

//=============================================================================

xbool alien_spawn_tube::OrbEnter( guid Orb )
{
    if ((m_State != STATE_IDLE) && (m_State != STATE_SPAWNING_POSTWAIT))
        return FALSE;

    if (m_ReservedBy != Orb)
        return FALSE;

    // Do your thing!
    if( !m_hAnimGroup.GetPointer() )
        return TRUE;    

    s32 Index = m_AnimPlayer.GetAnimIndex( g_StringMgr.GetString( m_iSpawnAnimName ) );

    if (Index == -1)
        return TRUE;

    // Turn logic on. we need to watch for an event
    SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );
    m_AnimPlayer.SetSimpleAnim( Index );

    m_State = STATE_SPAWNING_PREWAIT;

    return TRUE;
}

//=============================================================================

xbool alien_spawn_tube::CanBeReservedByMe( guid Orb )
{
    if ((m_State != STATE_IDLE) && (m_State != STATE_SPAWNING_POSTWAIT))
    {
        return FALSE;
    }
    if (m_gLastSpawnedNPC != NULL_GUID)
    {
        // Can't be reserved if last spawned NPC is still alive
        object_ptr<character> pChar( m_gLastSpawnedNPC );
        if (pChar.IsValid())
        {
            if (pChar->IsAlive())
                return FALSE;
        }        
        m_gLastSpawnedNPC = NULL_GUID;
    }
    if (NULL_GUID == m_ReservedBy)
    {
        return TRUE;
    }
    if (m_ReservedBy == Orb)
    {
        return TRUE;
    }

    return FALSE;
}

//=============================================================================

xbool alien_spawn_tube::Reserve( guid Orb )
{
    if ((m_State != STATE_IDLE) && (m_State != STATE_SPAWNING_POSTWAIT))
        return FALSE;
    if (Orb == m_ReservedBy)
        return TRUE;
    if (m_gLastSpawnedNPC != NULL_GUID)
    {
        // Can't be reserved if last spawned NPC is still alive
        object_ptr<character> pChar( m_gLastSpawnedNPC );
        if (pChar.IsValid())
        {
            if (pChar->IsAlive())
                return FALSE;
        }        
        m_gLastSpawnedNPC = NULL_GUID;
    }

    if (NULL_GUID != m_ReservedBy)
        return FALSE;

    m_ReservedBy = Orb;
    return TRUE;
}

//=============================================================================

xbool alien_spawn_tube::Unreserve( guid Orb )
{
    if (m_ReservedBy == Orb)
    {
        m_ReservedBy = NULL_GUID;
        return TRUE;
    }
    return FALSE;
}

//=============================================================================

void alien_spawn_tube::OnEvent( const event& Event )
{
    switch( Event.Type )
    {
    case event::EVENT_GENERIC:
        {
            const generic_event& GenericEvent = generic_event::GetSafeType( Event );                

            if (m_State == STATE_SPAWNING_PREWAIT)
            {            
                if (x_stricmp( GenericEvent.GenericType, "TUBE_SPAWN_OBJECT" ) == 0)
                {
                    m_State = STATE_SPAWNING_POSTWAIT;

                    // SPAWN THE CONTENTS OF THE TUBE!
                    if (-1 != m_TemplateIDToSpawn)
                    {    
                        const matrix4& L2W = GetL2W();
                        vector3 Pos = L2W.GetTranslation() + vector3(0,-200,0);
                        radian3 Rot = L2W.GetRotation();

                        Rot.Yaw += R_180;
    
                        guid gObj = g_TemplateMgr.CreateSingleTemplate( g_TemplateStringMgr.GetString( m_TemplateIDToSpawn ), Pos, L2W.GetRotation(), GetZone1(), GetZone2() );
                        object* pObj = g_ObjMgr.GetObjectByGuid( gObj );    
                        if (pObj)
                        {
                            if (pObj->IsKindOf( soldier::GetRTTI() ))
                            {
                                soldier& BO = soldier::GetSafeType( *pObj );
                                //BO.OnActivate( FALSE );
                                BO.SetOrbGuid( m_ReservedBy );
                                m_ReservedBy = NULL_GUID;
                                m_gLastSpawnedNPC = gObj;
//                                BO.PlayAnimation( g_StringMgr.GetString( m_AnimGroupName ),
//                                                  g_StringMgr.GetString( m_AnimName ),
//                                                  DEFAULT_BLEND_TIME,m_AnimFlags,0);

                                // setup all the data for a blackopps.
                                character_trigger_state::TriggerData pData;
                                pData.m_ActionType = action_ai_base::AI_PLAY_ANIMATION;
                                pData.m_MustSucceed = TRUE;
                                pData.m_ResponseList.AddFlags( response_list::RF_IGNORE_ATTACKS | 
                                                               response_list::RF_IGNORE_SIGHT | 
                                                               response_list::RF_IGNORE_SOUND | 
                                                               response_list::RF_IGNORE_ALERTS );
                                pData.m_TriggerGuid = 0;
                                pData.m_Blocking = FALSE;
                                pData.m_NextAiState = character_state::STATE_ATTACK;
                                x_strcpy( pData.m_UnionData.m_PlayAnimData.m_AnimGroupName, g_StringMgr.GetString( m_AnimGroupName ) );
                                x_strcpy( pData.m_UnionData.m_PlayAnimData.m_AnimName, g_StringMgr.GetString( m_AnimName ) );
                                pData.m_UnionData.m_PlayAnimData.m_AnimPlayTime = 0.0f;
                                pData.m_UnionData.m_PlayAnimData.m_AnimFlags    = m_AnimFlags ;
                                pData.m_ActionFocus = 0 ;

                                BO.TriggerState(pData);


                                BO.OnActivate(TRUE);
                                BO.SetPostTriggerTarget( m_TargetAffecter.GetGuid() );
                                BO.SetYaw( Rot.Yaw );
                            }

                            // Global var
                            if (m_SpawnedVarHandle.IsNonNull())
                                g_VarMgr.SetGuid( m_SpawnedVarHandle, gObj );

                            if (NULL_GUID != m_SpawnGroup)
                            {
                                object* pObj = g_ObjMgr.GetObjectByGuid(m_SpawnGroup);
                                if (pObj)
                                {
                                    if (pObj->IsKindOf( group::GetRTTI()))
                                    {
                                        group& G = group::GetSafeType( *pObj );

                                        G.AddGuid( gObj );
                                    }
                                }   
                            }

                            // Activation notice
                            guid ActivateThis = m_ActivateOnSpawn.GetGuid();
                            if (ActivateThis)
                            {
                                object* pObj = g_ObjMgr.GetObjectByGuid( ActivateThis );
                                if (pObj)
                                {
                                    pObj->OnActivate( TRUE );
                                }
                            }
                        }
                    }
                }
            }
        }
        break;
    }     
}


//=============================================================================

#ifdef X_EDITOR

s32 alien_spawn_tube::OnValidateProperties( xstring& ErrorMsg )
{
    // Make sure we call base class to get errors
    s32 nErrors = anim_surface::OnValidateProperties( ErrorMsg );

    if (NULL != m_SpawnGroup)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid(m_SpawnGroup);
        if (NULL == pObj)
        {
            nErrors++;
            ErrorMsg += "ERROR: Spawn Group guid for Alien Spawn Tube refers to nonexistant object [";
            ErrorMsg += guid_ToString( m_SpawnGroup );
            ErrorMsg += "]\n";
        }
        else
        {
            if (!pObj->IsKindOf( group::GetRTTI() ))
            {
                nErrors++;
                ErrorMsg += "ERROR: Spawn Group guid for Alien Spawn Tube refers to object that is not a group [";
                ErrorMsg += guid_ToString( m_SpawnGroup );
                ErrorMsg += "]\n";
            }
        }
    }

    return nErrors;
}

#endif
//=============================================================================

//=============================================================================

//=============================================================================