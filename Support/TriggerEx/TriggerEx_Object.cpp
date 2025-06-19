//==============================================================================
//
//  TriggerEx_Object.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "TriggerEx_Object.hpp"
#include "..\Support\TriggerEx\TriggerEx_Manager.hpp"
#include "Entropy.hpp"
#include "Render\Editor\editor_icons.hpp"
#include "..\Support\Globals\Global_Variables_Manager.hpp"

#include "Meta\trigger_meta_label.hpp"
#include "Meta\trigger_meta_breakpoint.hpp"

#include "Objects\BaseProjectile.hpp"
#include "Objects\Player.hpp"
#include "characters\character.hpp"
#include "Dictionary\global_dictionary.hpp"
#ifdef X_EDITOR
#include "actions\action_play_2d_sound.hpp"
#endif

//========================================================================
// GLOBALS
//=========================================================================

#ifdef X_EDITOR
static trigger_ex_object::triggex_ex_copy_data s_ActionCopyData;
#endif //X_EDITOR

static f32              s_SphereRadius = 100.0f;
static sphere           s_EditSphere(vector3(0,0,0), s_SphereRadius);

static const xcolor     s_TriggerColor_Sleep        (150,150,150);
static const xcolor     s_TriggerColor_Checking     (0,255,0);
static const xcolor     s_TriggerColor_Recovering   (255,255,0);
static const xcolor     s_TriggerColor_Delaying     (255,0,255);
  
static const s32        MAX_STRING_LEN = 255;

extern xbool g_game_running;
extern xbool g_first_person;
extern s32   g_TriggerAdvLogicCount;

#define TRIGGER_DATA_VERSION   1001

#ifndef CONFIG_RETAIL
xtick           xtLastTriggerExeTime     = 0;
#endif

//=========================================================================
// Spatial type table
trigger_ex_object::spatial_pair                 trigger_ex_object::s_SpatialPairTable[] = 
{
        spatial_pair("AXIS_CUBE",               trigger_ex_object::SPATIAL_TYPE_AXIS_CUBE),
        spatial_pair("SPHERICAL",               trigger_ex_object::SPATIAL_TYPE_SPHERICAL),
        spatial_pair( k_EnumEndStringConst,     trigger_ex_object::SPATIAL_TYPES_INVALID),  //**MUST BE LAST**//
};
trigger_ex_object::spatial_table                trigger_ex_object::s_SpatialEnumTable(s_SpatialPairTable);

//=========================================================================
// Activation type table
trigger_ex_object::activation_pair              trigger_ex_object::s_ActivationPairTable[] = 
{
        activation_pair("PLAYER",               trigger_ex_object::SPATIAL_ACTIVATE_ON_PLAYER),
        activation_pair("NPC",                  trigger_ex_object::SPATIAL_ACTIVATE_ON_NPC),
        activation_pair("BULLET",               trigger_ex_object::SPATIAL_ACTIVATE_ON_BULLET),
        activation_pair("NPC_OR_PLAYER",        trigger_ex_object::SPATIAL_ACTIVATE_ON_NPC_OR_PLAYER),
        activation_pair( k_EnumEndStringConst,  trigger_ex_object::SPATIAL_ACTIVATION_TYPES_INVALID),  //**MUST BE LAST**//
};
trigger_ex_object::activation_table             trigger_ex_object::s_ActivationEnumTable(s_ActivationPairTable);

//=========================================================================
// Activation type table
trigger_ex_object::activation_bullet_pair       trigger_ex_object:: s_ActivationBulletTypePairTable[] = 
{
        activation_bullet_pair("ALL",           trigger_ex_object::SPATIAL_ACTIVATE_BULLET_TYPES_ALL),
        activation_bullet_pair("BULLET",        trigger_ex_object::SPATIAL_ACTIVATE_BULLET_TYPES_BULLET),
        activation_bullet_pair("MES_ALT_FIRE",   trigger_ex_object::SPATIAL_ACTIVATE_BULLET_TYPES_MES_ALT_FIRE),
        activation_bullet_pair("NPC_BULLET",   trigger_ex_object::SPATIAL_ACTIVATE_BULLET_TYPES_NPC_BULLET),
        activation_bullet_pair("PLAYER_BULLET",   trigger_ex_object::SPATIAL_ACTIVATE_BULLET_TYPES_PLAYER_BULLET),
        activation_bullet_pair(k_EnumEndStringConst, trigger_ex_object::SPATIAL_ACTIVATION_BULLET_TYPES_END), //**MUST BE LAST **//
};
trigger_ex_object::activation_bullet_table      trigger_ex_object:: s_ActivationBulletTypeEnumTable(s_ActivationBulletTypePairTable);


//=========================================================================
// Response type table
trigger_ex_object::response_pair                trigger_ex_object::s_ResponsePairTable[] = 
{
        response_pair("IF_STILL_INSIDE",        trigger_ex_object::SPATIAL_RESPONSE_IF_STILL_INSIDE),
        response_pair("RETURN_TO_SLEEP",        trigger_ex_object::SPATIAL_RESPONSE_RETURN_TO_SLEEP),
        response_pair( k_EnumEndStringConst,    trigger_ex_object::SPATIAL_RESPONSE_TYPE_INVALID),  //**MUST BE LAST**//
};
trigger_ex_object::response_table               trigger_ex_object::s_ResponseEnumTable(s_ResponsePairTable);

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

struct trigger_ex_object_desc : public object_desc
{
    trigger_ex_object_desc( void ) : object_desc( 
        object::TYPE_TRIGGER_EX, 
        "TriggerEx", 
        "SCRIPT",

        object::ATTR_NEEDS_LOGIC_TIME       |
        object::ATTR_SPACIAL_ENTRY		    |
        object::ATTR_COLLIDABLE             |
        object::ATTR_COLLISION_PERMEABLE    |
        object::ATTR_BLOCKS_ALL_ACTORS      |
        object::ATTR_BLOCKS_ALL_PROJECTILES |
        object::ATTR_RENDERABLE,

        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_TARGETS_OBJS          |
        FLAGS_IS_DYNAMIC ) 
    {
        m_bRenderSpatial = TRUE;
        m_bBreakOnError  = FALSE;
        m_bRetryOnError  = TRUE;
    }

    //-------------------------------------------------------------------------

    virtual void OnEnumProp( prop_enum& List )
    {
        object_desc::OnEnumProp( List );
        List.PropEnumBool   ( "ObjectDesc\\Show Spatial",     "Show spacial fields always.", 0 );
        List.PropEnumBool   ( "ObjectDesc\\Break on Error",   "Pause editor when a trigger error occurs.", 0 );
        List.PropEnumBool   ( "ObjectDesc\\Retry on Error",   "Retry the same action again.", 0 );
    }
    
    //-------------------------------------------------------------------------

    virtual xbool OnProperty( prop_query&  I )
    {
        if( object_desc::OnProperty( I ) )
            return TRUE;
        
        if( I.VarBool( "ObjectDesc\\Show Spatial", m_bRenderSpatial ) )
            return TRUE;
        
        if( I.VarBool( "ObjectDesc\\Break on Error", m_bBreakOnError ) )
            return TRUE;

        if( I.VarBool( "ObjectDesc\\Retry on Error", m_bRetryOnError ) )
            return TRUE;

        return FALSE;
    }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        //object_desc::OnEditorRender( Object );
        editor_icon Icon = EDITOR_ICON_TRIGGER;

        if( Object.IsKindOf( trigger_ex_object::GetRTTI() ) )
        {
            trigger_ex_object& Trigger = trigger_ex_object::GetSafeType( Object );   

            if ( Trigger.GetAttrBits() & object::ATTR_EDITOR_SELECTED )
            {
                //selected
                Trigger.OnDebugRender();
                draw_Label( Trigger.GetPosition(), xcolor(255,255,255,255), Trigger.GetTriggerName() );
            }

            if ( m_bRenderSpatial || ( Trigger.GetAttrBits() & object::ATTR_EDITOR_SELECTED ))
            {
                Trigger.OnRenderSpatial();
            }
            
            if (g_game_running && Trigger.IsActive())
            {
                draw_Sphere( Trigger.GetPosition(), 50, Trigger.m_CurrentColor );
            }

            switch (Trigger.GetTriggerType())
            {
            case trigger_ex_object::TRIGGER_TYPE_SIMPLE:
                Icon = EDITOR_ICON_TRIGGER_SIMPLE;
                break;
            case trigger_ex_object::TRIGGER_TYPE_SPATIAL:
                Icon = EDITOR_ICON_TRIGGER_SPATIAL;
                break;
            case trigger_ex_object::TRIGGER_TYPE_VIEWABLE:
                Icon = EDITOR_ICON_TRIGGER_VIEWABLE;
                break;
            }      
            
            EditorIcon_Draw( Icon, Trigger.GetL2W(), FALSE, Trigger.m_CurrentColor );
            
            if (Trigger.m_DrawActivationIcon > 0)
            {
                xcolor Color;
                if ( Trigger.m_DrawActivationIcon == 1 )
                {
                    Color = xcolor( 255, 255, 0, 255);
                    Trigger.m_DrawActivationIcon = 0;
                }
                else
                {
                    Color = xcolor( 0, 255, 255, 255);
                }

                if (Trigger.m_DrawError)
                {
                    Color = xcolor( 255, 0, 0, 255);
                }

                EditorIcon_Draw( EDITOR_ICON_LOOP, Trigger.GetL2W(), FALSE, Color );
                return -1;
            }
        }
        else
        {
            ASSERT( 0 );
        }

        return Icon;         
    }

#endif // X_EDITOR

    //-------------------------------------------------------------------------

    virtual object* Create          ( void )
    {
        return new trigger_ex_object;
    }

    xbool m_bRenderSpatial;
    xbool m_bBreakOnError;
    xbool m_bRetryOnError;

} s_TriggerExObjectDesc;

//=========================================================================

const object_desc&  trigger_ex_object::GetTypeDesc( void ) const
{
    return s_TriggerExObjectDesc;
}

//=========================================================================

const object_desc&  trigger_ex_object::GetObjectType( void )
{
    return s_TriggerExObjectDesc;
}

//=========================================================================
// INTERNAL CLASSES
//=========================================================================
#ifdef X_EDITOR

trigger_ex_object::trigger_ex_selector::trigger_ex_selector( ) : 
m_ActionType( actions_ex_base:: TYPE_ACTION_OBJECT_ACTIVATION ),
m_Active( FALSE ), 
m_Parent( NULL )
{
}

//=============================================================================

void  trigger_ex_object::trigger_ex_selector::Init( trigger_ex_object* pParent )
{
    m_Parent = pParent;
}

//=============================================================================

void  trigger_ex_object::trigger_ex_selector::OnEnumProp ( prop_enum& rPropList )
{
    rPropList.PropEnumHeader  ( "ActionSelect",                  "Select the type of condition to add.", PROP_TYPE_HEADER);
     
    rPropList.PropEnumEnum    ( "ActionSelect\\General Actions",     actions_ex_base::m_ActionsGeneralEnum.BuildString(),       "Types of General actions available." ,PROP_TYPE_DONT_SAVE );
    rPropList.PropEnumEnum    ( "ActionSelect\\Logic Actions",       actions_ex_base::m_ActionsSpecificEnum.BuildString(),      "Types of Game Logic actions available." ,PROP_TYPE_DONT_SAVE );
    rPropList.PropEnumEnum    ( "ActionSelect\\Interface Actions",   actions_ex_base::m_ActionsInterfaceEnum.BuildString(),     "Types of Interface actions available." ,PROP_TYPE_DONT_SAVE );
    rPropList.PropEnumEnum    ( "ActionSelect\\Player Actions",      actions_ex_base::m_ActionsPlayerEnum.BuildString(),        "Types of Player actions available." ,PROP_TYPE_DONT_SAVE );
    rPropList.PropEnumEnum    ( "ActionSelect\\AI Actions",          actions_ex_base::m_ActionsAIEnum.BuildString(),            "Types of AI actions available." ,PROP_TYPE_DONT_SAVE );
    rPropList.PropEnumEnum    ( "ActionSelect\\Meta Actions",        actions_ex_base::m_ActionsMetaEnum.BuildString(),          "Types of Meta actions available." ,PROP_TYPE_DONT_SAVE );

    rPropList.PropEnumButton  ( "ActionSelect\\If Action",       "Adds a new action into the list.",  PROP_TYPE_MUST_ENUM );
    rPropList.PropEnumButton  ( "ActionSelect\\Else Action",     "Adds a new action into the list.",  PROP_TYPE_MUST_ENUM );
}

//=============================================================================

xbool trigger_ex_object::trigger_ex_selector::OnProperty ( prop_query& rPropQuery )
{
    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Selector to add conditions and actions
    /////////////////////////////////////////////////////////////////////////////////////////////////
    if( rPropQuery.IsVar( "ActionSelect\\If Action" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Add as If Action" );
        }
        else
        {
            ASSERT( m_Parent );
            if ( m_Parent->AddAction( m_ActionType, m_Parent->m_NumIfActions, FALSE ) )
            {
                m_Parent->m_NumIfActions++;
            }
        }
        
        return TRUE;
    }
    
    if( rPropQuery.IsVar( "ActionSelect\\Else Action" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Add as Else Action" );
        }
        else
        {
            ASSERT( m_Parent );
            if ( m_Parent->AddAction( m_ActionType, m_Parent->m_NumElseActions, TRUE ) )
            {
                m_Parent->m_NumElseActions++;
            }
        }
        
        return TRUE;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Actions
    /////////////////////////////////////////////////////////////////////////////////////////////////
    
    if ( rPropQuery.IsVar( "ActionSelect\\General Actions"   )     ||
         rPropQuery.IsVar( "ActionSelect\\Logic Actions"     )     ||
         rPropQuery.IsVar( "ActionSelect\\Interface Actions" )     ||
         rPropQuery.IsVar( "ActionSelect\\Player Actions"    )     ||
         rPropQuery.IsVar( "ActionSelect\\AI Actions"        )     ||
         rPropQuery.IsVar( "ActionSelect\\Meta Actions"      )
         )
    {
        if( rPropQuery.IsRead() )
        {
            if ( actions_ex_base::m_ActionsAllEnum.DoesValueExist( m_ActionType ) )
            {
                rPropQuery.SetVarEnum( actions_ex_base::m_ActionsAllEnum.GetString( m_ActionType ) );
            }
            else
            {
                rPropQuery.SetVarEnum( "INVALID" );
            } 
        }
        else
        {
            actions_ex_base::action_ex_types ActionType;

            if( actions_ex_base::m_ActionsAllEnum.GetValue( rPropQuery.GetVarEnum(), ActionType ) )
            {
                m_ActionType = ActionType;
            }
        }
        
        return( TRUE );
    }
   
    return FALSE;
}

//=============================================================================
//=============================================================================

trigger_ex_object::triggex_ex_copy_data::triggex_ex_copy_data( )
{
    m_ActionType = actions_ex_base::INVALID_ACTION_TYPES;
}

//=============================================================================

void trigger_ex_object::triggex_ex_copy_data::Copy( actions_ex_base* pAction )
{
    if (pAction)
    {
        m_Properties.Clear();
        m_ActionType = pAction->GetType();
        pAction->OnCopy(m_Properties);
    }
}

//=============================================================================

void trigger_ex_object::triggex_ex_copy_data::Paste( trigger_ex_object* pTrigger, xbool bAsIfAction, s32 iIndex )
{
    if (pTrigger)
    {
        if (bAsIfAction)
        {
            //pasting if action
            s32 iAction = pTrigger->m_NumIfActions;
            if ( pTrigger->AddAction( m_ActionType, iAction, FALSE ) )
            {
                pTrigger->m_NumIfActions++;

                pTrigger->m_IfActions[iAction]->OnPaste(m_Properties);
                pTrigger->m_IfActions[iAction]->SetElse(FALSE);
                pTrigger->SetActionIndex(pTrigger->m_IfActions[iAction], iIndex);
            }        
        }
        else
        {
            //pasting else action
            s32 iAction = pTrigger->m_NumElseActions;
            if ( pTrigger->AddAction( m_ActionType, iAction, TRUE ) )
            {
                pTrigger->m_NumElseActions++;

                pTrigger->m_ElseActions[iAction]->OnPaste(m_Properties);
                pTrigger->m_ElseActions[iAction]->SetElse(TRUE);
                pTrigger->SetActionIndex(pTrigger->m_ElseActions[iAction], iIndex);
            }   
        }
    }
}

//=========================================================================

xbool trigger_ex_object::triggex_ex_copy_data::HasActionToPaste(void)
{
    return (m_ActionType != actions_ex_base::INVALID_ACTION_TYPES);
}

#endif //X_EDITOR

//=========================================================================
// TRIGGER_OBJECT
//=========================================================================

trigger_ex_object::trigger_ex_object(void) :
m_DrawActivationIcon(0),
m_DrawError(FALSE),
m_CurrentColor(s_TriggerColor_Sleep),
m_ConditionAffecter(TRUE, 0),
m_TriggerType(TRIGGER_TYPE_SIMPLE),
m_UpdateRate(1.0f),
m_RecoveryRate(0.0f),
m_DelayRate(0.0f),
m_NumIfActions(0),
m_NumElseActions(0),
m_Behavior(TRIGGER_OCCURS_ONCE),
m_ResetType(TRIGGER_RESET_DESTROY),
m_OnActivate(TRUE),
m_RepeatCount(-1),
m_ActivateCount(0),
m_bTriggerFired(FALSE),
m_CurrentActionStatus(ACTION_ONHOLD),
m_iCurrentActionIndex(-1),
m_IsSpatialActivated(FALSE),
m_ActivationType(SPATIAL_ACTIVATE_ON_PLAYER),
m_ActivationBulletType( SPATIAL_ACTIVATE_BULLET_TYPES_ALL ),
m_SpatialType(SPATIAL_TYPE_AXIS_CUBE),
m_ResponseType(SPATIAL_RESPONSE_RETURN_TO_SLEEP),
m_ActorGuidVarName(-1),
m_ActorGuidVarHandle(HNULL),
m_bRequiresLineOfSight(FALSE),
m_State(STATE_SLEEPING),
m_NextUpdateTime(0.0f),
m_Next(NULL_GUID),            
m_Prev(NULL_GUID),
m_TriggerSlot(-1),
m_EnteringDelay(TRUE),
m_EnteringRecovery(TRUE)
{
#ifdef  X_EDITOR
    m_Selector.Init( this );
    m_iPasteIndex  = 0;
#endif // X_EDITOR
    
    s32 i=0;
    for( i=0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
    {
        m_IfActions[i]    = NULL;
        m_ElseActions[i]    = NULL;
    }

    //spatial dimensions
    for ( i = 0; i < MAX_SPATIAL_DIMENSIONS_PARAMS; i++)
          m_Dimensions[i] = 400.0f;   
    
#ifdef X_EDITOR
   // m_Name.Set("<Trigger>");
    m_Description.Set("<None>");
#endif // X_EDITOR

    m_bLOSVerified = FALSE;
}

//=========================================================================

trigger_ex_object::~trigger_ex_object(void)
{
    for( s32 i=0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
    { 
        if (m_IfActions[i] != NULL)
        {
            delete m_IfActions[i];
            m_IfActions[i] = NULL;
        }

        if (m_ElseActions[i] != NULL)
        {
            delete m_ElseActions[i];
            m_ElseActions[i] = NULL;
        }
    }
}

//=========================================================================

void trigger_ex_object::OnInit( void )
{
    object::OnInit();
    
    //we must have this flag in the descriptor to ever get logic for objects of this type
    //but we need to remove it immediately, we don't want logic until this trigger fires
    SetAttrBits( GetAttrBits() &  ~object::ATTR_NEEDS_LOGIC_TIME );

    //Register myself too the global Trigger_Manager object...
    g_TriggerExMgr.RegisterTrigger( *this );
    
    PreUpdateState();

#ifndef CONFIG_RETAIL
    xtLastTriggerExeTime = g_ObjMgr.GetGameTime();
#endif
}

//=========================================================================

void trigger_ex_object::PreUpdateState( void )
{
    //set this to its default..
    switch (m_TriggerType)
    {
        case TRIGGER_TYPE_SIMPLE:
            //Set the approritate state
            SetTriggerState(STATE_CHECKING);
            //Set the activation type
            m_ActivationType = SPATIAL_ACTIVATE_ON_PLAYER;
            break;
        case TRIGGER_TYPE_SPATIAL:
            //Set the approritate state
            SetTriggerState(STATE_SLEEPING);
            //Set the activation type
            m_ActivationType = SPATIAL_ACTIVATE_ON_PLAYER;
            break;
        case TRIGGER_TYPE_VIEWABLE:
            //Set the approritate state
            SetTriggerState(STATE_SLEEPING);
            //Set the activation type
            m_ActivationType = SPATIAL_ACTIVATE_ON_INTERNAL;
            break;
    }

    m_IsSpatialActivated = FALSE;
}

//=========================================================================

void trigger_ex_object::OnKill( void )
{
    object::OnKill();
    
    //Unregister myself too the global Trigger_Manager object...
    g_TriggerExMgr.UnregisterTrigger( *this );
}

//=========================================================================

void trigger_ex_object::ReleaseBlocking()
{
    //ASSERT( m_iCurrentActionIndex != -1 );
    if ( m_iCurrentActionIndex == -1)
    {
        return;
    }

    if( m_CurrentActionStatus == ACTION_RUN_IF )
    {
        if( m_IfActions[m_iCurrentActionIndex] && m_IfActions[m_iCurrentActionIndex]->IsKindOf(action_ai_base::GetRTTI()) )
        {   
            ((action_ai_base*)m_IfActions[m_iCurrentActionIndex])->ReleaseBlocking();
        }
    }
    else if (m_CurrentActionStatus == ACTION_RUN_ELSE )
    {
        if( m_ElseActions[m_iCurrentActionIndex] && m_ElseActions[m_iCurrentActionIndex]->IsKindOf(action_ai_base::GetRTTI()) )
        {   
            ((action_ai_base*)m_ElseActions[m_iCurrentActionIndex])->ReleaseBlocking();
        }
    }
}

//=========================================================================
bbox trigger_ex_object::GetLocalBBox( void ) const 
{
    if (m_TriggerType == TRIGGER_TYPE_SIMPLE)
    {
        return s_EditSphere.GetBBox(); 
    }
    else //spatial
    {
        bbox BBox;

        switch ( m_SpatialType )
        {
        case SPATIAL_TYPE_AXIS_CUBE:       
            {  
                f32 HalfWidth  = m_Dimensions[0]/2;
                f32 HalfHeight = m_Dimensions[1]/2;
                f32 HalfLength = m_Dimensions[2]/2;
                vector3 Min(    -HalfWidth,    -HalfHeight, -HalfLength );
                vector3 Max(     HalfWidth,     HalfHeight,  HalfLength );
                BBox.Set( Min, Max );
            }
            break;

        case SPATIAL_TYPE_SPHERICAL:    
            {
                vector3 Min(    -m_Dimensions[0],    -m_Dimensions[0], -m_Dimensions[0] );
                vector3 Max(     m_Dimensions[0],     m_Dimensions[0],  m_Dimensions[0] );
                BBox.Set( Min, Max );
            }
            break;

        default:
            ASSERT(0);
            break;
        }

        return BBox; 
    }
}


//=========================================================================

void trigger_ex_object::OnRender( void )
{
    if (m_OnActivate && (m_TriggerType == TRIGGER_TYPE_VIEWABLE))
    {   
        if (g_game_running && g_first_person)
        {
            if (!m_IsSpatialActivated)
            {
                ActivateSpatialTrigger();            
            }
        }
    }
}

//=========================================================================

#ifndef X_RETAIL
void trigger_ex_object::OnDebugRender( void )
{
#if defined( X_EDITOR )
    if (g_GameLogicDebug) 
    {
        //standard trigger drawing   
        if( GetAttrBits() & ATTR_EDITOR_SELECTED )
        {
            OnRenderActions();
            m_ConditionAffecter.OnDebugRender();
        }
    }
#endif
}
#endif // X_RETAIL

//=========================================================================

#if !defined( CONFIG_RETAIL )

void trigger_ex_object::OnRenderSpatial( void )
{
    if (m_TriggerType != TRIGGER_TYPE_SIMPLE)
    {
        //spatial trigger info
        xcolor DrawColor = m_CurrentColor;
        if( GetAttrBits() & ATTR_EDITOR_SELECTED )
        {
            DrawColor = xcolor(255,0,0);
        }

        switch ( m_SpatialType )
        {
        case SPATIAL_TYPE_AXIS_CUBE:       
            {
                // Renders a volume given a BBox
                draw_Volume ( GetBBox(), xcolor(DrawColor.R, DrawColor.G,DrawColor.B, 50));
                draw_BBox   ( GetBBox(), DrawColor);  
            }
            break;

        case SPATIAL_TYPE_SPHERICAL:         
            draw_Sphere( object::GetPosition(), m_Dimensions[0], DrawColor );
            break;

        default:
            ASSERT(0);
            break;
        }
    }
}

#endif // !defined( CONFIG_RETAIL )

//=========================================================================

#ifndef X_RETAIL
void  trigger_ex_object::OnRenderActions ( void )
{
    for( s32 i=0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
    { 
        if (m_IfActions[i] != NULL)
        {
            m_IfActions[i]->OnDebugRender(i);
        }

        if (m_ElseActions[i] != NULL)
        {
            m_ElseActions[i]->OnDebugRender(i);
        }
    }
}
#endif // X_RETAIL

//=========================================================================

void trigger_ex_object::OnColCheck	( void )
{
    if( g_game_running && m_TriggerType == TRIGGER_TYPE_SPATIAL)
    {
        //Spatial Trigger Only
        switch ( m_SpatialType )
        {
        case SPATIAL_TYPE_AXIS_CUBE:       
            {
                g_CollisionMgr.StartApply( GetGuid() );
                g_CollisionMgr.ApplyAABBox( GetBBox() );
                g_CollisionMgr.EndApply();
            }
            break;

        case SPATIAL_TYPE_SPHERICAL:         
            {
                g_CollisionMgr.StartApply( GetGuid() );
                g_CollisionMgr.ApplySphere( GetPosition(), m_Dimensions[0] );
                g_CollisionMgr.EndApply();
            }
            break;
       
        default:
            ASSERT(0);
            break;
        }
    }

//    object::OnColCheck();
}

//=============================================================================

void trigger_ex_object::OnColNotify( object& Object )
{
    if (m_OnActivate && m_TriggerType == TRIGGER_TYPE_SPATIAL )
    {
        xbool bCollide = FALSE;

        //spatial trigger
        switch ( m_ActivationType )
        {
        case SPATIAL_ACTIVATE_ON_PLAYER:
            {
                if ( Object.IsKindOf( player::GetRTTI() ) == TRUE )
                {
                    bCollide = TRUE;
                }
            }     
            break;      
        case SPATIAL_ACTIVATE_ON_NPC:
            {
                if ( Object.IsKindOf( character::GetRTTI() ) == TRUE )
                {
                    bCollide = TRUE;
                }
            }       
            break;         
        case SPATIAL_ACTIVATE_ON_BULLET:
            {
                if ( Object.IsKindOf( base_projectile::GetRTTI() ) || Object.IsKindOf( net_proj::GetRTTI() ) )
                {
                    if( m_ActivationBulletType != SPATIAL_ACTIVATE_BULLET_TYPES_ALL )
                    {
                        switch( m_ActivationBulletType )
                        {
                            case SPATIAL_ACTIVATE_BULLET_TYPES_BULLET:
                            {
                                if( Object.GetTypeDesc ().GetType() == object::TYPE_BULLET_PROJECTILE )
                                    bCollide = TRUE;
                            }break;

                            case SPATIAL_ACTIVATE_BULLET_TYPES_MES_ALT_FIRE:
                            {
                                if( Object.GetTypeDesc ().GetType() == object::TYPE_MESONSEEKER_PROJECTILE )
                                    bCollide = TRUE;
                            }break;
                            
                            case SPATIAL_ACTIVATE_BULLET_TYPES_NPC_BULLET:
                                {
                                    if( Object.IsKindOf( base_projectile::GetRTTI() ))
                                    {
                                        base_projectile& projectile = base_projectile::GetSafeType(Object);
                                        object *parentObject = g_ObjMgr.GetObjectByGuid( projectile.GetOwnerID() );
                                        if( parentObject && parentObject->IsKindOf(character::GetRTTI()) )
                                        {
                                            bCollide = TRUE;
                                        }
                                    }
                                    else if( Object.IsKindOf( net_proj::GetRTTI() ))
                                    {
                                        net_proj& projectile = net_proj::GetSafeType(Object);
                                        object *parentObject = g_ObjMgr.GetObjectByGuid( projectile.GetOriginGuid() );
                                        if( parentObject && parentObject->IsKindOf(character::GetRTTI()) )
                                        {
                                            bCollide = TRUE;
                                        }
                                    }
                                }break;

                            case SPATIAL_ACTIVATE_BULLET_TYPES_PLAYER_BULLET:
                                {
                                    if( Object.IsKindOf( base_projectile::GetRTTI() ))
                                    {
                                        base_projectile& projectile = base_projectile::GetSafeType(Object);
                                        object *parentObject = g_ObjMgr.GetObjectByGuid( projectile.GetOwnerID() );
                                        if( parentObject && parentObject->IsKindOf(player::GetRTTI()) )
                                        {
                                            bCollide = TRUE;
                                        }
                                    }
                                    else if( Object.IsKindOf( net_proj::GetRTTI() ))
                                    {
                                        net_proj& projectile = net_proj::GetSafeType(Object);
                                        object *parentObject = g_ObjMgr.GetObjectByGuid( projectile.GetOriginGuid() );
                                        if( parentObject && parentObject->IsKindOf(player::GetRTTI()) )
                                        {
                                            bCollide = TRUE;
                                        }
                                    }
                                }break;

                            default:
                            {
                                bCollide = FALSE;
                            }break;
                        }             
                    }
                    else
                    {
                        // All bullet types collide with this trigger
                        bCollide = TRUE;
                    }
                }
            }     
            break;     
        case SPATIAL_ACTIVATE_ON_NPC_OR_PLAYER:
            {
               if ( Object.IsKindOf( character::GetRTTI() ) == TRUE  || 
                    Object.IsKindOf( player::GetRTTI() )    == TRUE )
                {
                    bCollide = TRUE;
                }
            }     
            break;   
        case SPATIAL_ACTIVATE_ON_INTERNAL:
            {
                //no-op, special case for viewable
            }
            break;
        default:
            ASSERT(0);
            break; break;
        } 

        if (bCollide)
        {
            ActivateSpatialTrigger();
            if (m_ActorGuidVarHandle.IsNonNull())
                g_VarMgr.SetGuid(m_ActorGuidVarHandle, Object.GetGuid());
        }
    }
    
    object::OnColNotify(Object);
}

//=============================================================================

void trigger_ex_object::ActivateSpatialTrigger ( void )
{  
    //the m_IsSpatialActivated flag is used to prevent multiple activation events to cause the trigger
    //to udpate faster than its natural update rate and also prevent multiple triggering of actions.
    if (m_IsSpatialActivated)
        return;

    ForceNextUpdate();
    m_CurrentColor = xcolor(255,0,0);
    SetTriggerState(STATE_CHECKING);
    m_IsSpatialActivated    = TRUE;
}

//=============================================================================

void trigger_ex_object::DeactivateSpatialTrigger ( void )
{
    if (!m_IsSpatialActivated)
        return;
    
    SetTriggerState(STATE_SLEEPING);
    m_IsSpatialActivated    = FALSE;
}

//=========================================================================

void trigger_ex_object::OnEnumProp( prop_enum&  rPropList )
{
    object::OnEnumProp( rPropList );
        
#ifdef X_EDITOR
    m_Selector.OnEnumProp( rPropList );
    m_ConditionAffecter.EnumPropSelector( rPropList );

    if (s_ActionCopyData.HasActionToPaste())
    {
        rPropList.PropEnumHeader  ( "PasteAction",                        "A Trigger Action has been copied to memory.", PROP_TYPE_HEADER);
        rPropList.PropEnumButton  ( "PasteAction\\Paste If Action",       "Pastes a new action into the list.",  PROP_TYPE_MUST_ENUM );
        rPropList.PropEnumButton  ( "PasteAction\\Paste Else Action",     "Pastes a new action into the list.",  PROP_TYPE_MUST_ENUM );
        rPropList.PropEnumInt     ( "PasteAction\\Paste Index",           "Field index to paste into", 0 );
    }

#endif // X_EDITOR

    rPropList.PropEnumHeader  ( "Trigger Object",                 "The base class for trigger objects.", PROP_TYPE_HEADER);
   
    EnumPropDynamic( rPropList );

#ifdef X_EDITOR
    rPropList.PropEnumString  ( "Trigger Object\\Description",    "User defined text description of what this Trigger does.", PROP_TYPE_DONT_EXPORT);
#endif // X_EDITOR
    
    rPropList.PropEnumEnum    ( "Trigger Object\\Type",           "SIMPLE\0SPATIAL\0VIEWABLE\0", "Type of this trigger. Simple fire when active, Spatial fire when active and collision occurs, Viewable fire when active and rendered.", PROP_TYPE_MUST_ENUM );
    rPropList.PropEnumBool    ( "Trigger Object\\Start Active",   "Does this trigger start active?", 0 );
    rPropList.PropEnumBool    ( "Trigger Object\\Is Active",      "Is this trigger currently active?", PROP_TYPE_EXPOSE | PROP_TYPE_READ_ONLY | PROP_TYPE_DONT_SHOW );
    rPropList.PropEnumBool    ( "Trigger Object\\Is Spatial Active",      "Is this spatial trigger currently active?", PROP_TYPE_EXPOSE | PROP_TYPE_READ_ONLY | PROP_TYPE_DONT_SHOW );

    if (m_TriggerType == TRIGGER_TYPE_SIMPLE)
    {
        rPropList.PropEnumFloat   ( "Trigger Object\\Update Rate",    "The rate at which this trigger check for valid conditions in seconds( if 0, then no update occurs.) Note that the first update will occur at a random time between 0 and the update rate.", 0 );
    }

    rPropList.PropEnumFloat   ( "Trigger Object\\Delay Time",     "The time at which a trigger waits before it executues its actions in seconds. For repeating it will wait before each pass through the actions.", 0 );

    if (m_TriggerType != TRIGGER_TYPE_VIEWABLE)
    {
        //these don't make sense for viewable triggers
        rPropList.PropEnumEnum( "Trigger Object\\Behavior",       "ONCE\0REPEATING\0", "Type defines the trigger activation behavior.( ONCE allows only 1 execution, REPEAT allows more than one.)", PROP_TYPE_MUST_ENUM );

        if (m_Behavior == TRIGGER_OCCURS_REPEATING)
        {
            rPropList.PropEnumInt  ( "Trigger Object\\Repeat Count",   "The number of times a repeating trigger can be activated, if -1 then this trigger will repeat indefinitely.", PROP_TYPE_MUST_ENUM );
            rPropList.PropEnumFloat( "Trigger Object\\Recovery Rate",  "The rate at which repeating triggers recover in seconds.", 0 );
        }
    }

    if (m_TriggerType == TRIGGER_TYPE_VIEWABLE)
    {
        rPropList.PropEnumBool( "Trigger Object\\Requires Line Of Sight",   "Does this trigger require a clear line of sight to activate?", 0 );
    }
 
    //if not an infinitely repeating trigger
    if (! ((m_Behavior == TRIGGER_OCCURS_REPEATING) && (m_RepeatCount == -1)) )
    {
        rPropList.PropEnumEnum    ( "Trigger Object\\Reset Style",    "DESTROY\0DEACTIVATE\0", "Once all actions have completed for the specific count desired, what does this trigger do, destory itself, or deactivate itself?", 0 );
    }

    //data intergrity must happen prior to the actual conditions and actions
    rPropList.PropEnumInt     ( "Trigger Object\\NumIfActions",     "", PROP_TYPE_DONT_SHOW );
    rPropList.PropEnumInt     ( "Trigger Object\\NumElseActions",   "", PROP_TYPE_DONT_SHOW );
    m_ConditionAffecter.EnumPropConditionData( rPropList );

    //enum actual conditions and actions
    m_ConditionAffecter.EnumPropConditions( rPropList );
    EnumPropActions ( rPropList );
    m_ConditionAffecter.EnumPropElseConditions( rPropList );
    EnumPropElseActions( rPropList );

#ifdef X_EDITOR
    if ( (m_CurrentActionStatus != ACTION_ONHOLD) && g_game_running)
    {
        //the game is running at the index is moving
        rPropList.PropEnumHeader  ( "Debug",                       "Debug Info for this trigger (displays while game is running).", PROP_TYPE_HEADER);
        rPropList.PropEnumString  ( "Debug\\ConditionalPath",      "Are we in the if set or the else set?", PROP_TYPE_DONT_SAVE | PROP_TYPE_READ_ONLY);
        rPropList.PropEnumInt     ( "Debug\\Current Action Index", "Current Action Index showing which action in this trigger is currently running.", PROP_TYPE_DONT_SAVE | PROP_TYPE_READ_ONLY);
    }
    else if ( (m_CurrentActionStatus == ACTION_ONHOLD) && g_game_running)
    {
        rPropList.PropEnumHeader ( "Debug",       "Debug Info for this trigger (displays while game is running).", PROP_TYPE_HEADER);
        rPropList.PropEnumButton ( "Debug\\Clear Conditions", "Clear all conditions on this trigger.", PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT | PROP_TYPE_MUST_ENUM);
    }
#endif // X_EDITOR

    if (m_TriggerType != TRIGGER_TYPE_SIMPLE)
    {
        EnumPropSpatial( rPropList );
    }
}

//===========================================================================

xbool trigger_ex_object::OnProperty( prop_query& rPropQuery )
{
    MEMORY_OWNER( "trigger_ex_object::OnProperty()" );

    SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION );
    
    if( object::OnProperty( rPropQuery ) )
        return TRUE;
    
    if( OnPropertyDynamic( rPropQuery ) )
        return TRUE;
    
#ifdef X_EDITOR
    if( m_Selector.OnProperty( rPropQuery ) )
        return TRUE;

    if( m_ConditionAffecter.OnPropertySelector( rPropQuery ) )
        return TRUE;

    //handle paste
    if ( rPropQuery.IsVar( "PasteAction" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if ( actions_ex_base::m_ActionsAllEnum.DoesValueExist( s_ActionCopyData.m_ActionType ) )
            {
                rPropQuery.SetVarEnum( actions_ex_base::m_ActionsAllEnum.GetString( s_ActionCopyData.m_ActionType ) );
            }
            else
            {
                rPropQuery.SetVarEnum( "INVALID" );
            } 
        }

        return TRUE;
    }

    if ( rPropQuery.VarInt("PasteAction\\Paste Index", m_iPasteIndex) )
    {
        return TRUE;
    }

    if( rPropQuery.IsVar( "PasteAction\\Paste If Action" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Paste as If Action" );
        }
        else
        {
            s_ActionCopyData.Paste(this, TRUE, m_iPasteIndex);
        }

        return TRUE;
    }

    if( rPropQuery.IsVar( "PasteAction\\Paste Else Action" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Paste as Else Action" );
        }
        else
        {
            s_ActionCopyData.Paste(this, FALSE, m_iPasteIndex);
        }

        return TRUE;
    }

    if ( rPropQuery.IsVar("Debug\\Current Action Index") )
    {
        if( rPropQuery.IsRead() )
        {
            //for display only
            rPropQuery.SetVarInt(m_iCurrentActionIndex);
        }
        return TRUE;
    }

    if ( rPropQuery.IsVar("Debug\\ConditionalPath") )
    {
        if( rPropQuery.IsRead() )
        {
            //for display only
            if ( m_CurrentActionStatus == ACTION_RUN_IF )
            {
                rPropQuery.SetVarString("IF Set",32);
            }
            else if ( m_CurrentActionStatus == ACTION_RUN_ELSE )
            {
                rPropQuery.SetVarString("ELSE Set",32);
            }
            else
            {
                rPropQuery.SetVarString("ERROR",32);
            }
        }
        return TRUE;
    }

    if (rPropQuery.IsVar("Debug\\Clear Conditions"))
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Clear Conditions" );
        }
        else
        {
            m_ConditionAffecter.RemoveAllConditions();
        }
        return TRUE;
    }

    if ( rPropQuery.IsVar("Trigger Object\\Name") )
    {
        if( !rPropQuery.IsRead() )
        {
            SetName(rPropQuery.GetVarString());
        }
        return TRUE;
    }

    if ( rPropQuery.IsVar("Trigger Object\\Description") )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarString(m_Description.Get(), m_Description.MaxLen());
        }
        else
        {
            m_Description.Set(rPropQuery.GetVarString());
        }
        return TRUE;
    }
#endif // X_EDITOR

    if ( rPropQuery.IsVar( "Trigger Object\\Type"  ) )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_TriggerType )
            {
            case TRIGGER_TYPE_SIMPLE:       rPropQuery.SetVarEnum( "SIMPLE" );      break;
            case TRIGGER_TYPE_SPATIAL:      rPropQuery.SetVarEnum( "SPATIAL" );     break;
            case TRIGGER_TYPE_VIEWABLE:     rPropQuery.SetVarEnum( "VIEWABLE" );    break;
            default:
                ASSERT(0);
                break;
            }
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            u32 Bits = GetAttrBits();
            
            if( x_stricmp( pString, "SIMPLE" )==0)      
            { 
                m_TriggerType = TRIGGER_TYPE_SIMPLE;

                //change bits for PS2 only
                Bits &= ~ATTR_SPACIAL_ENTRY;
                Bits &= ~ATTR_COLLIDABLE;
                Bits &= ~ATTR_COLLISION_PERMEABLE;
                Bits &= ~ATTR_BLOCKS_ALL_ACTORS;
                Bits &= ~ATTR_BLOCKS_ALL_PROJECTILES;
                Bits &= ~ATTR_RENDERABLE;

                m_ActivationType = SPATIAL_ACTIVATE_ON_PLAYER;
                m_SpatialType    = SPATIAL_TYPE_AXIS_CUBE;
            }
            else if( x_stricmp( pString, "SPATIAL" )==0)     
            { 
                m_TriggerType = TRIGGER_TYPE_SPATIAL;

                //change bits for PS2 only
                Bits |= ATTR_SPACIAL_ENTRY;
                Bits |= ATTR_COLLIDABLE;
                Bits |= ATTR_COLLISION_PERMEABLE;
                Bits |= ATTR_BLOCKS_ALL_ACTORS;
                Bits |= ATTR_BLOCKS_ALL_PROJECTILES;
                Bits &= ~ATTR_RENDERABLE;

                m_ActivationType = SPATIAL_ACTIVATE_ON_PLAYER;
                m_SpatialType    = SPATIAL_TYPE_AXIS_CUBE;
            }
            else if( x_stricmp( pString, "VIEWABLE" )==0)    
            { 
                m_TriggerType = TRIGGER_TYPE_VIEWABLE;

                //change bits for PS2 only
                Bits |= ATTR_SPACIAL_ENTRY;
                Bits &= ~ATTR_COLLIDABLE;
                Bits &= ~ATTR_COLLISION_PERMEABLE;
                Bits &= ~ATTR_BLOCKS_ALL_ACTORS;
                Bits &= ~ATTR_BLOCKS_ALL_PROJECTILES;
                Bits |= ATTR_RENDERABLE;

                m_ActivationType = SPATIAL_ACTIVATE_ON_INTERNAL;
                m_SpatialType    = SPATIAL_TYPE_AXIS_CUBE;
            }        
            
            //update attr bits
            SetAttrBits(Bits);
            OnSpacialUpdate();
            PreUpdateState();
        }
        
        return TRUE;
    }
    
    if ( rPropQuery.VarFloat( "Trigger Object\\Update Rate"  ,  m_UpdateRate ) )
    {
        //Ensures the triggers get updated out of sync from one another...
        //  This might cause problems with repeating runs of complex trigger systems, use the define to disable...

        m_NextUpdateTime = x_frand(0, m_UpdateRate);

        return TRUE;
    }
    
    if ( rPropQuery.VarFloat( "Trigger Object\\Delay Time"  ,       m_DelayRate ) )
        return TRUE;
    
    if ( rPropQuery.VarFloat( "Trigger Object\\Recovery Rate"  ,    m_RecoveryRate ) )
        return TRUE;

    if ( rPropQuery.VarInt( "Trigger Object\\Repeat Count"  ,       m_RepeatCount ) )
        return TRUE;

    if ( rPropQuery.VarInt( "Trigger Object\\NumIfActions"  ,       m_NumIfActions  ))
        return TRUE;
 
    if ( rPropQuery.VarInt( "Trigger Object\\NumElseActions"  ,     m_NumElseActions  ))
        return TRUE;

    if ( rPropQuery.VarBool( "Trigger Object\\Start Active"  ,      m_OnActivate  ))
    {
        if (!rPropQuery.IsRead() && g_game_running)
        {
            OnActivate(m_OnActivate);
        }
        return TRUE;
    }

    if ( rPropQuery.VarBool( "Trigger Object\\Requires Line Of Sight"  , m_bRequiresLineOfSight  ))
        return TRUE;
    
    if ( m_ConditionAffecter.OnPropertyConditionData( rPropQuery ) )
        return TRUE;

    if ( m_ConditionAffecter.OnPropertyConditions( rPropQuery ) )
        return TRUE;
  
    if ( OnPropertyActions( rPropQuery ) )
        return TRUE;

    if ( rPropQuery.IsVar( "Trigger Object\\Is Active" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarBool( m_OnActivate );
        }

        return TRUE;
    }

    if ( rPropQuery.IsVar( "Trigger Object\\Is Spatial Active" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarBool( m_IsSpatialActivated );
        }

        return TRUE;
    }

    if ( rPropQuery.IsVar( "Trigger Object\\Behavior"  ) )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Behavior )
            {
            case TRIGGER_OCCURS_ONCE:        rPropQuery.SetVarEnum( "ONCE" );      break;
            case TRIGGER_OCCURS_REPEATING:   rPropQuery.SetVarEnum( "REPEATING" ); break;
            default:
                ASSERT(0);
                break;
            }
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "ONCE" )==0)        { m_Behavior = TRIGGER_OCCURS_ONCE;}
            if( x_stricmp( pString, "REPEATING" )==0)   { m_Behavior = TRIGGER_OCCURS_REPEATING;}
            
        }
        
        return TRUE;
    }
    
    if ( rPropQuery.IsVar( "Trigger Object\\Reset Style"  ) )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_ResetType )
            {
            case TRIGGER_RESET_DESTROY:      rPropQuery.SetVarEnum( "DESTROY" );    break;
            case TRIGGER_RESET_DEACTIVATE:   rPropQuery.SetVarEnum( "DEACTIVATE" ); break;
            default:
                ASSERT(0);
                break;
            }
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "DESTROY" )==0)      { m_ResetType = TRIGGER_RESET_DESTROY;}
            if( x_stricmp( pString, "DEACTIVATE" )==0)   { m_ResetType = TRIGGER_RESET_DEACTIVATE;}
            
        }
        
        return TRUE;
    }

    if( OnPropertySpatial( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

#ifdef X_EDITOR

// Top level validation function - checks all actions and conditions
s32 trigger_ex_object::OnValidateActions( xstring& ActionType, s32 nActions, actions_ex_base* Actions[], xstring& ErrorMsg )
{
    s32   i;
    s32   nErrors = 0;

    // Check all actions
    for( i = 0 ; i < nActions ; i++ )
    {
        // Lookup action
        actions_ex_base* pAction = Actions[i];
        ASSERT( pAction );

        // Clear errors
        xstring ActionErrorMsg;
        s32     nActionErrors = 0;

        // ObjectRef0
        xstring          ObjectRef0;
        object_affecter* pObjectRef0 = pAction->GetObjectRef0( ObjectRef0 );
        nActionErrors += OnValidateObject( ObjectRef0, pObjectRef0, ActionErrorMsg );
        
        // ObjectRef1
        xstring          ObjectRef1;
        object_affecter* pObjectRef1 = pAction->GetObjectRef1( ObjectRef1 );
        nActionErrors += OnValidateObject( ObjectRef1, pObjectRef1, ActionErrorMsg );

        // AnimRef
        xstring          AnimRef;
        s32              AnimName = -1;
        s32*             pAnimGroupName = pAction->GetAnimRef( AnimRef, AnimName );
        nActionErrors += OnValidateAnim( AnimRef, pAnimGroupName, AnimName,  ActionErrorMsg );

        // SoundRef
        xstring          SoundRef;
        s32              SoundName = -1;
        rhandle<char>*   pSoundPackage = pAction->GetSoundRef( SoundRef, SoundName );
        nActionErrors += OnValidateSound ( SoundRef, pSoundPackage, SoundName, ActionErrorMsg );

        // GlobalRef
        xstring          GlobalRef;
        s32*             pGlobalName = pAction->GetGlobalRef( GlobalRef );
        nActionErrors += OnValidateGlobal  ( GlobalRef, pGlobalName, ActionErrorMsg );

        // PropertyRef
        xstring          PropertyRef;
        s32              PropertyType = -1;
        s32*             pPropertyName = pAction->GetPropertyRef( PropertyRef, PropertyType );
        nActionErrors += OnValidateProperty( PropertyRef, pObjectRef0, pPropertyName, PropertyType, ActionErrorMsg );

        // TemplateRef
        xstring          TemplateRef;
        s32*             pTemplateName = pAction->GetTemplateRef( TemplateRef );
        nActionErrors += OnValidateTemplate( TemplateRef, pTemplateName, ActionErrorMsg );

        // Guid0Ref
        xstring          Guid0Ref;
        guid*            pGuid0 = pAction->GetGuidRef0( Guid0Ref );
        nActionErrors += OnValidateObject( Guid0Ref, pGuid0, ActionErrorMsg );

        // Guid1Ref
        xstring          Guid1Ref;
        guid*            pGuid1 = pAction->GetGuidRef1( Guid1Ref );
        nActionErrors += OnValidateObject( Guid1Ref, pGuid1, ActionErrorMsg );
extern xbool     g_bAutoBuild;
        if( !g_bAutoBuild )
        {
            // Report errors for play 2d
            if( pAction->GetType() == actions_ex_base::TYPE_ACTION_PLAY_2D_SOUND )
            {
                action_play_2d_sound* p2dAction = (action_play_2d_sound*)pAction;
                if( p2dAction->GetStreamed() == -1 )
                {
                    nActionErrors++;
                    ActionErrorMsg += "Play2d sound is being used - please set the streamed varibale to a valid setting!\n";
                }
            }
        }

        // Any errors?
        if( nActionErrors )
        {
            // Setup actopm info
            xstring ActionNumber;
            ActionNumber.Format( "%d", i );

            // Add action error to list
            ErrorMsg += ActionType + "[" + ActionNumber + "] " + pAction->GetDescription() + "\n";
            ErrorMsg += ActionErrorMsg + "\n";
            nErrors  += nActionErrors;
        }
    }

    return nErrors;
}

//=============================================================================

// Top level validation function - checks all actions and conditions
s32 trigger_ex_object::OnValidateProperties( xstring& ErrorMsg )
{
    // Check base class for errors first
    s32 nErrors = object::OnValidateProperties( ErrorMsg );
    if( nErrors )
        return nErrors;

    // Validate actions
    nErrors += OnValidateActions( xstring( "IfAction" ),   m_NumIfActions,   m_IfActions,   ErrorMsg );
    nErrors += OnValidateActions( xstring( "ElseAction" ), m_NumElseActions, m_ElseActions, ErrorMsg );

    // Validate conditions
    nErrors += m_ConditionAffecter.OnValidateProperties( *this, ErrorMsg );

    return nErrors;
}
#endif

//=============================================================================

void trigger_ex_object::EnumPropSpatial ( prop_enum& rPropList )
{      
    rPropList.PropEnumHeader  ( "Spatial Trigger", "Select the type of condition to add.", PROP_TYPE_HEADER);

    if (m_TriggerType == TRIGGER_TYPE_SPATIAL)
    {
        rPropList.PropEnumEnum    ( "Spatial Trigger\\Type", 
                                    s_SpatialEnumTable.BuildString(), 
                                    "Types of spatial triggers.  AxisCube is a box or Sphere is Radius.",  PROP_TYPE_MUST_ENUM );

        rPropList.PropEnumEnum    ( "Spatial Trigger\\Activation Triggers", 
                                    s_ActivationEnumTable.BuildString(), 
                                    "Things which will activate this spatial trigger.",  PROP_TYPE_MUST_ENUM  );

        if( m_ActivationType == SPATIAL_ACTIVATE_ON_BULLET )
            rPropList.PropEnumEnum ("Spatial Trigger\\Activation Bullet Type", s_ActivationBulletTypeEnumTable.BuildString (), "Type of bullet activates this trigger", PROP_TYPE_MUST_ENUM );

        rPropList.PropEnumExternal( "Spatial Trigger\\ActivaterVar", "global\0global_guid", 
                                    "(Optional) Global Variable (type guid) where we can store the activator guid; leave blank to not store this info.", PROP_TYPE_MUST_ENUM );
        
        rPropList.PropEnumEnum    ( "Spatial Trigger\\Response Type", 
                                    s_ResponseEnumTable.BuildString(), 
                                    "How to deal with repeating triggers.",  PROP_TYPE_MUST_ENUM  );
    }
    
    switch ( m_SpatialType )
    {
    case SPATIAL_TYPE_AXIS_CUBE:       
        rPropList.PropEnumFloat   ( "Spatial Trigger\\Cube Width",    "The width of the cube.", 0 );
        rPropList.PropEnumFloat   ( "Spatial Trigger\\Cube Height",   "The height of the cube.", 0 );
        rPropList.PropEnumFloat   ( "Spatial Trigger\\Cube Length",   "The length of the cube.", 0 );
        break;

    case SPATIAL_TYPE_SPHERICAL:         
        rPropList.PropEnumFloat   ( "Spatial Trigger\\Sphere Radius",   "The radius of the sphere.", 0 );
        break;

    default:
        ASSERT(0);
        break;
    }
    
#ifdef X_EDITOR
    if ( (m_CurrentActionStatus == ACTION_ONHOLD) && g_game_running)
    {
        rPropList.PropEnumButton (   "Spatial Trigger\\Test", "Test Fire this spatial trigger as if something triggered it.", PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT);
    }
#endif // X_EDITOR
}

//=============================================================================

xbool trigger_ex_object::OnPropertySpatial ( prop_query& rPropQuery )
{   
    SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSLATION );
    
    //always check for a valid guid handle
    if ( m_ActorGuidVarName == -1)
    {
        m_ActorGuidVarHandle = HNULL;
    }
    else if ( !g_VarMgr.GetGuidHandle( g_StringMgr.GetString(m_ActorGuidVarName), &m_ActorGuidVarHandle ) )
    {
        m_ActorGuidVarHandle = HNULL;
    }

#ifdef X_EDITOR
    if (rPropQuery.IsVar("Spatial Trigger\\Test"))
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Activate" );
        }
        else
        {
            ActivateSpatialTrigger();
        }
        return TRUE;
    }
#endif // X_EDITOR

    if ( rPropQuery.IsVar  ( "Spatial Trigger\\ActivaterVar" ))
    {
        if( rPropQuery.IsRead() )
        {
            if ( m_ActorGuidVarName >= 0 )
                rPropQuery.SetVarExternal( g_StringMgr.GetString(m_ActorGuidVarName), 256 );
            else
                rPropQuery.SetVarExternal("", 256);
            return TRUE;
        }
        else
        {
            if (x_strlen(rPropQuery.GetVarExternal()) > 0)
            {
                m_ActorGuidVarName = g_StringMgr.Add( rPropQuery.GetVarExternal() );
                return TRUE;
            }
        }
    }

    if ( SMP_UTIL_IsEnumVar<trigger_ex_spatial_types,trigger_ex_spatial_types>
        (rPropQuery, "Spatial Trigger\\Type", 
        m_SpatialType, s_SpatialEnumTable ) )
        return TRUE;
    
    if ( SMP_UTIL_IsEnumVar<trigger_ex_spatial_activation_types,trigger_ex_spatial_activation_types>
        (rPropQuery, "Spatial Trigger\\Activation Triggers", 
        m_ActivationType, s_ActivationEnumTable ) )
    {
        // Be sure and mark trigger to collide with bullets
        if( m_ActivationType == trigger_ex_object::SPATIAL_ACTIVATE_ON_BULLET )
            SetAttrBits( GetAttrBits() | object::ATTR_BLOCKS_ALL_PROJECTILES );
        else
            SetAttrBits( GetAttrBits() & (~object::ATTR_BLOCKS_ALL_PROJECTILES) );

        return TRUE;
    }

    if( m_ActivationType == SPATIAL_ACTIVATE_ON_BULLET )
    {
        if( SMP_UTIL_IsEnumVar<trigger_ex_spatial_activation_bullet_types, trigger_ex_spatial_activation_bullet_types> 
            (rPropQuery, "Spatial Trigger\\Activation Bullet Type",
            m_ActivationBulletType, s_ActivationBulletTypeEnumTable ) )
        {
            return TRUE;
        }
    }
    
    if ( SMP_UTIL_IsEnumVar<trigger_ex_spatial_activation_response,trigger_ex_spatial_activation_response>
        (rPropQuery, "Spatial Trigger\\Response Type", 
        m_ResponseType, s_ResponseEnumTable ) )
        return TRUE;

    if ( rPropQuery.VarFloat   ( "Spatial Trigger\\Cube Width",             m_Dimensions[0] ) )
        return TRUE;

    if ( rPropQuery.VarFloat   ( "Spatial Trigger\\Cube Height",            m_Dimensions[1] ) )
        return TRUE;
     
    if ( rPropQuery.VarFloat   ( "Spatial Trigger\\Cube Length",            m_Dimensions[2] ) )
        return TRUE;

    if ( rPropQuery.VarFloat   ( "Spatial Trigger\\Sphere Radius",          m_Dimensions[0] ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

xbool   trigger_ex_object::IsAwake( void )
{   
    if (m_State == STATE_SLEEPING)
        return FALSE;
    
    return TRUE;
}

//=============================================================================
//used by derived classes who want to update at the natural rate of the trigger but not execute the logic..
    
xbool trigger_ex_object::CanUpdate( f32 DeltaTime )
{
    return CheckNextTime(DeltaTime);
}

//=============================================================================

xbool trigger_ex_object::CheckNextTime ( f32 DeltaTime )
{
    //if we can update make sure to not syncrhonize by carrying the difference over..
    m_NextUpdateTime -= DeltaTime;
 
    if ( m_NextUpdateTime > 0.0f )
        return FALSE;
  
    return TRUE;
}

//=============================================================================

void trigger_ex_object::UpdateNextTime ( f32 Time )
{
    m_NextUpdateTime += Time;
}

//=============================================================================

void trigger_ex_object::SetTriggerState( const trigger_ex_state State )
{
    //cannot come out of RESET state once we enter it...
    if ( m_State == STATE_RESETTING )
        return;

    //set the desired state
    m_State = State;

    //no update for 0 rate, set it to sleep mode.
    if (( m_TriggerType == TRIGGER_TYPE_SIMPLE ) && ( m_UpdateRate == 0.0f ))
    {
        //set us to sleeping.
        m_State = STATE_SLEEPING;
    }

    switch ( m_State )
    {
    case STATE_SLEEPING:      
        //tell the manager that were asleep so we dont get anymore updates..
        g_TriggerExMgr.TriggerSleep( *this );
        m_CurrentColor = s_TriggerColor_Sleep;
        break;
        
    case STATE_CHECKING:    
        //tell the manager that were awake so we get moved into the updating list.
        ForceNextUpdate();
        g_TriggerExMgr.TriggerAwake( *this );
        m_DrawActivationIcon = 0;
        m_CurrentColor = s_TriggerColor_Checking;
        break;
        
    case STATE_RECOVERY:
        m_DrawActivationIcon = 0;
        m_CurrentColor = s_TriggerColor_Recovering;
        break;
        
    case STATE_DELAYING:
        m_DrawActivationIcon = 0;
        m_CurrentColor = s_TriggerColor_Delaying;
        break;

    case STATE_RESETTING:
        m_DrawActivationIcon = 0;
        m_CurrentColor = s_TriggerColor_Checking;
        //no-op 
        break;
        
    default:
        ASSERT(0);
        break;
    }
}

//=============================================================================

void trigger_ex_object::OnMove( const vector3& NewPos   )
{
    LogicCheckOnActivate();
    object::OnMove(NewPos);
}

//=============================================================================

void trigger_ex_object::ResetTrigger( void )
{
    SetTriggerState(STATE_RESETTING);
}

//=============================================================================

void trigger_ex_object::KillTrigger( void )
{
    m_ResetType = TRIGGER_RESET_DESTROY;
    m_iCurrentActionIndex = -1;
    m_CurrentActionStatus = ACTION_ONHOLD;
    SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );    
    SetTriggerState(STATE_RESETTING);
}

//=============================================================================

void trigger_ex_object::ExecuteLogic( f32 DeltaTime )
{
#ifndef CONFIG_RETAIL
    xtLastTriggerExeTime = g_ObjMgr.GetGameTime();
#endif

    //don't do anything if we are not active
    if (!m_OnActivate)
        return;

    //if we are in logic loop, don't do anything
    if (m_iCurrentActionIndex != -1)
        return;

    if (m_TriggerType != TRIGGER_TYPE_SIMPLE && !m_IsSpatialActivated) 
    {
        //if spacial is not activated, make sure we go to sleep
        m_State = STATE_SLEEPING;
    }

    switch ( m_State )
    {
    case STATE_SLEEPING: 
        ExecuteSleeping( DeltaTime );
        break;

    case STATE_CHECKING:
        ExecuteChecking( DeltaTime );
        break;

    case STATE_RECOVERY:
        ExecuteRecovery( DeltaTime );
        break;
        
    case STATE_DELAYING:
        ExecuteDelaying( DeltaTime );
        break;

    case STATE_RESETTING: 
        if ( m_ResetType == TRIGGER_RESET_DESTROY )
        {
            //destroy the trigger
            g_ObjMgr.DestroyObject( GetGuid() );
        }
        else
        {
            //turn the trigger off
            m_State = STATE_SLEEPING;           //first we change to another temp state
            m_NextUpdateTime = 0.0f;            //reset time
            m_OnActivate = FALSE;               //make trigger inactivate
            m_ActivateCount = 0;                //reset repeat counter
            m_bTriggerFired = FALSE;
            m_DrawActivationIcon = 0;
            m_iCurrentActionIndex = -1;
            m_CurrentActionStatus = ACTION_ONHOLD;
            if (m_TriggerType != TRIGGER_TYPE_SIMPLE)
            {
                DeactivateSpatialTrigger();         //make sure the spatial trigger is not active
                SetTriggerState(STATE_SLEEPING);
            }
            else
            {
                SetTriggerState(STATE_CHECKING);    //go into checking state
            }
        }
        break;

    default:

        ASSERT(0);
        break;
    }

    //spatial trigger specific
    if (m_IsSpatialActivated && m_TriggerType == TRIGGER_TYPE_SPATIAL )
    {
        if ( GetTriggerState() == STATE_CHECKING )
        {
            switch(m_ResponseType)
            {
            case SPATIAL_RESPONSE_IF_STILL_INSIDE:
                {
                    //do check to see if the object were intrested in is still inside of our volume
                    switch ( m_ActivationType )
                    {
                    case SPATIAL_ACTIVATE_ON_PLAYER:
                        {
                            if (QueryPlayerInVolume() == FALSE)
                            {
                                DeactivateSpatialTrigger();
                            }
                        }     
                        break;        
                    case SPATIAL_ACTIVATE_ON_NPC:
                        {
                            if (QueryNpcInVolume() == FALSE)
                            {
                                DeactivateSpatialTrigger();
                            }
                        }       
                        break;  
                    case SPATIAL_ACTIVATE_ON_NPC_OR_PLAYER:
                        {
                            if (QueryNpcInVolume() == FALSE && QueryPlayerInVolume() == FALSE)
                            {
                                DeactivateSpatialTrigger();
                            }
                        }       
                        break;
                    case SPATIAL_ACTIVATE_ON_BULLET:
                        {
                            if (QueryBulletInVolume() == FALSE)
                            {
                                DeactivateSpatialTrigger();
                            }
                        }     
                        break;    
                    case SPATIAL_ACTIVATE_ON_INTERNAL:
                        {
                            //no-op, special case for viewable
                        }
                        break;
                    default:
                        ASSERT(0);
                        break; break;
                    } 
                }
                break;
                
            case SPATIAL_RESPONSE_RETURN_TO_SLEEP:
                //goes back to sleep after activating..
                DeactivateSpatialTrigger();
                break;
               
            default:
                ASSERT(0);
                break;
            }
        }
    }
} 

//=============================================================================
//This function checks the logic of spatial triggers activated by on voulme, once it
//awakes during an OnActivate call..

void trigger_ex_object::LogicCheckOnActivate ( void )
{      
    if (!m_OnActivate)
        return;
    
    //for spatial
    if (m_TriggerType != TRIGGER_TYPE_SIMPLE)
    {
        //do check to see if the object were intrested in is still inside of our volume
        switch ( m_ActivationType )
        {
        case SPATIAL_ACTIVATE_ON_PLAYER:
            {
                if (QueryPlayerInVolume() == TRUE)
                {
                    ActivateSpatialTrigger();   
                }
            }     
            break;        
        case SPATIAL_ACTIVATE_ON_NPC:
            {
                if (QueryNpcInVolume() == TRUE)
                {
                    ActivateSpatialTrigger();   
                }
            }       
            break;  
        case SPATIAL_ACTIVATE_ON_NPC_OR_PLAYER:
            {
                if (QueryNpcInVolume() == TRUE || QueryPlayerInVolume() == TRUE)
                {
                    ActivateSpatialTrigger();   
                }
            }       
            break;
        case SPATIAL_ACTIVATE_ON_BULLET:
            {
                if (QueryBulletInVolume() == TRUE)
                {
                    ActivateSpatialTrigger();   
                }
            }     
            break;  
        case SPATIAL_ACTIVATE_ON_INTERNAL:
            {
                //no-op, special case for viewable
            }
            break;           
        default:
            ASSERT(0);
            break; 
        } 
    }       
}

//=============================================================================

void trigger_ex_object::ForceNextUpdate ( void )
{
    m_NextUpdateTime = 0.0f;
}

//=============================================================================

void trigger_ex_object::ExecuteSleeping ( f32 DeltaTime )
{
    ( void ) DeltaTime;

    //Sleep state puts the trigger into an inactive mode
    g_TriggerExMgr.TriggerSleep( *this );
}

//=============================================================================

void trigger_ex_object::ExecuteChecking ( f32 DeltaTime )
{
    ( void ) DeltaTime;
    
    //Check state, checks all the conditions, if they meet the requeisite state flags then
    //run the actions.. Then using the type, determine what to do post activiation...

    if ( CheckNextTime(DeltaTime) == FALSE )
        return;

    //Evaluate the conditions, only update the time if we don't get a true evaluation because
    //we don't want to time penalize the next state...
    switch (m_ConditionAffecter.EvaluateConditions( GetGuid() ))
    {
    case conditional_affecter::EVAL_COMMIT_IF:
        if (GetActionCount() > 0)
        {
            m_CurrentActionStatus = ACTION_RUN_IF;
            SetTriggerState(STATE_DELAYING);
            return;
        }
        break;
    case conditional_affecter::EVAL_COMMIT_ELSE:
        if (GetElseActionCount() > 0)
        {
            m_CurrentActionStatus = ACTION_RUN_ELSE;
            SetTriggerState(STATE_DELAYING);
        }
        break;
    case conditional_affecter::EVAL_NOT_MET:
        m_CurrentActionStatus = ACTION_ONHOLD;
        break;
    }
    m_DrawActivationIcon = 1;

    if (m_TriggerType == TRIGGER_TYPE_SIMPLE)
    {
        UpdateNextTime( m_UpdateRate );
    }
    else
    {
        ForceNextUpdate();
    }
}

//=============================================================================

void trigger_ex_object::ExecuteRecovery ( f32 DeltaTime )
{  
    //Recovery state, either destroy the object or sets it into checking mode with an extended
    //wait time until the next valid update within checking state.

    ( void ) DeltaTime;

    switch(m_Behavior) 
    {
    case TRIGGER_OCCURS_ONCE:
        if ( m_ActivateCount > 0)
        {
           SetTriggerState(STATE_RESETTING);
        }
        else
        {
           SetTriggerState(STATE_CHECKING);
        }
        break;
    case TRIGGER_OCCURS_REPEATING:
        {
            if ( m_RepeatCount != -1 && ( m_ActivateCount >= m_RepeatCount ) )
            {
                SetTriggerState(STATE_RESETTING);
            }            
            else 
            {
                //Only add the recoveryrate once..
                if (m_EnteringRecovery)
                {
                    m_EnteringRecovery  = FALSE;
                    UpdateNextTime( m_RecoveryRate );
                }
            
                //Check if we can return to our checking state now...
                if ((!m_bQuitRecovery) && (CheckNextTime(DeltaTime)==FALSE))
                    return;
 
                //Reset flag
                m_EnteringRecovery = TRUE;
            
                //Set the next state
                SetTriggerState(STATE_CHECKING);
            }
        }
        break; 
    default:
        ASSERT(0);
        break;;
    }
}

//=============================================================================

void trigger_ex_object::ExecuteDelaying ( f32 DeltaTime )
{
    //Delay state, waits for a specificed amount of time before executing the action...
    if (m_EnteringDelay)
    {
        m_EnteringDelay  = FALSE;
        UpdateNextTime( m_DelayRate );
    }
    
    //Check if we can go onto our recovery state now...
    if (CheckNextTime(DeltaTime)==FALSE)
        return;
    
    m_bQuitRecovery = FALSE;

    //Execute all actions..
    if (m_CurrentActionStatus != ACTION_ONHOLD)
    {
        ExecuteAllActions();
    }
    else
    {
        //nothing to execute
        m_bQuitRecovery = TRUE;
    }
   
    //Reset this flag
    m_EnteringDelay = TRUE;
    
    //Set the next state
    SetTriggerState(STATE_RECOVERY);
        
    //Turn on the draw activation flag
    m_DrawActivationIcon = 2;
    
    return;
}

//=============================================================================

xbool trigger_ex_object::AddAction( actions_ex_base::action_ex_types ActionType, s32 Number, xbool bAsElse )
{
    if ( Number >= 0 && Number < MAX_ACTION_ARRAY_SIZE )
    {
        if (!bAsElse)
        {
            if (m_IfActions[Number])
            { 
                delete m_IfActions[Number];
                m_IfActions[Number] = NULL;
            }
    
            m_IfActions[Number] = actions_ex_base::CreateAction( ActionType , GetGuid() );
            ASSERT(m_IfActions[Number]);
            m_IfActions[Number]->SetElse(FALSE);
        }
        else
        {
            if (m_ElseActions[Number])
            { 
                delete m_ElseActions[Number];
                m_ElseActions[Number] = NULL;
            }
    
            m_ElseActions[Number] = actions_ex_base::CreateAction( ActionType , GetGuid() );
            ASSERT(m_ElseActions[Number]);
            m_ElseActions[Number]->SetElse(TRUE);
        }
        return TRUE;
    }
    else
    {
        x_try;
        x_throw("You can not add any more actions to this trigger!");
        x_catch_display;
        return FALSE;
    }
}

//=============================================================================

void trigger_ex_object::EnumPropActions ( prop_enum& rPropList )
{

    if ( GetActionCount() > 0 )
    {
        //we have at least 1 action
        rPropList.PropEnumHeader  ( "Do", "All the Actions for this Trigger.", PROP_TYPE_HEADER);

        for( s32 i=0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
        { 
            if ( m_IfActions[i] )
            {
                ASSERT(m_IfActions[i]->GetElse() == FALSE);

                rPropList.PropEnumString( xfs("Do\\Action[%d]", i) , 
                                            m_IfActions[i]->GetTypeInfo(), PROP_TYPE_HEADER );        
            
                s32 iHeader = rPropList.PushPath( xfs("Do\\Action[%d]\\", i) );        
            
                m_IfActions[i]->OnEnumProp(rPropList);
            
                rPropList.PopPath( iHeader );
            }
        }
    }
}

//=============================================================================

void trigger_ex_object::EnumPropElseActions     ( prop_enum& rPropList )
{
    if ( GetElseActionCount() > 0 )
    {
        //we have at least 1 else action
        sml_string HeaderName;
        if (m_ConditionAffecter.GetElseConditionCount() > 0)
        {
            HeaderName.Set("Then Do");
        }
        else
        {
            HeaderName.Set("Else Do");
        }
        rPropList.PropEnumHeader  ( HeaderName.Get(), "All the else Actions for this Trigger.", PROP_TYPE_HEADER);

        for( s32 i=0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
        { 
            if ( m_ElseActions[i] )
            {
                ASSERT(m_ElseActions[i]->GetElse() == TRUE);

                rPropList.PropEnumString( xfs("%s\\Else Action[%d]", HeaderName.Get(), i) , 
                    m_ElseActions[i]->GetTypeInfo(), PROP_TYPE_HEADER );        
            
                s32 iHeader = rPropList.PushPath( xfs("%s\\Else Action[%d]\\", HeaderName.Get(), i) );        
            
                m_ElseActions[i]->OnEnumProp(rPropList);
            
                rPropList.PopPath( iHeader );
            }
        }
    }
}

//=============================================================================

xbool trigger_ex_object::OnPropertyActions ( prop_query& rPropQuery )
{
    //if actions
    if( rPropQuery.IsSimilarPath( "Do\\Action" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(0);
        
        ASSERT( iIndex < MAX_ACTION_ARRAY_SIZE && iIndex >= 0 );
  
        if ( rPropQuery.IsVar( "Do\\Action[]" ) )
        {
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarString(m_IfActions[iIndex]->GetDescription(), MAX_STRING_LEN );
            }

            return TRUE;
        }

        if (m_IfActions[iIndex])
        {
            s32 iHeader = rPropQuery.PushPath( "Do\\Action[]\\" );        
            
            if( m_IfActions[iIndex]->OnProperty(rPropQuery) )
            {
                rPropQuery.PopPath( iHeader );
                return TRUE;
            }  
            
            rPropQuery.PopPath( iHeader );
        }
    }
    
    //else actions
    if( rPropQuery.IsSimilarPath( "Else Do\\Else Action" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(0);
        
        ASSERT( iIndex < MAX_ACTION_ARRAY_SIZE && iIndex >= 0 );
  
        if ( rPropQuery.IsVar( "Else Do\\Else Action[]" ) )
        {
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarString(m_ElseActions[iIndex]->GetDescription(), MAX_STRING_LEN );
            }

            return TRUE;
        }

        if (m_ElseActions[iIndex])
        {
            s32 iHeader = rPropQuery.PushPath( "Else Do\\Else Action[]\\" );        
            
            if( m_ElseActions[iIndex]->OnProperty(rPropQuery) )
            {
                rPropQuery.PopPath( iHeader );
                return TRUE;
            }  
            
            rPropQuery.PopPath( iHeader );
        }
    }

    if( rPropQuery.IsSimilarPath( "Then Do\\Else Action" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(0);
        
        ASSERT( iIndex < MAX_ACTION_ARRAY_SIZE && iIndex >= 0 );
  
        if ( rPropQuery.IsVar( "Then Do\\Else Action[]" ) )
        {
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarString(m_ElseActions[iIndex]->GetDescription(), MAX_STRING_LEN );
            }

            return TRUE;
        }

        if (m_ElseActions[iIndex])
        {
            s32 iHeader = rPropQuery.PushPath( "Then Do\\Else Action[]\\" );        
            
            if( m_ElseActions[iIndex]->OnProperty(rPropQuery) )
            {
                rPropQuery.PopPath( iHeader );
                return TRUE;
            }  
            
            rPropQuery.PopPath( iHeader );
        }
    }

    return FALSE;
}

//=============================================================================

xbool trigger_ex_object::IsFirstInActionSet( actions_ex_base* pAction )
{
    if (pAction)
    {
        if (!pAction->GetElse())
        {
            if (m_IfActions[0] == pAction)
            {
                return TRUE;
            }
        }
        else
        {
            if (m_ElseActions[0] == pAction)
            {
                return TRUE;
            }        
        }

        return FALSE;
    }

    ASSERT(FALSE);
    return FALSE;
}

//=============================================================================

s32 trigger_ex_object::GetActionIndex ( actions_ex_base* pAction )
{
    for( s32 i = 0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
    { 
        if (!pAction->GetElse())
        {
            if (m_IfActions[i] == pAction)
            {
                return i;
            }
        }
        else
        {
            if (m_ElseActions[i] == pAction)
            {
                return i;
            }
        }
    }
    
    return -1;
}

//=============================================================================

xbool trigger_ex_object::HasDialogLine( void )
{
    s32 i = 0;
    for( i = 0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
    { 
        if (!m_IfActions[i])
            break;

        if( m_IfActions[i]->GetType() == actions_ex_base::TYPE_ACTION_AI_DIALOG_LINE ) 
        {
            return TRUE;
        }
    }

    for( i = 0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
    { 
        if (!m_ElseActions[i])
            break;

        if( m_ElseActions[i]->GetType() == actions_ex_base::TYPE_ACTION_AI_DIALOG_LINE ) 
        {
            return TRUE;
        }
    }

    return FALSE;
}

//=============================================================================

xbool trigger_ex_object::SetTriggerActionIndexToLabel( const char* pName )
{
    ASSERT( m_iCurrentActionIndex != -1 );

    if (m_CurrentActionStatus == ACTION_RUN_IF)
    {
        for( s32 i = 0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
        { 
            if (!m_IfActions[i])
                break;

            if( m_IfActions[i]->IsKindOf( trigger_meta_label::GetRTTI() ) )
            {
                trigger_meta_label *pLabel = (trigger_meta_label*) m_IfActions[i];
                if (x_strcmp(pLabel->GetLabel(), pName) == 0)
                {
                    m_iCurrentActionIndex = i;
                    return TRUE;
                }
            }
        }
    }
    else if (m_CurrentActionStatus == ACTION_RUN_ELSE)
    {
        for( s32 i = 0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
        { 
            if (!m_ElseActions[i])
                break;

            if( m_ElseActions[i]->IsKindOf( trigger_meta_label::GetRTTI() ) )
            {
                trigger_meta_label *pLabel = (trigger_meta_label*) m_ElseActions[i];
                if (x_strcmp(pLabel->GetLabel(), pName) == 0)
                {
                    m_iCurrentActionIndex = i;
                    return TRUE;
                }
            }
        }
    }
    else 
    {
        ASSERT(FALSE);
    }

    return FALSE;
}

//=============================================================================

void trigger_ex_object::GetValidLabels( xbool bElseAction, char* pLabels )
{
    s32     MaxStrLen       = 255;
    s32     CurrentIndex    = 0;

    if (!bElseAction)
    {
        //if labels
        for( s32 i = 0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
        { 
            if (!m_IfActions[i])
                break;

            if( m_IfActions[i]->IsKindOf( trigger_meta_label::GetRTTI() ) )
            {
                trigger_meta_label *pLabel = (trigger_meta_label*) m_IfActions[i];
                if (x_strlen(pLabel->GetLabel()) > 0)
                {
                    if( CurrentIndex < MaxStrLen )
                    {
                        x_strcpy( (char*) &pLabels[CurrentIndex], pLabel->GetLabel() );
                        CurrentIndex += x_strlen(pLabel->GetLabel());
                        pLabels[CurrentIndex] = 0;
                        CurrentIndex++;
                    }
                }
            }
        }
    }
    else 
    {
        //else labels
        for( s32 i = 0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
        { 
            if (!m_ElseActions[i])
                break;

            if( m_ElseActions[i]->IsKindOf( trigger_meta_label::GetRTTI() ) )
            {
                trigger_meta_label *pLabel = (trigger_meta_label*) m_ElseActions[i];
                if (x_strlen(pLabel->GetLabel()) > 0)
                {
                    if( CurrentIndex < MaxStrLen )
                    {
                        x_strcpy( (char*) &pLabels[CurrentIndex], pLabel->GetLabel() );
                        CurrentIndex += x_strlen(pLabel->GetLabel());
                        pLabels[CurrentIndex] = 0;
                        CurrentIndex++;
                    }
                }
            }
        }
    }

    pLabels[CurrentIndex] = 0;
	pLabels[CurrentIndex+1] = 0;
}

//=============================================================================

void trigger_ex_object::SetActionIndex ( actions_ex_base* pAction, s32 Index )
{
    if (Index < 0 )
    {
        return;
    }

    if (!pAction)
        return;

    if (!pAction->GetElse())
    {
        //if the index is too high, set it to the highest used index
        if (Index >= m_NumIfActions)
            Index = m_NumIfActions - 1;

        for( s32 i = 0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
        { 
            if (m_IfActions[i] == pAction)
            {
                //found the right action, first shift the array to fill in the empty spot
                for ( i++; i < MAX_ACTION_ARRAY_SIZE; i++)
                {
                    m_IfActions[i-1] = m_IfActions[i];
                }
                m_IfActions[MAX_ACTION_ARRAY_SIZE-1] = NULL;
    
                //now clear out the desired index
                for ( i = MAX_ACTION_ARRAY_SIZE-1; i > Index; i--)
                {
                    m_IfActions[i] = m_IfActions[i-1];
                }
                m_IfActions[Index] = pAction;

                return;
            }
        }
    }
    else
    {
        //if the index is too high, set it to the highest used index
        if (Index >= m_NumElseActions)
            Index = m_NumElseActions - 1;

        for( s32 i = 0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
        { 
            if (m_ElseActions[i] == pAction)
            {
                //found the right action, first shift the array to fill in the empty spot
                for ( i++; i < MAX_ACTION_ARRAY_SIZE; i++)
                {
                    m_ElseActions[i-1] = m_ElseActions[i];
                }
                m_ElseActions[MAX_ACTION_ARRAY_SIZE-1] = NULL;
    
                //now clear out the desired index
                for ( i = MAX_ACTION_ARRAY_SIZE-1; i > Index; i--)
                {
                    m_ElseActions[i] = m_ElseActions[i-1];
                }
                m_ElseActions[Index] = pAction;

                return;
            }
        }
    }

    
    ASSERT(FALSE);
}

//=============================================================================

void  trigger_ex_object::RemoveAction ( actions_ex_base* pAction, xbool bAndDelete )
{
    s32 i=0;
    
    if (!pAction)
        return;

    if (!pAction->GetElse())
    {
        for( i = 0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
        { 
            if (m_IfActions[i] == pAction)
            {
                if (bAndDelete)
                    delete m_IfActions[i];

                m_IfActions[i] = NULL;
                break;
            }
        } 
    
        if ( i == MAX_ACTION_ARRAY_SIZE )
        {
            x_DebugMsg("trigger_ex_object::RemoveAction, Cannot find action in table.");
            ASSERT(0);
            return;
        }

        //Shift the array to remove the empty slot...
        for ( i++; i < MAX_ACTION_ARRAY_SIZE; i++)
        {
            m_IfActions[i-1] = m_IfActions[i];
        }

        m_IfActions[MAX_ACTION_ARRAY_SIZE-1] = NULL;
        m_NumIfActions--;
    }
    else
    {
        for( i = 0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
        { 
            if (m_ElseActions[i] == pAction)
            {
                if (bAndDelete)
                    delete m_ElseActions[i];
                
                m_ElseActions[i] = NULL;
                break;
            }
        } 
    
        if ( i == MAX_ACTION_ARRAY_SIZE )
        {
            x_DebugMsg("trigger_ex_object::RemoveAction, Cannot find action in table.");
            ASSERT(0);
            return;
        }

        //Shift the array to remove the empty slot...
        for ( i++; i < MAX_ACTION_ARRAY_SIZE; i++)
        {
            m_ElseActions[i-1] = m_ElseActions[i];
        }

        m_ElseActions[MAX_ACTION_ARRAY_SIZE-1] = NULL;
        m_NumElseActions--;
    }
}

//=============================================================================

xbool trigger_ex_object::CanSwitchElse( actions_ex_base* pAction )
{
    ASSERT(pAction);
    if ( pAction )
    {
        if ( !pAction->GetElse() )
        {
            if ( GetElseActionCount() < MAX_ACTION_ARRAY_SIZE )
            {
                return TRUE;
            }
        }
        else
        {
            if ( GetActionCount() < MAX_ACTION_ARRAY_SIZE )
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

//=============================================================================

void trigger_ex_object::SwitchElse( actions_ex_base* pAction )
{
    ASSERT(pAction);
    if ( pAction && CanSwitchElse(pAction) )
    {
        RemoveAction(pAction, FALSE);
        pAction->SetElse(!pAction->GetElse());

        if ( !pAction->GetElse() )
        {
            ASSERT (!m_IfActions[m_NumIfActions]);
            m_IfActions[m_NumIfActions] = pAction;
            m_NumIfActions++;
        }
        else
        {
            ASSERT (!m_ElseActions[m_NumElseActions]);
            m_ElseActions[m_NumElseActions] = pAction;
            m_NumElseActions++;
        }
    }
}

//=============================================================================

void trigger_ex_object::ExecuteAllActions ( void )
{
    m_iCurrentActionIndex = 0;
    m_bTriggerFired = FALSE;

    SetAttrBits( GetAttrBits() | object::ATTR_NEEDS_LOGIC_TIME );
}

//=============================================================================

void  trigger_ex_object::EnumPropDynamic ( prop_enum& rPropList )
{   
    for( s32 i=0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
    { 
        if (m_IfActions[i] != NULL)
        {
            rPropList.PropEnumInt( xfs("Trigger Object\\AIfType[%d]", i), "", PROP_TYPE_DONT_SHOW );
        }

        if (m_ElseActions[i] != NULL)
        {
            rPropList.PropEnumInt( xfs("Trigger Object\\AElseType[%d]", i), "", PROP_TYPE_DONT_SHOW );
        }
    }
}

//=============================================================================

xbool trigger_ex_object::OnPropertyDynamic ( prop_query& rPropQuery )
{
    if( rPropQuery.IsSimilarPath( "Trigger Object\\AIfType[" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(0);
        ASSERT( iIndex < MAX_ACTION_ARRAY_SIZE && iIndex >= 0 );

        if( m_IfActions[iIndex] != NULL && rPropQuery.IsRead() )
        {
            rPropQuery.SetVarInt( m_IfActions[iIndex]->GetType() );
        }
        else
        { 
            s32 ActionType = -1;
            ActionType = rPropQuery.GetVarInt();
            AddAction( (actions_ex_base::action_ex_types) ActionType, iIndex, FALSE );
        }
        
        return TRUE;
    }
    
    if( rPropQuery.IsSimilarPath( "Trigger Object\\AElseType[" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(0);
        ASSERT( iIndex < MAX_ACTION_ARRAY_SIZE && iIndex >= 0 );

        if( m_ElseActions[iIndex] != NULL && rPropQuery.IsRead() )
        {
            rPropQuery.SetVarInt( m_ElseActions[iIndex]->GetType() );
        }
        else
        { 
            s32 ActionType = -1;
            ActionType = rPropQuery.GetVarInt();
            AddAction( (actions_ex_base::action_ex_types) ActionType, iIndex, TRUE );
        }
        
        return TRUE;
    }

    return FALSE;
}           

//=============================================================================

void trigger_ex_object::OnActivate ( xbool Flag )
{
    m_OnActivate = Flag;    
    
    //give all trigger actions the chance to know that this trigger has activated
    for (s32 j = 0 ; j < MAX_ACTION_ARRAY_SIZE; j++ )
    {
        if (m_IfActions[j])
            m_IfActions[j]->OnActivate(Flag);

        if (m_ElseActions[j])
            m_ElseActions[j]->OnActivate(Flag);
    }

    if (Flag)
    {
        //work around for volume spatial checks, because its failing for entities already inside
        //the volume when the trigger is activated
        LogicCheckOnActivate();
    }
    else if (GetAttrBits() & object::ATTR_NEEDS_LOGIC_TIME)
    {
        //we are already active, and being deactivated
        m_bTriggerFired = FALSE;
        m_iCurrentActionIndex = -1;
        m_CurrentActionStatus = ACTION_ONHOLD;
        SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );
        m_DrawActivationIcon = 0;
        m_bQuitRecovery = TRUE;
    }
}

//=============================================================================
// SPACIAL QUERIES
//=============================================================================

xbool trigger_ex_object::QueryPlayerInVolume ( void )
{
    //Query for the player...  
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_PLAYER );
     
    xbool bVal = FALSE;    
    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        
        if( pObject != NULL )
            bVal |= QueryObjectInVolume(pObject);
        
        SlotID = g_ObjMgr.GetNext( SlotID );
    }
    
    return bVal;
}

//=============================================================================

xbool trigger_ex_object::QueryNpcInVolume ( void )
{
    xbool bVal = FALSE;

    for( u32 objectTypeCount = 0; objectTypeCount < TYPE_END_OF_LIST; objectTypeCount++)
    {
        slot_id SlotID = g_ObjMgr.GetFirst((object::type)objectTypeCount);

        if( SlotID != SLOT_NULL )
        {
            object* tempObject = g_ObjMgr.GetObjectBySlot(SlotID);

            if(tempObject->GetAttrBits() & object::ATTR_CHARACTER_OBJECT )
            {
                while(SlotID != SLOT_NULL)
                {
                    object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        
                    if( pObject != NULL )
                        bVal |= QueryObjectInVolume(pObject);
    
                    SlotID = g_ObjMgr.GetNext( SlotID );
                }
            }
        }
    }

    return bVal;
}

//=============================================================================

xbool trigger_ex_object::QueryBulletInVolume ( void )
{
    //Query for bullets in the volume...
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_BULLET_PROJECTILE );

    xbool bVal = FALSE;
    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        
        if( pObject != NULL )
            bVal |= QueryObjectInVolume(pObject);
    
        SlotID = g_ObjMgr.GetNext( SlotID );
    }
    
    return bVal;
}

//=============================================================================

xbool trigger_ex_object::QueryObjectInVolume( object* pObject )
{
    switch ( m_SpatialType )
    {
    case SPATIAL_TYPE_AXIS_CUBE:       
        {  
            bbox PlayerBox = pObject->GetBBox();
            bbox ObjectBox = GetBBox();
            
            if (ObjectBox.Intersect(PlayerBox))
                return TRUE;
            else
                return FALSE;
        }
        break;
        
    case SPATIAL_TYPE_SPHERICAL:    
        { 
            bbox PlayerBox = pObject->GetBBox();
            
            if (PlayerBox.Intersect(GetPosition(), m_Dimensions[0]))
                return TRUE;
            else
                return FALSE;
        }
        break;
        
    default:
        ASSERT(0);
        break;
    }
    
    return FALSE;
}

//=============================================================================

void trigger_ex_object::ForceStartTrigger( void )
{
    m_OnActivate            = TRUE;
    m_iCurrentActionIndex   = 0;
    m_CurrentActionStatus   = ACTION_RUN_IF;

    TurnAttrBitsOn( object::ATTR_NEEDS_LOGIC_TIME );
}

//=============================================================================

void trigger_ex_object::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "trigger_ex_object::OnAdvanceLogic" );

    (void)DeltaTime;
    g_TriggerAdvLogicCount++;

    // TODO:SH - Not sure where this needs to go.
    //   Probably inside the if (m_OnActivate) below, but I'm 
    //   not sure what action status it should fall under.
    //   Nick - I'm leaving it up to you to relocate it.
    if (m_bRequiresLineOfSight && !m_bLOSVerified)
    {
        xbool   bViewable   = FALSE;
    
        const view* pView = eng_GetView();

        g_CollisionMgr.LineOfSightSetup( GetGuid(), pView->GetPosition(), GetPosition() );
        // Only need one collision to say that we can't see the trigger
        g_CollisionMgr.SetMaxCollisions(1);

        // Perform collision
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_PLAYER_LOS, (object::object_attr)( object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING ) );
        if ( g_CollisionMgr.m_nCollisions == 0 )
        {
            bViewable = TRUE;
            m_bLOSVerified = TRUE;
        }

        if (!bViewable)
        {
            m_bTriggerFired = FALSE;
            m_iCurrentActionIndex = -1;
            m_CurrentActionStatus = ACTION_ONHOLD;
            SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );
            m_DrawActivationIcon = 0;
            m_bLOSVerified = FALSE;
            m_IsSpatialActivated = FALSE;            
            return;
        }
    }
    
    if (m_OnActivate)
    {
        if (m_CurrentActionStatus == ACTION_ONHOLD)
        {
            //we need a valid index???
            ASSERT(FALSE);
            return;
        }

        if ( m_CurrentActionStatus == ACTION_RUN_IF )
        {
            ASSERT( m_iCurrentActionIndex != -1 );

            //Execute all actions...
            for (s32 j = m_iCurrentActionIndex ; j < MAX_ACTION_ARRAY_SIZE; j++ )
            {
                if (m_IfActions[j] == NULL)
                    continue;
            
                ASSERT(m_IfActions[j]->GetElse() == FALSE);

                if ( !ExecuteIndividualAction(m_IfActions[j], j, DeltaTime) )
                {
                    return;
                }
                else
                {
                    m_bTriggerFired = TRUE;
                }
            }
        }
        else if ( m_CurrentActionStatus == ACTION_RUN_ELSE )
        {
            ASSERT( m_iCurrentActionIndex != -1 );

            //Execute all else actions...
            for (s32 j = m_iCurrentActionIndex ; j < MAX_ACTION_ARRAY_SIZE; j++ )
            {
                if (m_ElseActions[j] == NULL)
                    continue;
            
                ASSERT(m_ElseActions[j]->GetElse() == TRUE);

                if ( !ExecuteIndividualAction(m_ElseActions[j], j, DeltaTime) )
                {
                    return;
                }
                else
                {
                    m_bTriggerFired = TRUE;
                }
            }
        }
    }

    if (m_bTriggerFired)
    {
        m_ActivateCount++;
    }
    else
    {
        //nothing fired!
        m_bQuitRecovery = TRUE;
    }

    //all actions finished
    m_bTriggerFired = FALSE;
    m_iCurrentActionIndex = -1;
    m_CurrentActionStatus = ACTION_ONHOLD;
    SetAttrBits( GetAttrBits() & ~object::ATTR_NEEDS_LOGIC_TIME );
    m_DrawActivationIcon = 0;
    m_bLOSVerified = FALSE;
}

//===========================================================================
// return TRUE to continue, FALSE to end advance
#ifdef X_EDITOR
extern g_EditorBreakpoint;
#endif // X_EDITOR

xbool trigger_ex_object::ExecuteIndividualAction ( actions_ex_base* pAction, s32 ActionIndex, f32 DeltaTime )
{
    ASSERT(pAction);
    m_DrawError = FALSE; //reset the error draw

    pAction->ResetErrorRegister();
    if (pAction->Execute( DeltaTime ))
    {
        //true, iterate the index
        m_iCurrentActionIndex = ActionIndex+1;
        ASSERT( m_iCurrentActionIndex != -1 );
    }
    else
    {
        //false, leave index where it is
        if (pAction->WasExecuteAnError())
        {
            m_DrawError = TRUE;
#ifdef X_EDITOR
            if (((trigger_ex_object_desc&)GetTypeDesc()).m_bBreakOnError)
            {
                g_EditorBreakpoint = TRUE; //pause
            }
#endif // X_EDITOR
        }

        return FALSE;
    }

    if ( pAction->IsKindOf( trigger_meta_base::GetRTTI() ) )
    {
#ifdef TARGET_PS2
        if ( pAction->IsKindOf( trigger_meta_breakpoint::GetRTTI() ) )
        {
            //this is a breakpoint, don't stop on the PS2
            return TRUE;
        }
#endif

        if ( pAction->IsKindOf( trigger_meta_label::GetRTTI() ) )
        {
            //this is a label, this should not stop the advance
            return TRUE;
        }
        
        //stop execution every time we encounter a meta not handled above
        return FALSE;
    }

    return TRUE;

}

//===========================================================================

xbool trigger_ex_object::RetryOnError( void )
{
    //we only want to do re-tries in the editor
#ifdef X_EDITOR
    return (((trigger_ex_object_desc&)GetTypeDesc()).m_bRetryOnError);
#else // X_EDITOR
    return FALSE;
#endif // X_EDITOR
}

//===========================================================================
// PS2 Debug Data Functions
//===========================================================================

#ifndef CONFIG_RETAIL

void trigger_ex_object::DumpData( X_FILE* pFile )
{
    s32 Active = 0;
    if (m_OnActivate)
        Active = 1;

    s32 LogicFlag = 0;
    if (GetAttrBits() & object::ATTR_NEEDS_LOGIC_TIME)
        LogicFlag = 1;

    s32 GoodGuid = 0;
    object* pObject = g_ObjMgr.GetObjectByGuid(GetGuid());
    if (pObject && pObject == this)
        GoodGuid = 1;

    s32 TypeDescLogic = 0;
    if (GetTypeDesc().HasLogic())
        TypeDescLogic = 1;

    char CAS[5];
    char TYPE[5];
    char CST[4];
    if ( m_CurrentActionStatus == ACTION_RUN_IF )
    {
        x_strcpy(CAS, "IF  ");
    }
    else if ( m_CurrentActionStatus == ACTION_RUN_ELSE )
    {
        x_strcpy(CAS, "ELSE");
    }
    else
    {
        x_strcpy(CAS, "HOLD");
    }

    switch (m_TriggerType)
    {
    case TRIGGER_TYPE_SIMPLE:   x_strcpy(TYPE, "SMPL"); break;
    case TRIGGER_TYPE_SPATIAL:  x_strcpy(TYPE, "SPCL"); break;
    case TRIGGER_TYPE_VIEWABLE: x_strcpy(TYPE, "VWBL"); break;
    default:                    x_strcpy(TYPE, "????"); break;
    }

    switch (m_State)
    {
    case STATE_SLEEPING:    x_strcpy(CST,"Slp"); break;
    case STATE_CHECKING:    x_strcpy(CST,"Chk"); break;
    case STATE_DELAYING:    x_strcpy(CST,"Dly"); break;
    case STATE_RECOVERY:    x_strcpy(CST,"Rcy"); break;
    case STATE_RESETTING:   x_strcpy(CST,"Rst"); break;
    default:                x_strcpy(CST,"???"); break;
    }

    x_fprintf( pFile, "%1d %s desc%1d %08X:%08X G%1d ATTR_%1d %3d %s %s %3d %6f %g\n",
        Active,
        TYPE,
        TypeDescLogic,
        (u32)((GetGuid()>>32)&0xFFFFFFFF),
        (u32)((GetGuid()>>0 )&0xFFFFFFFF),
        GoodGuid,
        LogicFlag,
        m_ActivateCount, 
        CST, 
        CAS,
        m_iCurrentActionIndex,        
        m_NextUpdateTime,
        x_TicksToMs(xtLastTriggerExeTime));
}

/*
void trigger_ex_object::DumpData( text_out& DataDumpFile )
{
    DataDumpFile.AddGuid        ("Guid", GetGuid() );
    DataDumpFile.AddF32         ("NUT",  m_NextUpdateTime );

    if ( m_CurrentActionStatus == ACTION_RUN_IF )
    {
        DataDumpFile.AddString  ("CAS",  "IF" );
    }
    else if ( m_CurrentActionStatus == ACTION_RUN_ELSE )
    {
        DataDumpFile.AddString  ("CAS",  "ELSE" );
    }
    else
    {
        DataDumpFile.AddString  ("CAS",  "HOLD" );
    }
    DataDumpFile.AddS32         ("CAI",  m_iCurrentActionIndex );
    DataDumpFile.AddS32         ("ACT",  m_ActivateCount );

    switch (m_State)
    {
    case STATE_SLEEPING:    DataDumpFile.AddString  ("CST",  "Slp"); break;
    case STATE_CHECKING:    DataDumpFile.AddString  ("CST",  "Chk"); break;
    case STATE_DELAYING:    DataDumpFile.AddString  ("CST",  "Dly"); break;
    case STATE_RECOVERY:    DataDumpFile.AddString  ("CST",  "Rcy"); break;
    case STATE_RESETTING:   DataDumpFile.AddString  ("CST",  "Rst"); break;
    default:                DataDumpFile.AddString  ("CST",  "???"); break;
    }

    DataDumpFile.AddEndLine     ( );
}
*/

#endif CONFIG_RETAIL

//===========================================================================
// Editor Only Functions
//===========================================================================

#ifdef X_EDITOR
void trigger_ex_object::EditorPreGame( void )
{
    for( s32 i=0; i< MAX_ACTION_ARRAY_SIZE ; i++ )
    { 
        if (m_IfActions[i] != NULL)
        {
            m_IfActions[i]->EditorPreGame();
        }

        if (m_ElseActions[i] != NULL)
        {
            m_ElseActions[i]->EditorPreGame();
        }
    }
}

//===========================================================================

char* trigger_ex_object::GetTriggerTypeString( void )
{
    switch (m_TriggerType)
    {
    case TRIGGER_TYPE_SIMPLE:   return "Simple";
    case TRIGGER_TYPE_SPATIAL:  return "Spatial";
    case TRIGGER_TYPE_VIEWABLE: return "Viewable";
    default:                    return "Unknown";
    }
}

//===========================================================================

char* trigger_ex_object::GetTriggerStatusString( void )
{
    switch (m_State)
    {
    case STATE_SLEEPING:    return "Sleeping";
    case STATE_CHECKING:    return "Checking";
    case STATE_DELAYING:    return "Delaying";
    case STATE_RECOVERY:    return "Recovery";
    case STATE_RESETTING:   return "Resetting";
    default:                return "Unknown";
    }
}

//===========================================================================

char* trigger_ex_object::GetTriggerActionString( void )
{
    switch (m_CurrentActionStatus)
    {
    case ACTION_ONHOLD:    return "HOLD";
    case ACTION_RUN_IF:    return "IF";
    case ACTION_RUN_ELSE:  return "ELSE";
    default:               return "???";
    }
}

//===========================================================================

void trigger_ex_object::CopyAction( actions_ex_base* pAction )
{
    s_ActionCopyData.Copy(pAction);
}

#endif // X_EDITOR















