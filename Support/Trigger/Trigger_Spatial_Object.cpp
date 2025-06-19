//=========================================================================
//
//  Trigger_Object.cpp
//
//=========================================================================

//=========================================================================
//  INCLUDES
//=========================================================================

#include "Trigger\Trigger_Spatial_Object.hpp"
#include "Trigger\Trigger_Manager.hpp"
#include "Entropy.hpp"

#include "Render\Editor\editor_icons.hpp"
#include "Objects\BaseProjectile.hpp"
#include "Objects\Player.hpp"
#include "characters\character.hpp"

#include "..\MiscUtils\SimpleUtils.hpp"
#include "..\Support\Inputmgr\GamePad.hpp"

//=========================================================================
// STATIC DEFINITIONS AND CONSTS
//=========================================================================

static f32              s_InitDimValue  = 100.0f;
static f32              s_SphereRadius  = 100.0f;
static sphere           s_EditSphere(vector3(0,0,0), s_SphereRadius);
static xcolor           s_ColorAdd(0,0,0);

//=========================================================================
// Spatial type table

trigger_spatial_object::spatial_pair            trigger_spatial_object::s_SpatialPairTable[] = 
{
        spatial_pair("RAY",                     trigger_spatial_object::TYPE_RAY),
        spatial_pair("PLANE",                   trigger_spatial_object::TYPE_PLANE),
        //spatial_pair("CUBIC",                 trigger_spatial_object::TYPE_CUBIC), //UNIMPLEMENTED
        spatial_pair("AXIS_CUBE",               trigger_spatial_object::TYPE_AXIS_CUBE),
        spatial_pair("SPHERICAL",               trigger_spatial_object::TYPE_SPHERICAL),
      
        spatial_pair( k_EnumEndStringConst,     trigger_spatial_object::SPATIAL_TYPES_INVALID),  //**MUST BE LAST**//
};
trigger_spatial_object::spatial_table           trigger_spatial_object::s_SpatialEnumTable(s_SpatialPairTable);

//=========================================================================
// Activation type table

trigger_spatial_object::activation_pair         trigger_spatial_object::s_ActivationPairTable[] = 
{
        activation_pair("BY_OVERRIDE_TYPE",     trigger_spatial_object::ACTIVATE_ON_BY_OVERRIDE_TYPE),
        activation_pair("PLAYER",               trigger_spatial_object::ACTIVATE_ON_PLAYER),
        activation_pair("NPC",                  trigger_spatial_object::ACTIVATE_ON_NPC),
        activation_pair("BULLET",               trigger_spatial_object::ACTIVATE_ON_BULLET),
        activation_pair("NPC_OR_PLAYER",        trigger_spatial_object::ACTIVATE_ON_NPC_OR_PLAYER),
        activation_pair("ON_USE",               trigger_spatial_object::ACTIVATE_ON_USE_EVENT),

        activation_pair( k_EnumEndStringConst,  trigger_spatial_object::ACTIVATION_TYPES_INVALID),  //**MUST BE LAST**//
};
trigger_spatial_object::activation_table        trigger_spatial_object::s_ActivationEnumTable(s_ActivationPairTable);

//=========================================================================
// Tesponse type table

trigger_spatial_object::response_pair           trigger_spatial_object::s_ResponsePairTable[] = 
{
        response_pair("IF_STILL_INSIDE",        trigger_spatial_object::RESPONSE_IF_STILL_INSIDE),
        response_pair("RETURN_TO_SLEEP",        trigger_spatial_object::RESPONSE_RETURN_TO_SLEEP),
        response_pair("PERMANENT_ON",           trigger_spatial_object::RESPONSE_PERMANENT_ON),
        response_pair("PERMANENT_OFF",          trigger_spatial_object::RESPONSE_PERMANENT_OFF),
      
        response_pair( k_EnumEndStringConst,    trigger_spatial_object::ACTIVATION_RESPONSE_TYPE_INVALID),  //**MUST BE LAST**//
};
trigger_spatial_object::response_table         trigger_spatial_object::s_ResponseEnumTable(s_ResponsePairTable);

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct trigger_spatial_object_desc : public object_desc
{
    trigger_spatial_object_desc( void ) : object_desc( 
        object::TYPE_SPATIAL_TRIGGER, 
        "Spatial Trigger", 
        "SCRIPT",

        object::ATTR_SPACIAL_ENTRY		 |
        object::ATTR_COLLISION_PERMEABLE,

        FLAGS_IS_DYNAMIC            ) {}

    //-------------------------------------------------------------------------

    virtual object* Create          ( void )
    {
        return new trigger_spatial_object;
    }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        draw_Label( Object.GetPosition(), XCOLOR_RED, "<<OBSOLETE>>" );    
        return EDITOR_ICON_TRIGGER;
    }

#endif // X_EDITOR

} s_TriggerSpatialObjectDesc;

//=========================================================================

const object_desc&  trigger_spatial_object::GetTypeDesc( void ) const
{
    return s_TriggerSpatialObjectDesc;
}

//=========================================================================

const object_desc&  trigger_spatial_object::GetObjectType( void )
{
    return s_TriggerSpatialObjectDesc;
}

//=========================================================================

//=========================================================================
// TRIGGER_SPATIAL_OBJECT
//=========================================================================

trigger_spatial_object::trigger_spatial_object(void) : trigger_object() ,  
m_IsActivated(FALSE),
m_ActivationType(ACTIVATE_ON_PLAYER),
m_SpatialType(TYPE_AXIS_CUBE),
m_ResponseType(RESPONSE_RETURN_TO_SLEEP),
m_TriggerActor(NULL),
m_EventPollingStarted(FALSE),
m_HasPressed(FALSE)
{
    for (s32 i = 0; i < MAX_DIMENSIONS_PARAMS; i++)
          m_Dimensions[i] = s_InitDimValue;      
}

//=========================================================================

trigger_spatial_object::~trigger_spatial_object(void)
{}

//=========================================================================

bbox trigger_spatial_object::GetLocalBBox( void ) const 
{ 
    bbox BBox;

    switch ( m_SpatialType )
    {
    case TYPE_RAY:   
        {
            vector3 Min(0,0,0);
            vector3 Max(0,0,m_Dimensions[0]);

            BBox.Set( Min,  Max );
        }
        break;
        
    case TYPE_PLANE:    
        {
          
            f32 HalfWidth  = m_Dimensions[0]/2;
            f32 HalfHeight = m_Dimensions[1]/2;

            const f32 MIN_LENGTH = 10;
            
            vector3 Min(    -HalfWidth,    -HalfHeight, -MIN_LENGTH );
            vector3 Max(     HalfWidth,     HalfHeight,  MIN_LENGTH );
         
            BBox.Set( Min, Max );
        }
        
        break;
        
    case TYPE_CUBIC:       
        {  
            f32 HalfWidth  = m_Dimensions[0]/2;
            f32 HalfHeight = m_Dimensions[1]/2;
            f32 HalfLength = m_Dimensions[2]/2;

            vector3 Min(    -HalfWidth,    -HalfHeight, -HalfLength );
            vector3 Max(     HalfWidth,     HalfHeight,  HalfLength );
            
            BBox.Set( Min, Max );
        }
        
        break;
        
    case TYPE_AXIS_CUBE:       
        {  
            f32 HalfWidth  = m_Dimensions[0]/2;
            f32 HalfHeight = m_Dimensions[1]/2;
            f32 HalfLength = m_Dimensions[2]/2;
            
            vector3 Min(    -HalfWidth,    -HalfHeight, -HalfLength );
            vector3 Max(     HalfWidth,     HalfHeight,  HalfLength );
            
            BBox.Set( Min, Max );
        }
        
        break;

    case TYPE_SPHERICAL:    
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

#ifdef TARGET_PC
    vector3 size = BBox.GetSize();

    if ( size == vector3(0,0,0) )
        BBox = s_EditSphere.GetBBox(); 
#endif

    return BBox; 
}

//==============================================================================

#ifndef X_RETAIL
void trigger_spatial_object::OnDebugRender ( void )
{
    trigger_object::OnDebugRender();

    xcolor DrawColor = m_CurrentColor;
 
    if( GetAttrBits() & ATTR_EDITOR_SELECTED )
    {
        DrawColor = xcolor(255,0,0);
    }

    switch ( m_SpatialType )
    {
    case TYPE_RAY:   
        {
            vector3 vEndpoint(0,0,m_Dimensions[0]);
   
            const matrix4 M = GetL2W();
        
            vEndpoint = M*vEndpoint;
        
            draw_Line( object::GetPosition(), vEndpoint, DrawColor );  
        }
        break;

    case TYPE_PLANE:    
        {
            const s32 NumPoints = 6;

            vector3 vPoints[NumPoints];
        
            f32 HalfWidth  = m_Dimensions[0]/2;
            f32 HalfHeight = m_Dimensions[1]/2;

            vPoints[0] = vector3(    HalfWidth,     HalfHeight,0);
            vPoints[1] = vector3(    HalfWidth,    -HalfHeight,0);
            vPoints[2] = vector3(   -HalfWidth,    -HalfHeight,0);
            vPoints[3] = vector3(   -HalfWidth,     HalfHeight,0);
            vPoints[4] = vector3(    HalfWidth,     HalfHeight,0);
            vPoints[5] = vector3(   -HalfWidth,    -HalfHeight,0);

            const matrix4 M = GetL2W();
        
            M.Transform( vPoints, vPoints, NumPoints );
        
            // Renders a wire polygon
            SMP_UTIL_draw_Polygon( vPoints, NumPoints, DrawColor, TRUE );
        }

        break;

    case TYPE_CUBIC:       
        {
            vector3 Halves( m_Dimensions[0]/2, m_Dimensions[1]/2, m_Dimensions[2]/2 );
        
            const matrix4 M = GetL2W();
        
            // Renders a wire polygon
            SMP_UTIL_draw_Cube ( Halves, M, DrawColor );

            // Renders a volume given a BBox
            draw_Volume ( GetBBox(), DrawColor);
        }
     
         break;
     
    case TYPE_AXIS_CUBE:       
        {
            if( GetAttrBits() & ATTR_EDITOR_SELECTED )
            {
                // Renders a volume given a BBox
                draw_Volume ( GetBBox(), xcolor(DrawColor.R, DrawColor.G,DrawColor.B, 50));
            } 

            draw_BBox( GetBBox(), DrawColor);  
        }
    
        break;

    case TYPE_SPHERICAL:         
        draw_Sphere( object::GetPosition(), m_Dimensions[0], DrawColor );
        break;

    default:
        ASSERT(0);
        break;
    }
}
#endif // X_RETAIL

//=============================================================================

void trigger_spatial_object::OnInit ( void )
{
    trigger_object::OnInit();
    
    //starts in sleeping state...
    SetTriggerState(STATE_SLEEPING);

    //set this to its default..
    m_ActivationType = ACTIVATE_ON_PLAYER;
}

//=============================================================================

void trigger_spatial_object::OnKill ( void )
{
    trigger_object::OnKill();
}

//=============================================================================

void trigger_spatial_object::OnColCheck	( void )
{  
    switch ( m_SpatialType )
    {
    case TYPE_RAY:   
        { 
            vector3 vEndpoint(0,0,m_Dimensions[0]);
       
            const matrix4 M = GetL2W();
            
            vEndpoint = M*vEndpoint;

            g_CollisionMgr.StartApply( GetGuid() );
             
            //collision is facing senestivie so we use tris facing both sides..
            
            g_CollisionMgr.ApplyTriangle ( 
                GetPosition(),
                vEndpoint,
                vEndpoint );
          
            g_CollisionMgr.ApplyTriangle ( 
                vEndpoint,
                vEndpoint,
                GetPosition() );

            g_CollisionMgr.EndApply();
        }
        break;

    case TYPE_PLANE:    
        {
            const s32 NumPoints = 4;

            vector3 vPoints[NumPoints];
            
            f32 HalfWidth  = m_Dimensions[0]/2;
            f32 HalfHeight = m_Dimensions[1]/2;

            vPoints[0] = vector3(    HalfWidth,     HalfHeight,0);
            vPoints[1] = vector3(    HalfWidth,    -HalfHeight,0);
            vPoints[2] = vector3(   -HalfWidth,    -HalfHeight,0);
            vPoints[3] = vector3(   -HalfWidth,     HalfHeight,0);
        
            const matrix4 M = GetL2W();
            
            M.Transform( vPoints, vPoints, NumPoints );
      
            g_CollisionMgr.StartApply( GetGuid() );
            
            //collision is facing senestivie so we use tris facing both sides..
    
            g_CollisionMgr.ApplyTriangle ( 
                vPoints[0],
                vPoints[1],
                vPoints[2] );
           
             g_CollisionMgr.ApplyTriangle ( 
                vPoints[0],
                vPoints[2],
                vPoints[3] );

             g_CollisionMgr.ApplyTriangle ( 
                vPoints[2],
                vPoints[1],
                vPoints[0] );
           
             g_CollisionMgr.ApplyTriangle ( 
                vPoints[3],
                vPoints[2],
                vPoints[0] );

            g_CollisionMgr.EndApply();
        }

        break;

    case TYPE_CUBIC:       
        {
          //TODO
        }
         
         break;
         
    case TYPE_AXIS_CUBE:       
        {
            g_CollisionMgr.StartApply( GetGuid() );
            g_CollisionMgr.ApplyAABBox( GetBBox() );
            g_CollisionMgr.EndApply();
        }
        
        break;

    case TYPE_SPHERICAL:         
        {
            g_CollisionMgr.StartApply( GetGuid() );
            g_CollisionMgr.ApplySphere( GetPosition(),
                                        m_Dimensions[0] );
            g_CollisionMgr.EndApply();
        }
        
        break;

    default:
        ASSERT(0);
        break;
    }
    
     object::OnColCheck();
}

//=============================================================================

void trigger_spatial_object::OnColNotify( object& Object )
{
    if (m_OnActivate)
    {
        switch ( m_ActivationType )
        {
        case ACTIVATE_ON_PLAYER:
            {
                if ( Object.IsKindOf( player::GetRTTI() ) == TRUE )
                {
                    ActivateTrigger();
                    m_TriggerActor = Object.GetGuid();
                }
            }     
            break;      
        case ACTIVATE_ON_NPC:
            {
                if ( Object.IsKindOf( character::GetRTTI() ) == TRUE )
                {
                    ActivateTrigger();
                    m_TriggerActor = Object.GetGuid();
                }
            }       
            break;         
        case ACTIVATE_ON_BULLET:
            {
                if ( Object.IsKindOf( base_projectile::GetRTTI() ) == TRUE )
                {
                    ActivateTrigger();
                    m_TriggerActor = Object.GetGuid();
                }
            }     
            break;     
        case ACTIVATE_ON_NPC_OR_PLAYER:
            {
               if ( Object.IsKindOf( character::GetRTTI() ) == TRUE  || 
                    Object.IsKindOf( player::GetRTTI() )    == TRUE )
                {
                    ActivateTrigger();
                    m_TriggerActor = Object.GetGuid();
                }
            }     
            break;      
        case ACTIVATE_ON_USE_EVENT:
            {
                if ( Object.IsKindOf( player::GetRTTI() ) == TRUE )
                {
                    StartPlayerPolling();
                    m_TriggerActor = Object.GetGuid();
                }
            }
            break;
        case ACTIVATE_ON_BY_OVERRIDE_TYPE:
            {
                //no-op
            }
            break;
        default:
            ASSERT(0);
            break; break;
        } 
    }
    
    object::OnColNotify(Object);
}

//=============================================================================

void trigger_spatial_object::ActivateTrigger ( void )
{  
    //the m_IsActivated flag is used to prevent multiple activation events to cause the trigger
    //to udpate faster than its natural update rate and also prevent multiple triggering of actions.

    if (m_IsActivated)
        return;
    
    ForceNextUpdate();
    
    //ExecuteAllActions( PRE_ACTIVATE_ACTION_FLAG );

    m_CurrentColor = xcolor(255,0,0);
    
    SetTriggerState(STATE_CHECKING);
    
    m_IsActivated           = TRUE;
    m_EventPollingStarted   = FALSE;
    m_HasPressed            = FALSE;
}

//=============================================================================

void trigger_spatial_object::DeactivateTrigger ( void )
{
    if (!m_IsActivated)
        return;
    
    SetTriggerState(STATE_SLEEPING);
    
    //ExecuteAllActions( POST_ACTIVATE_ACTION_FLAG );

    m_IsActivated           = FALSE;
    m_EventPollingStarted   = FALSE;
    m_HasPressed            = FALSE;
}

//=============================================================================

void trigger_spatial_object::StartPlayerPolling ( void )
{  
    //the m_IsActivated flag is used to prevent multiple activation events to cause the trigger
    //to udpate faster than its natural update rate and also prevent multiple triggering of actions.

    if ( m_IsActivated && m_HasPressed == FALSE )
        return;
     
    //ExecuteAllActions( PRE_ACTIVATE_ACTION_FLAG );

    ForceNextUpdate();
    
    SetTriggerState(STATE_CHECKING);
    
    m_IsActivated           = TRUE;
    m_EventPollingStarted   = TRUE;
    m_HasPressed            = FALSE;
}

//=============================================================================

void trigger_spatial_object::ExecuteLogic ( f32 DeltaTime )
{    
   TRIGGER_CONTEXT("trigger_spatial_object::ExecuteLogic");

    //Logic for spatial triggers..
    // -Only activated once spatial conditions are meet
    // -Only allows for 1 execution loop before it executes its logic
    // -If the state isnt checking state after calling parents trigger_object::ExecuteLogic
    //  it waits for the state to return back to checking before it checks its logic..
    // -Its logic can be of type :
    //     -RESPONSE_IF_STILL_INSIDE : checks volume to see if object is still inside, turns off if not
    //     -RESPONSE_RETURN_TO_SLEEP : goes back to sleep
    //     -RESPONSE_PERMANENT_ON    : stays on forever after that
    //     -RESPONSE_PERMANENT_OFF   : destroys itself 
    
   //m_OnActivate controls whether the object is alive or not..
    if (!m_OnActivate)
        return;
    
    //Event polling uses it own logic loop overriding the default logic loop...
    if (m_EventPollingStarted)
    {
        if (ExecutePlayerPolling( DeltaTime )==FALSE)
            return;
    }

    trigger_object::ExecuteLogic( DeltaTime );
    
    if (m_IsActivated)
    {
        if ( GetTriggerState() == STATE_CHECKING )
        {
            switch(m_ResponseType)
            {
            case RESPONSE_IF_STILL_INSIDE:
                {
                    //do check to see if the object were intrested in is still inside of our volume
                    switch ( m_SpatialType )
                    {
                    case TYPE_RAY:case TYPE_PLANE: case TYPE_CUBIC://these types dont support this response type
                        {
                            DeactivateTrigger();
                        }
                        break;
                        
                    case TYPE_AXIS_CUBE:case TYPE_SPHERICAL:
                        {
                            switch ( m_ActivationType )
                            {
                            case ACTIVATE_ON_PLAYER:
                                {
                                    if (QueryPlayerInVolume() == FALSE)
                                    {
                                        DeactivateTrigger();
                                    }
                                }     
                                break;        
                            case ACTIVATE_ON_NPC:
                                {
                                    if (QueryNpcInVolume() == FALSE)
                                    {
                                        DeactivateTrigger();
                                    }
                                }       
                                break;  
                            case ACTIVATE_ON_NPC_OR_PLAYER:
                                {
                                    if (QueryNpcInVolume() == FALSE && QueryPlayerInVolume() == FALSE)
                                    {
                                        DeactivateTrigger();
                                    }
                                }       
                                break;
                            case ACTIVATE_ON_BULLET:
                                {
                                    if (QueryBulletInVolume() == FALSE)
                                    {
                                        DeactivateTrigger();
                                    }
                                }     
                                break;       
                            case ACTIVATE_ON_USE_EVENT:  
                                {
                                    //as long as the player is inside the volume we poll, the 
                                    //volume check is done within ExecutePlayerPolling, so we just
                                    //return to that state..
                                    
                                    if (QueryPlayerInVolume() == FALSE)
                                    {
                                        DeactivateTrigger();
                                    }
                                    else
                                    {
                                        StartPlayerPolling();
                                    }
                                }
                                break;
                            case ACTIVATE_ON_BY_OVERRIDE_TYPE:
                                {
                                    //no-op
                                }
                                break;
                            default:
                                ASSERT(0);
                                break; break;
                            } 
                        }
                        break;
                        
                    default:
                        ASSERT(0);
                        break;
                    }
                }
                break;
                
            case RESPONSE_RETURN_TO_SLEEP:
                //goes back to sleep after activating..
                DeactivateTrigger();
                break;
                
            case RESPONSE_PERMANENT_ON:

                if (m_ActivationType == ACTIVATE_ON_USE_EVENT)
                {
                    StartPlayerPolling();
                }

                break;
                
            case RESPONSE_PERMANENT_OFF:
                KillTrigger();
                break;
                
            default:
                ASSERT(0);
                break;
            }
        }
    }
}

//=============================================================================

xbool trigger_spatial_object::ExecutePlayerPolling ( f32 DeltaTime )
{
    //Player event polling uses it own logic loop overriding the default logic loop of ExecuteLogic
    //There are 3 ways we can leave the event polling state. 
    //  -player leaves the volume
    //  -player uses the use key..
    //  -we are using an invalid spatial type..
 
    //if the player has pressed the button no need to poll as our activation condition has been meet..
    if ( m_HasPressed )
        return TRUE;
   
    //update at our natural update rate...
    if ( CanUpdate(DeltaTime) == FALSE )
        return FALSE;

    switch ( m_SpatialType )
    {
    case TYPE_RAY:case TYPE_PLANE: case TYPE_CUBIC://these types dont support this state
        {
            DeactivateTrigger();
        }
        break;
        
    case TYPE_AXIS_CUBE:case TYPE_SPHERICAL:
        {
            if (QueryPlayerInVolume() == FALSE)
            {
                DeactivateTrigger();
            }
            else
            {
                //watch for use event here, if there is one activate the trigger
                if (CheckForUseEvent())
                {
                   m_HasPressed = TRUE;
                   ForceNextUpdate();
                   return TRUE;
                }
            }
            
        } 
        break;

    default:

        break;
    }

    return FALSE;
}

//=============================================================================
//This function checks the logic of spatial triggers activated by on voulme, once it
//awakes during an OnActivate call..

void trigger_spatial_object::LogicCheckOnActivate ( void )
{    
    TRIGGER_CONTEXT("trigger_spatial_object::ForceLogicCheckOnActivate");
    
    if (!m_OnActivate)
        return;
    
    
    switch(m_ResponseType)
    {
    case RESPONSE_IF_STILL_INSIDE: case RESPONSE_RETURN_TO_SLEEP: case RESPONSE_PERMANENT_ON:case RESPONSE_PERMANENT_OFF:
        {
            //do check to see if the object were intrested in is still inside of our volume
            switch ( m_SpatialType )
            {
            case TYPE_RAY:case TYPE_PLANE: case TYPE_CUBIC://these types dont support this response type
                //no-op
                break;
                
            case TYPE_AXIS_CUBE:case TYPE_SPHERICAL:
                {
                    switch ( m_ActivationType )
                    {
                    case ACTIVATE_ON_PLAYER:
                        {
                            if (QueryPlayerInVolume() == TRUE)
                            {
                                ActivateTrigger();   
                            }
                        }     
                        break;        
                    case ACTIVATE_ON_NPC:
                        {
                            if (QueryNpcInVolume() == TRUE)
                            {
                                ActivateTrigger();   
                            }
                        }       
                        break;  
                    case ACTIVATE_ON_NPC_OR_PLAYER:
                        {
                            if (QueryNpcInVolume() == TRUE || QueryPlayerInVolume() == TRUE)
                            {
                                ActivateTrigger();   
                            }
                        }       
                        break;
                    case ACTIVATE_ON_BULLET:
                        {
                            if (QueryBulletInVolume() == TRUE)
                            {
                                ActivateTrigger();   
                            }
                        }     
                        break;       
                    case ACTIVATE_ON_USE_EVENT:    
                        {
                            if (QueryPlayerInVolume() == TRUE)
                            {
                                StartPlayerPolling();   
                            }
                        }
                        break;
                    case ACTIVATE_ON_BY_OVERRIDE_TYPE:
                        {
                            //no-op
                        }
                        break;;
                    default:
                        ASSERT(0);
                        break; break;
                    } 
                }
                break;
                
            default:
                ASSERT(0);
                break;
            }
        }
        break;
        
    default:
        ASSERT(0);
        break;
    }
}

//=============================================================================

void  trigger_spatial_object::OnEnumProp ( prop_enum& rPropList )
{   
    trigger_object::OnEnumProp( rPropList );
    
    rPropList.AddHeader  ( "Spatial Trigger", "Select the type of condition to add.", PROP_TYPE_HEADER);

    rPropList.AddEnum    ( "Spatial Trigger\\Type", 
        s_SpatialEnumTable.BuildString(), 
        "Types of spatial triggers.",  PROP_TYPE_MUST_ENUM );
    
    rPropList.AddEnum    ( "Spatial Trigger\\Activation Triggers", 
        s_ActivationEnumTable.BuildString(), 
        "Things which will activate this spatial trigger.",  PROP_TYPE_MUST_ENUM  );
    
    rPropList.AddEnum    ( "Spatial Trigger\\Response Type", 
        s_ResponseEnumTable.BuildString(), 
        "Things which will activate this spatial trigger.",  PROP_TYPE_MUST_ENUM  );

    switch ( m_SpatialType )
    {
    case TYPE_RAY:   
        rPropList.AddFloat   ( "Spatial Trigger\\Ray Length",     "The length of the ray." );
        break;

    case TYPE_PLANE:    
        rPropList.AddFloat   ( "Spatial Trigger\\Plane Width",    "The width of the plane." );
        rPropList.AddFloat   ( "Spatial Trigger\\Plane Height",   "The height of the plane." );
        break;

    case TYPE_CUBIC:       
       rPropList.AddFloat   ( "Spatial Trigger\\Cube Width",    "The width of the cube." );
       rPropList.AddFloat   ( "Spatial Trigger\\Cube Height",   "The height of the cube." );
       rPropList.AddFloat   ( "Spatial Trigger\\Cube Length",   "The length of the cube." );
        break;

    case TYPE_AXIS_CUBE:       
       rPropList.AddFloat   ( "Spatial Trigger\\Cube Width",    "The width of the cube." );
       rPropList.AddFloat   ( "Spatial Trigger\\Cube Height",   "The height of the cube." );
       rPropList.AddFloat   ( "Spatial Trigger\\Cube Length",   "The length of the cube." );
        break;

    case TYPE_SPHERICAL:         
        rPropList.AddFloat   ( "Spatial Trigger\\Sphere Radius",   "The radius of the sphere." );
        break;

    default:
        ASSERT(0);
        break;
    }
}

//=============================================================================

xbool trigger_spatial_object::OnProperty ( prop_query& rPropQuery )
{   
    SetAttrBits( GetAttrBits() | FLAGS_DIRTY_TRANSLATION );
    
    if( trigger_object::OnProperty( rPropQuery ) )
        return TRUE;
    
    if ( SMP_UTIL_IsEnumVar<spatial_types,spatial_types>(rPropQuery, "Spatial Trigger\\Type", 
        m_SpatialType, s_SpatialEnumTable ) )
        return TRUE;
    
    if ( SMP_UTIL_IsEnumVar<activation_types,activation_types>(rPropQuery, "Spatial Trigger\\Activation Triggers", 
        m_ActivationType, s_ActivationEnumTable ) )
        return TRUE;

    if ( SMP_UTIL_IsEnumVar<activation_response_type,activation_response_type>(rPropQuery, "Spatial Trigger\\Response Type", 
        m_ResponseType, s_ResponseEnumTable ) )
        return TRUE;

    if ( rPropQuery.VarFloat   ( "Spatial Trigger\\Ray Length",             m_Dimensions[0] ) )
        return TRUE;
    
    if ( rPropQuery.VarFloat   ( "Spatial Trigger\\Plane Width",            m_Dimensions[0] ) )
        return TRUE;
     
    if ( rPropQuery.VarFloat   ( "Spatial Trigger\\Plane Height",           m_Dimensions[1] ) )
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
// ALL THE QUERY VOULME CALLS CURRENTLY USE SIMPLE BOUNDING VOULME CHECKS ....

xbool trigger_spatial_object::QueryPlayerInVolume ( void )
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

xbool trigger_spatial_object::QueryNpcInVolume ( void )
{
    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_HAZMAT );

    xbool bVal = FALSE;

    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        
        if( pObject != NULL )
            bVal |= QueryObjectInVolume(pObject);
    
        SlotID = g_ObjMgr.GetNext( SlotID );
    }
    
    SlotID = g_ObjMgr.GetFirst( object::TYPE_GRUNT );
    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        
        if( pObject != NULL )
            bVal |= QueryObjectInVolume(pObject);
    
        SlotID = g_ObjMgr.GetNext( SlotID );
    }

    SlotID = g_ObjMgr.GetFirst( object::TYPE_FRIENDLY_SCIENTIST );
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

xbool trigger_spatial_object::QueryBulletInVolume ( void )
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

xbool trigger_spatial_object::QueryObjectInVolume( object* pObject )
{
    switch ( m_SpatialType )
    {
    case TYPE_RAY: case TYPE_PLANE:   
        
        return FALSE;       //These types don't support voulme queries..
        
        break;
        
    case TYPE_CUBIC:       
        
        return FALSE;       //TODO
        
        break;
        
    case TYPE_AXIS_CUBE:       
        {  
            bbox PlayerBox = pObject->GetBBox();
            bbox ObjectBox = GetBBox();
            
            if (ObjectBox.Intersect(PlayerBox))
                return TRUE;
            else
                return FALSE;
        }
        
        break;
        
    case TYPE_SPHERICAL:    
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

xbool trigger_spatial_object::CheckForUseEvent( void )
{
    //This functions watches the player inputs and waits for a use event...
    
    
    object* pObj = g_ObjMgr.GetObjectByGuid( m_TriggerActor );
    
    if( pObj->GetType() == object::TYPE_PLAYER )
    {

        player& Player = player::GetSafeType( *pObj );
      
        if( g_IngamePad[ Player.GetActivePlayerPad() ].GetLogical( ingame_pad::ACTION_USE ).WasValue )
        {
            return TRUE;
        }
    }
    return FALSE;
}

//=============================================================================

void trigger_spatial_object::OnActivate( xbool Flag )
{ 
    trigger_object::OnActivate( Flag );
    
    //work around for volume spatial checks, becuase its failing for entites already inside
    //the volume when the trigger is activated

    if ( Flag == TRUE )
        LogicCheckOnActivate();
} 

//=============================================================================

void trigger_spatial_object::Setup(   
                                   activation_types            ActivateType, 
                                   spatial_types               SpatialType,
                                   activation_response_type    ResponseType,
                                   const f32*                  Dimensions )
{
    m_ActivationType    = ActivateType;           
    m_SpatialType       = SpatialType;              
    m_ResponseType      = ResponseType;
    
    Setup( Dimensions );
}

//=============================================================================

void trigger_spatial_object::Setup( const f32* Dimensions )
{
    m_Dimensions[0] = Dimensions[0];
    m_Dimensions[1] = Dimensions[1];
    m_Dimensions[2] = Dimensions[2];
}
