//==============================================================================
//
//  Trigger_Object.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Trigger_Object.hpp"
#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "Entropy.hpp"
#include "Render\Editor\editor_icons.hpp"

//=========================================================================
// GLOBALS
//=========================================================================

static f32              s_SphereRadius = 50.0f;
static sphere           s_EditSphere(vector3(0,0,0), s_SphereRadius);

static const xcolor     s_TriggerColor_Sleep        (150,150,150);
static const xcolor     s_TriggerColor_Checking     (0,255,0);
static const xcolor     s_TriggerColor_Recovering   (255,255,0);
static const xcolor     s_TriggerColor_Delaying     (255,0,255);
  
static const s32        MAX_BIT_SIZE_OF_FLAGS = 32;
static const s32        MAX_STRING_LEN = 255;


//=========================================================================
// INTERNAL CLASSES
//=========================================================================

trigger_object::trigger_selector::trigger_selector( ) : 
m_ConditionType( conditional_base::TYPE_CONDITION_PLAYER_HEALTH ),
m_ActionType( actions_base:: TYPE_ACTION_PLAY_SOUND ),
m_Active( FALSE ), 
m_Parent( NULL )
{}

//=============================================================================

void  trigger_object::trigger_selector::Init( trigger_object* pParent )
{
    m_Parent = pParent;
}

//=============================================================================

void  trigger_object::trigger_selector::OnEnumProp ( prop_enum& rPropList )
{
    rPropList.AddHeader  ( "Selector", "Select the type of condition to add.", PROP_TYPE_HEADER);
     
    rPropList.AddEnum    ( "Selector\\Condition Misc",        conditional_base::m_ConditionalMiscEnum.BuildString(),    "Types of Misc condtions available." ,0 );
    rPropList.AddEnum    ( "Selector\\Condition AI",          conditional_base::m_ConditionalAIEnum.BuildString(),      "Types of AI condtions available." ,0 );
    rPropList.AddEnum    ( "Selector\\Condition Player",      conditional_base::m_ConditionalPlayerEnum.BuildString(),  "Types of Player condtions available." ,0 );
   
    rPropList.AddButton  ( "Selector\\Add Condition",  "Adds a new Condition into the list.",  PROP_TYPE_MUST_ENUM );
    
    rPropList.AddEnum    ( "Selector\\Action Misc",        actions_base::m_ActionsMiscEnum.BuildString(),       "Types of Misc actions available." ,0 );
    rPropList.AddEnum    ( "Selector\\Action AI",          actions_base::m_ActionsAIEnum.BuildString(),         "Types of AI actions available." ,0 );
    rPropList.AddEnum    ( "Selector\\Action Player",      actions_base::m_ActionsPlayerEnum.BuildString(),     "Types of Player actions available." ,0 );
    rPropList.AddEnum    ( "Selector\\Action Variables",   actions_base::m_ActionsVariablesEnum.BuildString(),  "Types of Variables actions available." ,0 );
    rPropList.AddEnum    ( "Selector\\Action Door",        actions_base::m_ActionsDoorEnum.BuildString(),       "Types of door actions available." ,0 );
    rPropList.AddEnum    ( "Selector\\Action Factions",    actions_base::m_ActionsFactionsEnum.BuildString(),   "Types of faction actions available." ,0 );

    rPropList.AddButton  ( "Selector\\Add Action",  "Adds a new action into the list.",  PROP_TYPE_MUST_ENUM );

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    //Globals Variables Interface 
 /*
    rPropList.AddHeader  ( "Selector\\Global Variables", "Select the type of condition to add.", PROP_TYPE_HEADER );
    
    rPropList.AddString	 ( "Selector\\Global Variables\\New Variable Name", "Name of the New Global Variable.", PROP_TYPE_MUST_ENUM );

    rPropList.AddButton  ( "Selector\\Global Variables\\Add Int",     "Adds a new global interger.",    PROP_TYPE_MUST_ENUM );
    rPropList.AddButton  ( "Selector\\Global Variables\\Add Float",   "Adds a new global float.",       PROP_TYPE_MUST_ENUM );
    rPropList.AddButton  ( "Selector\\Global Variables\\Add Bool",    "Adds a new global bool.",        PROP_TYPE_MUST_ENUM );
    rPropList.AddButton  ( "Selector\\Global Variables\\Add Timer",   "Adds a new global timer..",      PROP_TYPE_MUST_ENUM );
   
    rPropList.AddHeader  ( "Selector\\Global Variables\\Variables",   "Select the type of condition to add.", PROP_TYPE_HEADER );
    
    {
        s32 iHeader = rPropList.PushPath( "Selector\\Global Variables\\Variables\\" );        
        
        g_VarMgr.OnEnumPropVariables( rPropList, 0 );
        
        rPropList.PopPath( iHeader );
    }
*/
}

//=============================================================================

xbool trigger_object::trigger_selector::OnProperty ( prop_query& rPropQuery )
{
    if( rPropQuery.IsVar( "Selector\\Add Condition" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Add Condition" );
        }
        else
        {
            ASSERT( m_Parent );
            m_Parent->AddCondition( m_ConditionType, m_Parent->m_NumConditons ); 
            m_Parent->m_NumConditons++;
        }
        
        return TRUE;
    }

    if( rPropQuery.IsVar( "Selector\\Add Action" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Add Action" );
        }
        else
        {
            ASSERT( m_Parent );
            m_Parent->AddAction( m_ActionType, m_Parent->m_NumActions ); 
            m_Parent->m_NumActions++;
        }
        
        return TRUE;
    }
/*    
    if( rPropQuery.IsVar( "Selector\\Global Variables\\Add Int" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Add Int" );
        }
        else
        {
            if (m_VariableName.IsEmpty())
            {
#ifdef TARGET_PC
                MessageBox( NULL, "No variable named defined.", "Warning",MB_ICONWARNING );
#endif
            }
            else
            {
                g_VarMgr.RegisterInt( m_VariableName.Get() );
            }
        }
        
        return TRUE;
    }
    
    if( rPropQuery.IsVar( "Selector\\Global Variables\\Add Float" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Add Float" );
        }
        else
        {
            if (m_VariableName.IsEmpty())
            {
#ifdef TARGET_PC
                MessageBox( NULL, "No variable named defined.", "Warning",MB_ICONWARNING  );
#endif
            }
            else
            {
                g_VarMgr.RegisterFloat( m_VariableName.Get() );
            }
        }
        
        return TRUE;
    }
    
    if( rPropQuery.IsVar( "Selector\\Global Variables\\Add Bool" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Add Bool" );
        }
        else
        {
            if (m_VariableName.IsEmpty())
            {
#ifdef TARGET_PC
                MessageBox( NULL, "No variable named defined.", "Warning",MB_ICONWARNING  );
#endif
            }
            else
            {
                g_VarMgr.RegisterBool( m_VariableName.Get() );
            }
        }
        
        return TRUE;
    }

    if( rPropQuery.IsVar( "Selector\\Global Variables\\Add Timer" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Add Timer" );
        }
        else
        {
            if (m_VariableName.IsEmpty())
            {
#ifdef TARGET_PC
                MessageBox( NULL, "No variable named defined.", "Warning",MB_ICONWARNING  );
#endif
            }
            else
            {
                g_VarMgr.RegisterTimer( m_VariableName.Get() );
            }
        }
        
        return TRUE;
    }

    if ( rPropQuery.VarString( "Selector\\Global Variables\\New Variable Name" , m_VariableName.Get(), m_VariableName.MaxLen() ) )
    {
       return TRUE; 
    }
*/  
  
   
    /////////////////////////////////////////////////////////////////////////////////////////////////
    
    if ( rPropQuery.IsVar( "Selector\\Action Misc"      )     ||
         rPropQuery.IsVar( "Selector\\Action AI"        )     ||
         rPropQuery.IsVar( "Selector\\Action Player"    )     ||
         rPropQuery.IsVar( "Selector\\Action Variables" )     ||
         rPropQuery.IsVar( "Selector\\Action Door"      )     ||
         rPropQuery.IsVar( "Selector\\Action Factions"  )
         )
    {
        if( rPropQuery.IsRead() )
        {
            if ( actions_base::m_ActionsAllEnum.DoesValueExist( m_ActionType ) )
            {
                rPropQuery.SetVarEnum( actions_base::m_ActionsAllEnum.GetString( m_ActionType ) );
            }
            else
            {
                rPropQuery.SetVarEnum( "INVALID" );
            } 
        }
        else
        {
            actions_base::action_types ActionType;

            if( actions_base::m_ActionsAllEnum.GetValue( rPropQuery.GetVarEnum(), ActionType ) )
            {
                m_ActionType = ActionType;
            }
        }
        
        return( TRUE );
    }

    if ( 
        rPropQuery.IsVar( "Selector\\Condition Misc"    )       ||
        rPropQuery.IsVar( "Selector\\Condition AI"      )       ||
        rPropQuery.IsVar( "Selector\\Condition Player"  )       ||
        rPropQuery.IsVar( "Selector\\Action Door"       )
        )
    {
        if( rPropQuery.IsRead() )
        {  
            if ( conditional_base::m_ConditionalAllEnum.DoesValueExist( m_ConditionType ) )
            {
                rPropQuery.SetVarEnum( conditional_base::m_ConditionalAllEnum.GetString( m_ConditionType ) );
            }
            else
            {
                rPropQuery.SetVarEnum( "INVALID" );
            }           
        }
        else
        {
            conditional_base::conditional_types ConditionType;
            if( conditional_base::m_ConditionalAllEnum.GetValue( rPropQuery.GetVarEnum(), ConditionType ) )
            {
                m_ConditionType = ConditionType;
            }
        }
        
        return( TRUE );
    } 

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    if( rPropQuery.IsSimilarPath( "Selector\\Global Variables\\Variables\\" ) )
    {
        s32 iHeader = rPropQuery.PushPath( "Selector\\Global Variables\\Variables\\" );        
        
        if ( g_VarMgr.OnPropertyVariables(rPropQuery) )
        {
            rPropQuery.PopPath( iHeader );
            return TRUE;
        }
        
        rPropQuery.PopPath( iHeader );
        
    }
    
    return FALSE;
}

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

struct trigger_object_desc : public object_desc
{

//=========================================================================

    trigger_object_desc( void ) : object_desc( 
        object::TYPE_TRIGGER, 
        "Trigger Object", 
        "SCRIPT",

        object::ATTR_SPACIAL_ENTRY,

        FLAGS_IS_DYNAMIC ) {}
    
    //-------------------------------------------------------------------------

    virtual object* Create          ( void )
    {
        return new trigger_object;
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

} s_TriggerObjectDesc;

//=========================================================================

const object_desc&  trigger_object::GetTypeDesc( void ) const
{
    return s_TriggerObjectDesc;
}

//=========================================================================

const object_desc&  trigger_object::GetObjectType( void )
{
    return s_TriggerObjectDesc;
}

//=========================================================================
// TRIGGER_OBJECT
//=========================================================================

trigger_object::trigger_object(void) :
m_UpdateRate(1.0f),
m_RecoveryRate(0.0f),
m_DelayRate(0.0f),
m_AndFlags(0),
m_OrFlags(0),
m_ElseAndFlags(0),
m_ElseOrFlags(0),      
m_NumConditons(0),
m_NumActions(0),
m_Type(TRIGGER_ONCE),
m_DrawActivationSphere(FALSE),
m_CurrentColor(s_TriggerColor_Sleep),
m_OnActivate(TRUE),
m_RepeatCount(0),
m_ActivateCount(0),
m_UseElse(FALSE),
m_ExecuteElseActions(FALSE),
m_State(STATE_SLEEPING),
m_NextUpdateTime(0.0f),
m_Next(NULL),            
m_Prev(NULL),
m_TriggerSlot(-1),
m_EnteringDelay(TRUE),
m_EnteringRecovery(TRUE)
{
#ifdef TARGET_PC
    m_Selector.Init( this );
#endif
    
    for( s32 i=0; i< MAX_PTR_ARRAY_SIZE ; i++ )
    { 
        m_Conditions[i] = NULL;
        m_Actions[i]    = NULL;
    }
}

//=========================================================================

trigger_object::~trigger_object(void)
{
  for( s32 i=0; i< MAX_PTR_ARRAY_SIZE ; i++ )
    { 
        if (m_Conditions[i] != NULL)
        {
            delete m_Conditions[i];
            m_Conditions[i] = NULL;
        }

        if (m_Actions[i] != NULL)
        {
            delete m_Actions[i];
            m_Actions[i] = NULL;
        }
    }
}

//=========================================================================

void trigger_object::OnInit( void )
{
    object::OnInit();
    
    //Register myself too the global Trigger_Manager object...
    g_TriggerMgr.RegisterTrigger( *this );
    
    //Set the approritate state
    SetTriggerState(STATE_CHECKING);
}

//=========================================================================

void trigger_object::OnKill( void )
{
    object::OnKill();
    
    //Unregister myself too the global Trigger_Manager object...
    g_TriggerMgr.UnregisterTrigger( *this );
}

//=========================================================================

bbox trigger_object::GetLocalBBox( void ) const 
{
    return s_EditSphere.GetBBox(); 
}

//=========================================================================

#ifndef X_RETAIL
void trigger_object::OnDebugRender( void )
{
    if (m_DrawActivationSphere)
        draw_Sphere( object::GetPosition(), s_SphereRadius, s_TriggerColor_Checking );

    draw_BBox( GetBBox(), xcolor(255,0,0) );
    OnRenderActions();
}
#endif // X_RETAIL

//=========================================================================

void  trigger_object::OnRenderActions ( void )
{
    for( s32 i=0; i< MAX_PTR_ARRAY_SIZE ; i++ )
    { 
        if (m_Actions[i] != NULL)
        {
            m_Actions[i]->OnRender();
        }
    }
}

//=========================================================================

void trigger_object::OnEnumProp( prop_enum&  rPropList )
{
    object::OnEnumProp( rPropList );
        
#ifdef TARGET_PC
    m_Selector.OnEnumProp( rPropList );
#endif

    rPropList.AddHeader  ( "Trigger Object",              "The base class for trigger objects.", PROP_TYPE_HEADER);
   
    EnumPropDynamic( rPropList );

    rPropList.AddBool    ( "Trigger Object\\OnActivate",    "On activate flag" );
    rPropList.AddFloat   ( "Trigger Object\\Update Rate",   "The rate at which this trigger updates in seconds( if 0, then no update occurs. )" );
    rPropList.AddFloat   ( "Trigger Object\\Delay Time",    "The time at which a trigger waits before it executues its actions in seconds" );
    rPropList.AddFloat   ( "Trigger Object\\Recovery Rate", "The rate at which repeating triggers recover in seconds( only applies to repeating triggers.)" );
    rPropList.AddInt     ( "Trigger Object\\Repeat Count",  "The number of times a repeating trigger can be activated." );
 
    rPropList.AddEnum    ( "Trigger Object\\Type", "ONCE\0REPEATING\0REPEATING_COUNTED\0", "Type defines the trigger activation behavior.( ONCE allows only 1 execution, REPEAT allows more than one.)" );
  
    EnumPropConditions( rPropList );
    
    rPropList.AddInt     ( "Trigger Object\\And Flags",   "AND Flags for activation.",  PROP_TYPE_DONT_SHOW);
    rPropList.AddInt     ( "Trigger Object\\Or Flags",    "OR Flags for activation." ,  PROP_TYPE_DONT_SHOW);
    
    rPropList.AddString  ( "Trigger Object\\Actions Set",   "",  PROP_TYPE_DONT_SAVE | PROP_TYPE_READ_ONLY );

    EnumPropActions ( rPropList );
    
    rPropList.AddInt     ( "Trigger Object\\NumConditions",   "", PROP_TYPE_DONT_SHOW );
    rPropList.AddInt     ( "Trigger Object\\NumActions",      "", PROP_TYPE_DONT_SHOW );
    
    rPropList.AddBool    ( "Trigger Object\\Else",  "If this trigger uses an else block.", PROP_TYPE_DONT_SHOW );
     
    rPropList.AddString  ( "Trigger Object\\Else Block",   "",  PROP_TYPE_DONT_SAVE | PROP_TYPE_READ_ONLY );

    EnumPropElseConditions( rPropList );
     
    rPropList.AddInt     ( "Trigger Object\\Else And Flags",   "AND Flags for Else activation.",  PROP_TYPE_DONT_SHOW);
    rPropList.AddInt     ( "Trigger Object\\Else Or Flags",    "OR Flags for Else activation." ,  PROP_TYPE_DONT_SHOW);
   
    rPropList.AddString  ( "Trigger Object\\Else Actions Set",   "",  PROP_TYPE_DONT_SAVE | PROP_TYPE_READ_ONLY );

    EnumPropElseActions( rPropList );
   
}

//===========================================================================

xbool trigger_object::OnProperty( prop_query& rPropQuery )
{
    SetAttrBits( GetAttrBits() | FLAGS_DIRTY_TRANSLATION );
    
    if( object::OnProperty( rPropQuery ) )
        return TRUE;
    
    if( OnPropertyDynamic( rPropQuery ) )
        return TRUE;
    
#ifdef TARGET_PC
    if( m_Selector.OnProperty( rPropQuery ) )
        return TRUE;
#endif

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
    
    if ( rPropQuery.IsVar( "Trigger Object\\And Flags" ) )
    {
        if( rPropQuery.IsRead() )
        {
            CalculateAndFlags();
            
            rPropQuery.SetVarInt( *((s32*) &m_AndFlags) );
        }
        else
        {   
            m_AndFlags = (u32) rPropQuery.GetVarInt();
        } 
        
        return TRUE;
    }
      
    if ( rPropQuery.IsVar( "Trigger Object\\Or Flags" ) )
    {
        if( rPropQuery.IsRead() )
        {
            CalculateOrFlags();
            
            rPropQuery.SetVarInt( *((s32*) &m_OrFlags) );
        }
        else
        {   
            m_OrFlags = (u32) rPropQuery.GetVarInt();
        }
        
        return TRUE;
    }
   
    if ( rPropQuery.IsVar( "Trigger Object\\Else And Flags" ) )
    {
        if( rPropQuery.IsRead() )
        {
            CalculateAndFlags();
            
            rPropQuery.SetVarInt( *((s32*) &m_ElseAndFlags) );
        }
        else
        {   
            m_ElseAndFlags = (u32) rPropQuery.GetVarInt();
        } 
        
        return TRUE;
    }
      
    if ( rPropQuery.IsVar( "Trigger Object\\Else Or Flags" ) )
    {
        if( rPropQuery.IsRead() )
        {
            CalculateOrFlags();
            
            rPropQuery.SetVarInt( *((s32*) &m_ElseOrFlags) );
        }
        else
        {   
            m_ElseOrFlags = (u32) rPropQuery.GetVarInt();
        }
        
        return TRUE;
    }

    if ( rPropQuery.VarInt( "Trigger Object\\NumConditions"  ,      m_NumConditons ))
        return TRUE;
    
    if ( rPropQuery.VarInt( "Trigger Object\\NumActions"  ,         m_NumActions  ))
        return TRUE;
 
    if ( rPropQuery.VarBool( "Trigger Object\\OnActivate"  ,        m_OnActivate  ))
        return TRUE;
    
    if ( OnPropertyConditions( rPropQuery ) )
        return TRUE;
  
    if ( OnPropertyActions( rPropQuery ) )
        return TRUE;

    if ( rPropQuery.IsVar( "Trigger Object\\Type"  ) )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Type )
            {
            case TRIGGER_ONCE:                  rPropQuery.SetVarEnum( "ONCE" );                break;
            case TRIGGER_REPEATING:             rPropQuery.SetVarEnum( "REPEATING" );           break;
            case TRIGGER_REPEATING_COUNTED:     rPropQuery.SetVarEnum( "REPEATING_COUNTED" );   break;
            default:
                ASSERT(0);
                break;
            }
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "ONCE" )==0)                { m_Type = TRIGGER_ONCE;}
            if( x_stricmp( pString, "REPEATING" )==0)           { m_Type = TRIGGER_REPEATING;}
            if( x_stricmp( pString, "REPEATING_COUNTED" )==0)   { m_Type = TRIGGER_REPEATING_COUNTED;}
            
        }
        
        return TRUE;
    }
    
    if ( rPropQuery.IsVar ( "Trigger Object\\Else" ) ) 
    {
        if( rPropQuery.IsRead() )
        {
            m_UseElse = CheckElseState();

            rPropQuery.SetVarBool( m_UseElse ); 
        }
        else
        {
            m_UseElse = rPropQuery.GetVarBool();
        }

        return TRUE;
    }

    return FALSE;
}

//=============================================================================

void trigger_object::CalculateAndFlags ( void )
{           
    m_AndFlags      = 0;
    m_ElseAndFlags  = 0;

    s32 i=0;
    
    for ( i = 0 ; i < MAX_PTR_ARRAY_SIZE; i++ )
    {
        ASSERT( i < MAX_BIT_SIZE_OF_FLAGS );
        
        if (m_Conditions[i] == NULL || m_Conditions[i]->GetFlag() != conditional_base::FLAG_AND )
            continue;
        
        if (m_Conditions[i]->GetElse() == TRUE)
            m_ElseAndFlags |= BIT(i);
        else
            m_AndFlags |= BIT(i);
    }
}

//=============================================================================

void trigger_object::CalculateOrFlags ( void )
{
    m_OrFlags       = 0;
    m_ElseOrFlags   = 0;

    s32 i=0;
    
    for ( i = 0 ; i < MAX_PTR_ARRAY_SIZE; i++ )
    {
        ASSERT( i < MAX_BIT_SIZE_OF_FLAGS );
        
        if (m_Conditions[i] == NULL || m_Conditions[i]->GetFlag() != conditional_base::FLAG_OR )
            continue;
        
        if (m_Conditions[i]->GetElse() == TRUE)
            m_ElseOrFlags |= BIT(i);
        else
            m_OrFlags |= BIT(i);
    }
}

//=============================================================================

xbool trigger_object::CheckElseState( void )
{
    //return true if we are a trigger which uses an else block...
    
    xbool Rval = FALSE;

    s32 i = 0;

    for ( i = 0 ; i < MAX_PTR_ARRAY_SIZE; i++ )
    {
        if (m_Conditions[i] == NULL)
            continue;
        
        Rval |= m_Conditions[i]->GetElse();
    } 
    
    for ( i = 0 ; i < MAX_PTR_ARRAY_SIZE; i++ )
    {
        if (m_Actions[i] == NULL)
            continue;
        
        Rval |= m_Actions[i]->GetElse();
    }

    return Rval;
}

//=============================================================================

xbool   trigger_object::IsAwake( void )
{   
    if (m_State == STATE_SLEEPING)
        return FALSE;
    
    return TRUE;
}

//=============================================================================
//used by derived classes who want to update at the natural rate of the trigger but not execute the logic..
    
xbool trigger_object::CanUpdate( f32 DeltaTime )
{
    return CheckNextTime(DeltaTime);
}

//=============================================================================

xbool trigger_object::CheckNextTime ( f32 DeltaTime )
{
    //if we can update make sure to not syncrhonize by carrying the difference over..
    m_NextUpdateTime -= DeltaTime;
 
    if ( m_NextUpdateTime > 0.0f )
        return FALSE;
  
    return TRUE;
}

//=============================================================================

void trigger_object::UpdateNextTime ( f32 Time )
{
    m_NextUpdateTime += Time;
}

//=============================================================================

void trigger_object::SetTriggerState( const trigger_state State )
{
    //cannot come out of DYING state once we enter it...
    if ( m_State == STATE_DYING )
        return;

    //set the desired state
     m_State = State;

    //no update for 0 rate, set it to sleep mode.
    if ( m_UpdateRate == 0.0f )
    {
        //set us to sleeping.
        m_State = STATE_SLEEPING;
    }

    switch ( m_State )
    {
    case STATE_SLEEPING:      
        //tell the manager that were asleep so we dont get anymore updates..
        g_TriggerMgr.TriggerSleep( *this );
        m_CurrentColor = s_TriggerColor_Sleep;
        break;
        
    case STATE_CHECKING:    
        //tell the manager that were awake so we get moved into the updating list.
        ForceNextUpdate();
        g_TriggerMgr.TriggerAwake( *this );
        m_DrawActivationSphere = FALSE;
        m_CurrentColor = s_TriggerColor_Checking;
        break;
        
    case STATE_RECOVERY:
        m_DrawActivationSphere = FALSE;
        m_CurrentColor = s_TriggerColor_Recovering;
        break;
        
    case STATE_DELAYING:
        m_DrawActivationSphere = FALSE;
        m_CurrentColor = s_TriggerColor_Delaying;
        break;

    case STATE_DYING:
        //no-op : could execute actions on an death event..
        break;
        
    default:
        ASSERT(0);
        break;
    }
}

//=============================================================================

void trigger_object::KillTrigger( void )
{
    SetTriggerState(STATE_DYING);
}

//=============================================================================

void trigger_object::ExecuteLogic( f32 DeltaTime )
{
    if (!m_OnActivate)
        return;

    TRIGGER_CONTEXT( "trigger_object::ExecuteLogic" );

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

    case STATE_DYING: 
        g_ObjMgr.DestroyObject( GetGuid() );
        break;

    default:

        ASSERT(0);
        break;
    }
} 

//=============================================================================

void trigger_object::ForceNextUpdate ( void )
{
    m_NextUpdateTime = 0.0f;
}

//=============================================================================

void trigger_object::ExecuteSleeping ( f32 DeltaTime )
{
    ( void ) DeltaTime;

    //Sleep state puts the trigger into an inactive mode
    g_TriggerMgr.TriggerSleep( *this );
}

//=============================================================================

void trigger_object::ExecuteChecking ( f32 DeltaTime )
{
    ( void ) DeltaTime;
    
    //Check state, checks all the conditions, if they meet the requeisite state flags then
    //run the actions.. Then using the type, determine what to do post activiation...

    TRIGGER_CONTEXT( "trigger_object::ExecuteChecking" );

    if ( CheckNextTime(DeltaTime) == FALSE )
        return;

    //Evaulte the conditions, only update the time if we dont get a true evaulaiton because
    //we dont want to time penalize the next state...

    xbool bUpdateTime = !(EvaulateCondtions( ));

    if (bUpdateTime)
        UpdateNextTime( m_UpdateRate );
}

//=============================================================================

void trigger_object::ExecuteRecovery ( f32 DeltaTime )
{  
    //Recovery state, either destroy the object or sets it into checking mode with an extended
    //wait time until the next valid update within checking state.

   ( void ) DeltaTime;

    TRIGGER_CONTEXT( "trigger_object::ExecuteRecovery" );

    switch(m_Type) 
    {
    case TRIGGER_ONCE:
        {
            g_ObjMgr.DestroyObject( object::GetGuid() );
        }
        break;
    case TRIGGER_REPEATING:
        {
            //Only add the recoveryrate once..
            if (m_EnteringRecovery)
            {
                m_EnteringRecovery  = FALSE;
                UpdateNextTime( m_RecoveryRate );
            }
            
            //Check if we can return to our checking state now...
            if (CheckNextTime(DeltaTime)==FALSE)
                return;
 
            //Reset flag
            m_EnteringRecovery = TRUE;
            
            //Set the next state
           SetTriggerState(STATE_CHECKING);
        }
        break; 
    case TRIGGER_REPEATING_COUNTED:
        {
            if ( m_ActivateCount > m_RepeatCount )
            {
                g_ObjMgr.DestroyObject( object::GetGuid() );
            }
            else
            {
                //Only add the recoveryrate once...
                if (m_EnteringRecovery)
                {
                    m_EnteringRecovery  = FALSE;
                    UpdateNextTime( m_RecoveryRate );
                }
                
                //Check if we can return to our checking state now...
                if (CheckNextTime(DeltaTime)==FALSE)
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

void trigger_object::ExecuteDelaying ( f32 DeltaTime )
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
    
    //Execute all actions..
    ExecuteAllActions();
   
    //Reset this flag
    m_EnteringDelay = TRUE;
    
    //Set the next state
    SetTriggerState(STATE_RECOVERY);
        
    //Turn on the draw activation flag
    m_DrawActivationSphere = TRUE;
    
    return;
}

//=============================================================================

xbool trigger_object::EvaulateCondtions ( void )
{
    //evaluate all conditions with respect to the else flags...
    
    xbool Rval = TRUE;

    if ( m_UseElse == FALSE )
    {  
        Rval = EvaulateMainCondtions();
    }
    else
    {
        Rval = EvaulateMainCondtions();
        
        if ( Rval == FALSE )
        {
            Rval = EvaulateElseCondtions();

            if ( Rval == TRUE )
            {
                m_ExecuteElseActions = TRUE;
            }
        }
    }

    return Rval;
}

//=============================================================================

xbool trigger_object::EvaulateMainCondtions   ( void )
{
    u32     ConditonState   = 0;
    xbool   Rval = FALSE;

    for (s32 i = 0 ; i < MAX_PTR_ARRAY_SIZE; i++ )
    {
        ASSERT( i < MAX_BIT_SIZE_OF_FLAGS );
        
        if ( m_Conditions[i] == NULL || m_Conditions[i]->GetElse() == TRUE )
            continue;
        
        if (m_Conditions[i]->Execute(this))
            ConditonState |= BIT(i);
    }
    
    if ( (ConditonState & m_AndFlags) == m_AndFlags )
    {
        //check if any or flags are on too...
        if ( m_OrFlags == 0 || (ConditonState & m_OrFlags) > 0 )
        {
            SetTriggerState(STATE_DELAYING);
            Rval = TRUE;
        }
    }

    return Rval;
}

//=============================================================================

xbool trigger_object::EvaulateElseCondtions   ( void )
{  
    u32     ConditonState   = 0;
    xbool   Rval = FALSE;

    for (s32 i = 0 ; i < MAX_PTR_ARRAY_SIZE; i++ )
    {
        ASSERT( i < MAX_BIT_SIZE_OF_FLAGS );
        
        if ( m_Conditions[i] == NULL || m_Conditions[i]->GetElse() == FALSE )
            continue;
        
        if (m_Conditions[i]->Execute(this))
            ConditonState |= BIT(i);
    }
    
    if ( (ConditonState & m_ElseAndFlags) == m_ElseAndFlags )
    {
        //check if any or flags are on too...
        if ( m_ElseOrFlags == 0 || (ConditonState & m_ElseOrFlags) > 0 )
        {
            SetTriggerState(STATE_DELAYING);
            Rval = TRUE;
        }
    }
    
    return Rval;
}

//=============================================================================

void trigger_object::ExecuteAllActions ( void )
{
    //Increment exectution count..
    m_ActivateCount++;
 
    if ( m_UseElse == FALSE )
    {
        //Execute all actions...
        for (s32 j = 0 ; j < MAX_PTR_ARRAY_SIZE; j++ )
        {
            if (m_Actions[j] == NULL)
                continue;
            
            m_Actions[j]->Execute( this );
        }
    }
    else
    {
        if (m_ExecuteElseActions == FALSE)
        {
            //Execute all non else actions...
            for (s32 j = 0 ; j < MAX_PTR_ARRAY_SIZE; j++ )
            {
                if (m_Actions[j] == NULL || m_Actions[j]->GetElse() == TRUE)
                    continue;
                
                m_Actions[j]->Execute( this );
            }
        }
        else
        {
            //Execute all else actions...
            for (s32 j = 0 ; j < MAX_PTR_ARRAY_SIZE; j++ )
            {
                if (m_Actions[j] == NULL || m_Actions[j]->GetElse() == FALSE)
                    continue;
                
                m_Actions[j]->Execute( this );
            }
        }

        m_ExecuteElseActions = FALSE;
    }
}

//=============================================================================

void trigger_object::AddCondition ( conditional_base::conditional_types ConditionType, s32 Number )
{
    ASSERT ( Number < MAX_PTR_ARRAY_SIZE );
    
    if (m_Conditions[Number])
    {
        delete m_Conditions[Number];
        m_Conditions[Number] = NULL;
    }

    m_Conditions[Number] = conditional_base::CreateCondition( ConditionType, GetGuid() );
}

//=============================================================================

void trigger_object::AddAction( actions_base::action_types ActionType, s32 Number )
{
    ASSERT ( Number < MAX_PTR_ARRAY_SIZE );

    if (m_Actions[Number])
    { 
        delete m_Actions[Number];
        m_Actions[Number] = NULL;
    }
    
    m_Actions[Number] = actions_base::CreateAction( ActionType , GetGuid() );
}

//=============================================================================

void trigger_object::EnumPropConditions ( prop_enum& rPropList )
{
    s32 i = 0;

    for(  i = 0; i< MAX_PTR_ARRAY_SIZE ; i++ )
    { 
        if (m_Conditions[i] != NULL && m_Conditions[i]->GetElse() == FALSE)
        {
            rPropList.AddString( xfs("Trigger Object\\Condition[%d]", i) , 
                m_Conditions[i]->GetTypeInfo(), PROP_TYPE_HEADER );        
            
            s32 iHeader = rPropList.PushPath( xfs("Trigger Object\\Condition[%d]\\", i) );        

            m_Conditions[i]->OnEnumProp(rPropList);

            rPropList.PopPath( iHeader );
        }
    }

    /////////////////////////////////////////////////////////////////////////

    rPropList.AddHeader( "Trigger Object\\Condition Info" , "Human readable conditions overview.", 0);

    for( i = 0; i< MAX_PTR_ARRAY_SIZE ; i++ )
    { 
        if (m_Conditions[i] != NULL && m_Conditions[i]->GetElse() == FALSE)
        {
            rPropList.AddString( xfs("Trigger Object\\Condition Info\\[%d]", i) , 
                "Text Info", PROP_TYPE_HEADER|PROP_TYPE_DONT_SAVE );        
        }
    }
}

//=============================================================================

void trigger_object::EnumPropActions ( prop_enum& rPropList )
{
    for( s32 i=0; i< MAX_PTR_ARRAY_SIZE ; i++ )
    { 
        if (m_Actions[i] != NULL && m_Actions[i]->GetElse() == FALSE)
        {
            rPropList.AddString( xfs("Trigger Object\\Action[%d]", i) , 
                 m_Actions[i]->GetTypeInfo(), PROP_TYPE_HEADER );        
            
            s32 iHeader = rPropList.PushPath( xfs("Trigger Object\\Action[%d]\\", i) );        
            
            m_Actions[i]->OnEnumProp(rPropList);
            
            rPropList.PopPath( iHeader );
        }
    }
}

//=============================================================================

void trigger_object::EnumPropElseConditions  ( prop_enum& rPropList )
{
    s32 i = 0;

    for(  i = 0; i< MAX_PTR_ARRAY_SIZE ; i++ )
    { 
        if (m_Conditions[i] != NULL && m_Conditions[i]->GetElse() == TRUE)
        {
            rPropList.AddString( xfs("Trigger Object\\Else Condition[%d]", i) , 
                m_Conditions[i]->GetTypeInfo(), PROP_TYPE_HEADER );        
            
            s32 iHeader = rPropList.PushPath( xfs("Trigger Object\\Else Condition[%d]\\", i) );        
            
            m_Conditions[i]->OnEnumProp(rPropList);
            
            rPropList.PopPath( iHeader );
        }
    } 
    
    /////////////////////////////////////////////////////////////////////////

    rPropList.AddHeader( "Trigger Object\\Else Condition Info" , "Human readable else conditions overview.", 0);

    for( i = 0; i< MAX_PTR_ARRAY_SIZE ; i++ )
    { 
        if (m_Conditions[i] != NULL && m_Conditions[i]->GetElse() == TRUE)
        {
            rPropList.AddString( xfs("Trigger Object\\Else Condition Info\\[%d]", i) , 
                "Text Info", PROP_TYPE_HEADER|PROP_TYPE_DONT_SAVE );        
        }
    }
}

//=============================================================================

void trigger_object::EnumPropElseActions     ( prop_enum& rPropList )
{
    for( s32 i=0; i< MAX_PTR_ARRAY_SIZE ; i++ )
    { 
        if (m_Actions[i] != NULL && m_Actions[i]->GetElse() == TRUE)
        {
            rPropList.AddString( xfs("Trigger Object\\Else Action[%d]", i) , 
                m_Actions[i]->GetTypeInfo(), PROP_TYPE_HEADER );        
            
            s32 iHeader = rPropList.PushPath( xfs("Trigger Object\\Else Action[%d]\\", i) );        
            
            m_Actions[i]->OnEnumProp(rPropList);
            
            rPropList.PopPath( iHeader );
        }
    }
}

//=============================================================================

xbool trigger_object::OnPropertyConditions ( prop_query& rPropQuery )
{  
    if( rPropQuery.IsSimilarPath( "Trigger Object\\Condition" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(0);
        
        ASSERT( iIndex < MAX_PTR_ARRAY_SIZE && iIndex >= 0 && m_Conditions[iIndex] );
        
        if ( rPropQuery.IsVar( "Trigger Object\\Condition[]" ) )
        { 
            
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarString(m_Conditions[iIndex]->GetTypeName(), MAX_STRING_LEN );
            }
            
            return TRUE;
        }
        
        if (m_Conditions[iIndex])
        {
            s32 iHeader = rPropQuery.PushPath( "Trigger Object\\Condition[]\\" );        
            
            if( m_Conditions[iIndex]->OnProperty(rPropQuery) )
            {
                rPropQuery.PopPath( iHeader );
                return TRUE;
            }  
            
            rPropQuery.PopPath( iHeader );
        }
    }
    
    if( rPropQuery.IsSimilarPath( "Trigger Object\\Else Condition" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(0);
        
        ASSERT( iIndex < MAX_PTR_ARRAY_SIZE && iIndex >= 0 && m_Conditions[iIndex] );
        
        if ( rPropQuery.IsVar( "Trigger Object\\Else Condition[]" ) )
        { 
            
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarString(m_Conditions[iIndex]->GetTypeName(), MAX_STRING_LEN );
            }
            
            return TRUE;
        }
        
        if (m_Conditions[iIndex])
        {
            s32 iHeader = rPropQuery.PushPath( "Trigger Object\\Else Condition[]\\" );        
            
            if( m_Conditions[iIndex]->OnProperty(rPropQuery) )
            {
                rPropQuery.PopPath( iHeader );
                return TRUE;
            }  
            
            rPropQuery.PopPath( iHeader );
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    if( rPropQuery.IsSimilarPath( "Trigger Object\\Condition Info" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(0);
        
        ASSERT( iIndex < MAX_PTR_ARRAY_SIZE && iIndex >= 0 && m_Conditions[iIndex] );
        
        if ( rPropQuery.IsVar( "Trigger Object\\Condition Info\\[]" ) )
        { 
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarString(m_Conditions[iIndex]->GetInfo(), MAX_STRING_LEN );
            }
            
            return TRUE;
        }
    } 
    
    if( rPropQuery.IsSimilarPath( "Trigger Object\\Else Condition Info" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(0);
        
        ASSERT( iIndex < MAX_PTR_ARRAY_SIZE && iIndex >= 0 && m_Conditions[iIndex] );
        
        if ( rPropQuery.IsVar( "Trigger Object\\Else Condition Info\\[]" ) )
        { 
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarString(m_Conditions[iIndex]->GetInfo(), MAX_STRING_LEN );
            }
            
            return TRUE;
        }
    }
    
    return FALSE;
}

//=============================================================================

xbool trigger_object::OnPropertyActions ( prop_query& rPropQuery )
{
    if( rPropQuery.IsSimilarPath( "Trigger Object\\Action" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(0);
        
        ASSERT( iIndex < MAX_PTR_ARRAY_SIZE && iIndex >= 0 );
  
        if ( rPropQuery.IsVar( "Trigger Object\\Action[]" ) )
        {
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarString(m_Actions[iIndex]->GetTypeName(), MAX_STRING_LEN );
            }

            return TRUE;
        }

        if (m_Actions[iIndex])
        {
            s32 iHeader = rPropQuery.PushPath( "Trigger Object\\Action[]\\" );        
            
            if( m_Actions[iIndex]->OnProperty(rPropQuery) )
            {
                rPropQuery.PopPath( iHeader );
                return TRUE;
            }  
            
            rPropQuery.PopPath( iHeader );
        }
    }
    
    if( rPropQuery.IsSimilarPath( "Trigger Object\\Else Action" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(0);
        
        ASSERT( iIndex < MAX_PTR_ARRAY_SIZE && iIndex >= 0 );
  
        if ( rPropQuery.IsVar( "Trigger Object\\Else Action[]" ) )
        {
            if( rPropQuery.IsRead() )
            {
                rPropQuery.SetVarString(m_Actions[iIndex]->GetTypeName(), MAX_STRING_LEN );
            }

            return TRUE;
        }

        if (m_Actions[iIndex])
        {
            s32 iHeader = rPropQuery.PushPath( "Trigger Object\\Else Action[]\\" );        
            
            if( m_Actions[iIndex]->OnProperty(rPropQuery) )
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

void  trigger_object::RemoveCondition ( conditional_base* pCondition )
{
    s32 i=0;
    
    for( i = 0; i< MAX_PTR_ARRAY_SIZE ; i++ )
    { 
        if (m_Conditions[i] == pCondition)
        {
            delete m_Conditions[i];
            m_Conditions[i] = NULL;
            break;
        }
    }
    
    if ( i == MAX_PTR_ARRAY_SIZE )
    {
        x_DebugMsg("trigger_object::RemoveCondition, Cannot find condition in table.");
        ASSERT(0);
        return;
    }

    //Shift the array to remove the empty slot...

    for ( i++; i < MAX_PTR_ARRAY_SIZE; i++)
    {
        m_Conditions[i-1] = m_Conditions[i];
    }

    m_Conditions[MAX_PTR_ARRAY_SIZE-1] = NULL;

    m_NumConditons--;
}

//=============================================================================

void  trigger_object::RemoveAction ( actions_base* pAction )
{
    s32 i=0;
    
    for( i = 0; i< MAX_PTR_ARRAY_SIZE ; i++ )
    { 
        if (m_Actions[i] == pAction)
        {
            delete m_Actions[i];
            m_Actions[i] = NULL;
            break;
        }
    } 
    
    if ( i == MAX_PTR_ARRAY_SIZE )
    {
        x_DebugMsg("trigger_object::RemoveAction, Cannot find action in table.");
        ASSERT(0);
        return;
    }

    //Shift the array to remove the empty slot...

    for ( i++; i < MAX_PTR_ARRAY_SIZE; i++)
    {
        m_Actions[i-1] = m_Actions[i];
    }

    m_Actions[MAX_PTR_ARRAY_SIZE-1] = NULL;

    m_NumActions--;
}

//=============================================================================

void  trigger_object::EnumPropDynamic ( prop_enum& rPropList )
{
    s32 i = 0;

    for( i=0; i< MAX_PTR_ARRAY_SIZE ; i++ )
    { 
        if (m_Conditions[i] != NULL)
        {
            rPropList.AddInt( xfs("Trigger Object\\C[%d]", i ), "", PROP_TYPE_DONT_SHOW );
        }
    }
    
    for( i=0; i< MAX_PTR_ARRAY_SIZE ; i++ )
    { 
        if (m_Actions[i] != NULL)
        {
            rPropList.AddInt( xfs("Trigger Object\\A[%d]", i), "", PROP_TYPE_DONT_SHOW );
        }
    }
}

//=============================================================================

xbool trigger_object::OnPropertyDynamic ( prop_query& rPropQuery )
{
    
    if( rPropQuery.IsSimilarPath( "Trigger Object\\C[" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(0);
        
        ASSERT( iIndex < MAX_PTR_ARRAY_SIZE && iIndex >= 0 );
        
        
        if( m_Conditions[iIndex] != NULL && rPropQuery.IsRead() )
        {
            rPropQuery.SetVarInt( m_Conditions[iIndex]->GetType() );
        }
        else
        { 
            s32 ConditionType = -1;
            
            ConditionType = rPropQuery.GetVarInt();
            
            AddCondition( (conditional_base::conditional_types) ConditionType, iIndex );
        }
        
        return( TRUE );
     }
    
    if( rPropQuery.IsSimilarPath( "Trigger Object\\A[" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(0);
        
        ASSERT( iIndex < MAX_PTR_ARRAY_SIZE && iIndex >= 0 );

        if( m_Actions[iIndex] != NULL && rPropQuery.IsRead() )
        {
            
            rPropQuery.SetVarInt( m_Actions[iIndex]->GetType() );
        }
        else
        { 
            s32 ActionType = -1;
            
            ActionType = rPropQuery.GetVarInt();
            
            AddAction( (actions_base::action_types) ActionType, iIndex );
        }
        
        return( TRUE );
    }
    
    return FALSE;
}           

//=============================================================================

void trigger_object::OnActivate ( xbool Flag )
{
    m_OnActivate = Flag;
}

//===========================================================================

#ifdef WIN32
void trigger_object::EditorPreGame( void )
{
    for( s32 i=0; i< MAX_PTR_ARRAY_SIZE ; i++ )
    { 
        if (m_Actions[i] != NULL)
        {
            m_Actions[i]->EditorPreGame();
        }
    }
}
#endif















