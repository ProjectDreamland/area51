//==============================================================================
//
//  GZCoreObj.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  define HERE
//
//==============================================================================

//==========================================================================
// INCLUDE
//==========================================================================
#include "GZCoreObj.hpp"
#include "e_Draw.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "AlienOrb.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "Player.hpp"
#include "Characters\Character.hpp"
#include "Loco\LocoUtil.hpp"
#include "objects\group.hpp"
#include "Objects\ParticleEmiter.hpp"

//==========================================================================
// DEFINES
//==========================================================================

//==========================================================================
// GLOBAL
//==========================================================================

//==========================================================================
// FUNTIONS
//==========================================================================
static struct ground_zero_core_obj_desc : public object_desc
{
    ground_zero_core_obj_desc( void ) : object_desc( 
        object::TYPE_GZ_CORE_OBJ, 
        "GZ Core Object",
        "AI",

        object::ATTR_COLLIDABLE             |
        object::ATTR_BLOCKS_ALL_PROJECTILES |
        object::ATTR_TRANSPARENT            |
        object::ATTR_NEEDS_LOGIC_TIME       |
        object::ATTR_DAMAGEABLE             |
        object::ATTR_SPACIAL_ENTRY,

        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_IS_DYNAMIC    ) {}

        virtual object* Create( void ) { return new ground_zero_core_obj; }

#ifdef X_EDITOR

        virtual s32  OnEditorRender( object& Object ) const
        {
            (void)Object;            
            //if( (Object. GetAttrBits() & object::ATTR_EDITOR_SELECTED) || 
            //    (Object.GetAttrBits() & object::ATTR_EDITOR_PLACEMENT_OBJECT) )
                Object.OnDebugRender();

            return EDITOR_ICON_TWO_WAY_ARROW;
        }
#endif // X_EDITOR

} s_ground_zero_core_obj_desc ;

//==============================================================================

const object_desc&  ground_zero_core_obj::GetTypeDesc ( void ) const
{
    return s_ground_zero_core_obj_desc;
}

//==============================================================================

const object_desc&  ground_zero_core_obj::GetObjectType ( void )
{
    return s_ground_zero_core_obj_desc;
}

//==============================================================================
ground_zero_core_obj::ground_zero_core_obj(void)
{
    m_MarkerStart = 0;
    m_MarkerEnd = 0;
    m_GroupShieldGUID = 0;
    m_FxGUID = 0;
    m_FxGUIDShort = 0;
    m_SuperD_obj = 0;
    m_SoundEmitterFlare = 0;
    m_SoundEmitterShield = 0;


    m_ShieldDetected = FALSE;
    m_LogicActive = FALSE;
}

//==============================================================================
ground_zero_core_obj::~ground_zero_core_obj(void)
{
}
//==============================================================================
void ground_zero_core_obj::OnInit( void )
{
}

//==============================================================================
 
bbox ground_zero_core_obj::GetLocalBBox( void ) const
{ 
    bbox b;

    b.Set (vector3(0,0,0),vector3(10,10,10));
    return( b );
}

//==============================================================================
void ground_zero_core_obj::OnDebugRender( void )
{
#ifdef X_EDITOR
    if( m_MarkerStart != 0 && m_MarkerEnd != 0 )
    {
        if( m_ShieldDetected )
            draw_Line( g_ObjMgr.GetObjectByGuid(m_MarkerStart)->GetPosition(), g_ObjMgr.GetObjectByGuid(m_MarkerEnd)->GetPosition(), XCOLOR_GREEN );
        else if( m_LogicActive )
            draw_Line( g_ObjMgr.GetObjectByGuid(m_MarkerStart)->GetPosition(), g_ObjMgr.GetObjectByGuid(m_MarkerEnd)->GetPosition(), XCOLOR_RED );
        else
            draw_Line( g_ObjMgr.GetObjectByGuid(m_MarkerStart)->GetPosition(), g_ObjMgr.GetObjectByGuid(m_MarkerEnd)->GetPosition(), XCOLOR_BLUE );
    }

    if( m_GroupShieldGUID )
    {
        group* g = (group*)g_ObjMgr.GetObjectByGuid (m_GroupShieldGUID);
        draw_Line (GetPosition (),g->GetPosition (), XCOLOR_GREEN );
        for( s32 i = 0 ; i < g->GetNumChildren() ; i ++ )
        {
            object* objPtr = g_ObjMgr.GetObjectByGuid( g->GetChild(i) ); 
            draw_BBox(objPtr->GetBBox(),XCOLOR_GREEN);

            draw_Line(GetPosition(), objPtr->GetPosition(), XCOLOR_RED );
        }
    }

    if( GetSafeObject(m_FxGUID) )
        draw_Line(GetPosition(), g_ObjMgr.GetObjectByGuid(m_FxGUID)->GetPosition(), XCOLOR_RED );

    if( GetSafeObject(m_FxGUIDShort) )
        draw_Line( GetPosition(), g_ObjMgr.GetObjectByGuid(m_FxGUIDShort)->GetPosition(), XCOLOR_RED );

    if( GetSafeObject(m_SoundEmitterFlare) )
        draw_Line( GetPosition(), g_ObjMgr.GetObjectByGuid(m_SoundEmitterFlare)->GetPosition(), XCOLOR_YELLOW);

    if( GetSafeObject(m_SoundEmitterShield) )
        draw_Line( GetPosition(), g_ObjMgr.GetObjectByGuid(m_SoundEmitterShield)->GetPosition(), XCOLOR_YELLOW);
#endif
}


//==============================================================================
void ground_zero_core_obj::OnAdvanceLogic( f32 DeltaTime )
{
    (void)DeltaTime;

    if( m_SuperD_obj == NULL && m_SuperDGUID != 0 )
    {
        m_SuperD_obj = (super_destructible_obj*)g_ObjMgr.GetObjectByGuid( m_SuperDGUID );
    }
    
    if( m_SuperD_obj != NULL )
    {
        if( m_SuperD_obj->IsDestroyed () )
        {
            // kick on the the Flare sound emitter
            if( GetSafeObject(m_SoundEmitterFlare) )
            {
                g_ObjMgr.GetObjectByGuid(m_SoundEmitterFlare)->OnActivate(TRUE);
            }

            m_LogicActive = TRUE;
        }
        else
            m_LogicActive = FALSE;
    }

    if( m_LogicActive == FALSE )
        return;

    // run logic on ground zero core
    g_CollisionMgr.RaySetup( GetGuid(), g_ObjMgr.GetObjectByGuid(m_MarkerStart)->GetPosition(), g_ObjMgr.GetObjectByGuid(m_MarkerEnd)->GetPosition() );
    // Only need one collision to say that we can't throw there
    g_CollisionMgr.SetMaxCollisions(1);
    // Perform collision
    g_CollisionMgr.CheckCollisions( object::TYPE_PROP_SURFACE, object::ATTR_COLLIDABLE , object::ATTR_COLLISION_PERMEABLE );

    if (  g_CollisionMgr.m_nCollisions != 0  )
    {
        guid HitGUID = 0;
        HitGUID = g_CollisionMgr.m_Collisions[0].ObjectHitGuid;
        group* g = (group*)g_ObjMgr.GetObjectByGuid(m_GroupShieldGUID);

        for( s32 i = 0 ; i < g->GetNumChildren() ; i ++ )
        {
            if( HitGUID == g->GetChild(i) && m_ShieldDetected == FALSE )
            {
                // one of the shields in the group was hit by the ray. kick off
                // a particle and tell the shield to play something.
                if( GetSafeObject(m_FxGUID) )
                    g_ObjMgr.GetObjectByGuid(m_FxGUID)->OnActivate(FALSE);
                if( GetSafeObject(m_FxGUIDShort) )
                    g_ObjMgr.GetObjectByGuid(m_FxGUIDShort)->OnActivate(TRUE);
                if( GetSafeObject(m_SoundEmitterShield) )
                    g_ObjMgr.GetObjectByGuid(m_SoundEmitterShield)->OnActivate(TRUE);
 
                m_ShieldDetected = TRUE;
            }
        }
        return;
    }
    else
    {
        // Not hitting any of the grouped shields
        if( GetSafeObject(m_FxGUID) )
            g_ObjMgr.GetObjectByGuid(m_FxGUID)->OnActivate(TRUE);
        if( GetSafeObject(m_FxGUIDShort) )
            g_ObjMgr.GetObjectByGuid(m_FxGUIDShort)->OnActivate(FALSE);
        if( GetSafeObject(m_SoundEmitterShield) )
            g_ObjMgr.GetObjectByGuid(m_SoundEmitterShield)->OnActivate(FALSE);

        m_ShieldDetected = FALSE;
    }
}

//=============================================================================

void ground_zero_core_obj::OnEnumProp      ( prop_enum&    List )
{
    object::OnEnumProp( List );

    List.PropEnumHeader( "GZ Core Object",                                  "Ground Zero Core Object Properties", 0 );
    List.PropEnumHeader( "GZ Core Object\\Control Guids",                   "List of Guids that are needed to have the Core Object function", 0 );
    List.PropEnumGuid  ( "GZ Core Object\\Control Guids\\LOS Marker Start", "Marker that will start the LOS check", 0 );
    List.PropEnumGuid  ( "GZ Core Object\\Control Guids\\LOS Marker End",   "Marker that will end the LOS check", 0 );
    List.PropEnumGuid  ( "GZ Core Object\\Control Guids\\SuperD Guid",      "Core SuperD that will block the line of sight untill it is destroyed", 0 );
    List.PropEnumGuid  ( "GZ Core Object\\Control Guids\\Group of Shield",  "Group of objects (shields) that the particle effect is going to react to.", 0 );
    List.PropEnumGuid  ( "GZ Core Object\\Control Guids\\Partical Emitter", "The particle of the beam that is comeing from the core. this is changed if it hits a shield.", 0 );
    List.PropEnumGuid  ( "GZ Core Object\\Control Guids\\Partical Emitter Short", "The particle of the beam that is comeing from the core. this is changed if it hits a shield.", 0 );

    List.PropEnumGuid  ( "GZ Core Object\\Control Guids\\Sound Emitter Shield", ".", 0 );
    List.PropEnumGuid  ( "GZ Core Object\\Control Guids\\Sound Emitter Solar Flare", ".", 0 );


    // other stuff goes here.
}

//=============================================================================

xbool ground_zero_core_obj::OnProperty      ( prop_query&   I    )
{
    if( object::OnProperty( I ) )
    {
        // do nothing
    }
    else if( I.VarGUID( "GZ Core Object\\Control Guids\\LOS Marker Start"   , m_MarkerStart ) )     {    }
    else if( I.VarGUID( "GZ Core Object\\Control Guids\\LOS Marker End"     , m_MarkerEnd ) )       {    }
    else if( I.VarGUID( "GZ Core Object\\Control Guids\\SuperD Guid"        , m_SuperDGUID ) )      {    }
    else if( I.VarGUID( "GZ Core Object\\Control Guids\\Group of Shield"    , m_GroupShieldGUID ) ) {    }
    else if( I.VarGUID( "GZ Core Object\\Control Guids\\Partical Emitter"   , m_FxGUID ) )          {    }
    else if( I.VarGUID( "GZ Core Object\\Control Guids\\Partical Emitter Short"     , m_FxGUIDShort ) )          {    }
    else if( I.VarGUID( "GZ Core Object\\Control Guids\\Sound Emitter Shield"        , m_SoundEmitterShield ) )    {    }
    else if( I.VarGUID( "GZ Core Object\\Control Guids\\Sound Emitter Solar Flare"   , m_SoundEmitterFlare ) )     {    }

    else 
    {
        return FALSE;
    }
    return TRUE;
}

//=========================================================================

void ground_zero_core_obj::OnPain( const pain& Pain )
{   
    (void)Pain;
}

//=========================================================================
object* ground_zero_core_obj::GetSafeObject( guid GUID )
{
    object* pObj;
    pObj = NULL;
    pObj = g_ObjMgr.GetObjectByGuid(GUID);
    return( pObj );
}
