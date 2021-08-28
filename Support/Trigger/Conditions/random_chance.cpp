///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Conditionals.cpp
//
//
///////////////////////////////////////////////////////////////////////////////

#include "..\Support\Trigger\Trigger_Conditionals.hpp"
#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "..\Support\Objects\Player.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"

//=========================================================================
// STATIC FUNCTIONS
//=========================================================================

static sml_string s_OrStr("OR  ");
static sml_string s_AndStr("AND");

//=============================================================================

static const char* GetFlagStr( conditional_base::conditional_flags Flag )
{
    if (Flag == conditional_base::FLAG_OR)
        return s_OrStr.Get();
    else
        return s_AndStr.Get();
}

//=============================================================================

typedef enum_pair<conditional_base::conditional_types> conditional_enum_pair;

//=============================================================================

static conditional_enum_pair s_ConditonalAllEnumTable[] = 
{
        conditional_enum_pair("PLAYER_HEALTH",       conditional_base::TYPE_CONDITION_PLAYER_HEALTH),
        conditional_enum_pair("PLAYER_HAS",          conditional_base::TYPE_CONDITION_PLAYER_HAS),
        conditional_enum_pair("OBJECT_EXIST",        conditional_base::TYPE_CONDITION_OBJECT_EXIST),
        conditional_enum_pair("CHECK_STATE_VARS",    conditional_base::TYPE_CONDITION_CHECK_STATE_VARS),
        conditional_enum_pair("CHECK_SCRIPT",        conditional_base::TYPE_CONDITION_CHECK_SCRIPT),
        conditional_enum_pair("CHECK_TRIGGER_STATE", conditional_base::TYPE_CONDITION_CHECK_TRIGGER_STATE),
        conditional_enum_pair("RANDOM_CHANCE",       conditional_base::TYPE_CONDITION_RANDOM_CHANCE),
        conditional_enum_pair("ONLY_ON_THIS_NPC",    conditional_base::TYPE_CONDITION_ONLY_ON_THIS_NPC),
        conditional_enum_pair("ONLY_ON_THIS_ITEM",   conditional_base::TYPE_CONDITION_ONLY_ON_THIS_ITEM),
        conditional_enum_pair("COUNT_THINGS",        conditional_base::TYPE_CONDITION_COUNT_THINGS),
        conditional_enum_pair("CHECK_TIMMER",        conditional_base::TYPE_CONDITION_CHECK_TIMMER),
        conditional_enum_pair("COUNTER",             conditional_base::TYPE_CONDITION_COUNTER),

        conditional_enum_pair( k_EnumEndStringConst,conditional_base::INVALID_CONDITIONAL_TYPES),  //**MUST BE LAST**//
};

static conditional_enum_pair s_ConditonalPlayerEnumTable[] = 
{
        conditional_enum_pair("PLAYER_HEALTH",       conditional_base::TYPE_CONDITION_PLAYER_HEALTH),
        conditional_enum_pair("PLAYER_HAS",          conditional_base::TYPE_CONDITION_PLAYER_HAS),

        conditional_enum_pair( k_EnumEndStringConst,conditional_base::INVALID_CONDITIONAL_TYPES),  //**MUST BE LAST**//
};

static conditional_enum_pair s_ConditonalAIEnumTable[] = 
{
        conditional_enum_pair("ONLY_ON_THIS_NPC",    conditional_base::TYPE_CONDITION_ONLY_ON_THIS_NPC),

        conditional_enum_pair( k_EnumEndStringConst,conditional_base::INVALID_CONDITIONAL_TYPES),  //**MUST BE LAST**//
};

static conditional_enum_pair s_ConditonalMiscEnumTable[] = 
{
        conditional_enum_pair("OBJECT_EXIST",        conditional_base::TYPE_CONDITION_OBJECT_EXIST),
        conditional_enum_pair("CHECK_STATE_VARS",    conditional_base::TYPE_CONDITION_CHECK_STATE_VARS),
        conditional_enum_pair("CHECK_SCRIPT",        conditional_base::TYPE_CONDITION_CHECK_SCRIPT),
        conditional_enum_pair("CHECK_TRIGGER_STATE", conditional_base::TYPE_CONDITION_CHECK_TRIGGER_STATE),
        conditional_enum_pair("RANDOM_CHANCE",       conditional_base::TYPE_CONDITION_RANDOM_CHANCE),
        conditional_enum_pair("ONLY_ON_THIS_ITEM",   conditional_base::TYPE_CONDITION_ONLY_ON_THIS_ITEM),
        conditional_enum_pair("COUNT_THINGS",        conditional_base::TYPE_CONDITION_COUNT_THINGS),
        conditional_enum_pair("CHECK_TIMMER",        conditional_base::TYPE_CONDITION_CHECK_TIMMER),
        conditional_enum_pair("COUNTER",             conditional_base::TYPE_CONDITION_COUNTER),

        conditional_enum_pair( k_EnumEndStringConst,conditional_base::INVALID_CONDITIONAL_TYPES),  //**MUST BE LAST**//
};

enum_table<conditional_base::conditional_types> conditional_base::m_ConditionalAllEnum(     s_ConditonalAllEnumTable    );        
enum_table<conditional_base::conditional_types> conditional_base::m_ConditionalPlayerEnum(  s_ConditonalPlayerEnumTable );     
enum_table<conditional_base::conditional_types> conditional_base::m_ConditionalAIEnum(      s_ConditonalAIEnumTable     );         
enum_table<conditional_base::conditional_types> conditional_base::m_ConditionalMiscEnum(    s_ConditonalMiscEnumTable   );      

//=============================================================================

condition_create_function* conditional_base::m_CreateHead = NULL;

//=============================================================================

conditional_base*   conditional_base::CreateCondition ( const conditional_types& rType , const guid& rParentGuid )
{
    if ( rType ==  conditional_base::INVALID_CONDITIONAL_TYPES )
        return NULL;
    
    if (conditional_base::m_CreateHead == NULL)
        return NULL;
    
    conditional_base* pNewCondition = NULL;
    
    condition_create_function* pCurrent = conditional_base::m_CreateHead;
    condition_create_function* pMatched = NULL;
    
    while( pCurrent != NULL )
    {
        if (pCurrent->m_Type == rType)
        {
            pMatched = pCurrent;
            break;
        }
        
        pCurrent = pCurrent->m_Next;
    }
    
    if (pMatched == NULL)
        return NULL;
    
    pNewCondition = pMatched->m_pCreateCondition( rParentGuid );
    
    return pNewCondition;
}

//=========================================================================
// CONDITIONAL_BASE
//=========================================================================

conditional_base::conditional_base(  guid ParentGuid ) : m_Flag(FLAG_AND), m_ElseFlag(FALSE), m_ParentGuid(ParentGuid)
{}

//=============================================================================

conditional_base::~conditional_base() 
{}

//=============================================================================

void conditional_base::OnEnumProp ( prop_enum& rPropList )
{
   rPropList.AddInt     ( "Flag" , "" , PROP_TYPE_DONT_SHOW  );
   rPropList.AddEnum    ( "Activation Flag" ,"MUST BE TRUE\0COULD BE TRUE\0", 
       "The state this condtion has to be in for it to activate an action.", PROP_TYPE_MUST_ENUM|PROP_TYPE_DONT_SAVE  );
   rPropList.AddBool    ( "Else",     "Condition is used in the else block if TRUE", 0 );
   rPropList.AddButton  ( "Remove" ,  "Removes this condtion.", PROP_TYPE_MUST_ENUM  );
}

//=============================================================================

xbool conditional_base::OnProperty ( prop_query& rPropQuery )
{  
    if ( rPropQuery.VarBool  ( "Else", m_ElseFlag ) )
    {
        return TRUE;
    }
 
    if ( rPropQuery.VarInt  ( "Flag", *((s32*) &m_Flag )) )
    {
        return TRUE;
    }

    if ( rPropQuery.IsVar( "Activation Flag"  ) )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Flag )
            {
            case FLAG_AND:          rPropQuery.SetVarEnum( "MUST BE TRUE" );                    break;
            case FLAG_OR:           rPropQuery.SetVarEnum( "COULD BE TRUE" );                   break;
              
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "MUST BE TRUE" )==0) {                m_Flag = FLAG_AND;}
            if( x_stricmp( pString, "COULD BE TRUE" )==0) {               m_Flag = FLAG_OR;}
            
            return( TRUE );
        }
    }
   
    if( rPropQuery.IsVar( "Remove" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Remove" );
        }
        else
        {
#ifdef WIN32            
            //if( IDYES == MessageBox( NULL, "Are you sure you want to delete this entry?", "Warning",MB_ICONWARNING|MB_YESNO ) )
            {
                ASSERT( m_ParentGuid );
                
                object* pObject = NULL;
                
                if ( SMP_UTIL_IsGuidOfType ( &pObject, m_ParentGuid , trigger_object::GetRTTI() ) == TRUE )
                {
                    //NOTE :: RemoveCondition destroys the condition immedeaitely, so do not perform any operation 
                    //      on the condition after calling that function..

                    ((trigger_object*)pObject)->RemoveCondition( this );

                    return TRUE;
                }        
            }
#endif//WIN32
        }
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

void conditional_base::RegisterCreationFunction( condition_create_function* pCreate )
{
    if (conditional_base::m_CreateHead == NULL)
        conditional_base::m_CreateHead = pCreate;
    else
    {
        condition_create_function* pCurrent = conditional_base::m_CreateHead;
        
        while( pCurrent->m_Next != NULL )
        {
            pCurrent = pCurrent->m_Next;
        }

        pCurrent->m_Next = pCreate;
    }
}

//=========================================================================
// PLAYER_HEALTH : checks on player health...
//=========================================================================

condition_create_function player_health::m_Register( TYPE_CONDITION_PLAYER_HEALTH, player_health::Create );

player_health::code_pair player_health::s_PairTable[] = 
{
        code_pair("GREATER_INCLUSIVE", player_health::CODE_GREATER_INCLUSIVE),
        code_pair("LESSER_INCLUSIVE",  player_health::CODE_LESSER_INCLUSIVE),
        code_pair("GREATER",           player_health::CODE_GREATER),
        code_pair("LESSER",            player_health::CODE_LESSER),
        code_pair("EQUAL",             player_health::CODE_EQUAL),
        code_pair("NOT_EQUAL",         player_health::CODE_NOT_EQUAL),
        
        code_pair( k_EnumEndStringConst,player_health::INVALID_CODES),  //**MUST BE LAST**//
};

player_health::code_table player_health::s_EnumTable( s_PairTable );

//=============================================================================

player_health::player_health(  guid ParentGuid ):  conditional_base(  ParentGuid ), m_Code(CODE_GREATER_INCLUSIVE), m_Value(0.0f)
{}

//=============================================================================

xbool player_health::Execute         ( trigger_object* pParent )
{ 
    TRIGGER_CONTEXT( "CONDITION * player_health::Execute" );
    
    (void) pParent;

    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if (!pPlayer)
        return FALSE;
    
    f32 CurrentHealth = pPlayer->GetHealth();
    
    switch ( m_Code )
    {
    case CODE_GREATER_INCLUSIVE:    return CurrentHealth >= m_Value;
    case CODE_LESSER_INCLUSIVE:     return CurrentHealth <= m_Value;
    case CODE_GREATER:              return CurrentHealth >  m_Value;
    case CODE_LESSER:               return CurrentHealth <  m_Value;
    case CODE_EQUAL:                return CurrentHealth == m_Value;
    case CODE_NOT_EQUAL:            return CurrentHealth != m_Value;
    default:
        ASSERT(0);
        break;
    }
    
    return FALSE; 
}    

//=============================================================================

void player_health::OnEnumProp ( prop_enum& rPropList )
{   
    //object info
    rPropList.AddInt ( "Code" , "",  PROP_TYPE_DONT_SHOW  );
    
    rPropList.AddFloat ( "Health" ,  
        "The value of the health to check against." );
    
    rPropList.AddEnum( "Logic", s_EnumTable.BuildString() , "Logic available to this condtional.", PROP_TYPE_DONT_SAVE );
 
    conditional_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool player_health::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Code"  ,  m_Code ) )
        return TRUE;
    
    if ( rPropQuery.VarFloat( "Health"  ,  m_Value ) )
        return TRUE;

    if ( SMP_UTIL_IsEnumVar<s32,codes>(rPropQuery, "Logic", m_Code, s_EnumTable ) )
        return TRUE;
 
    if( conditional_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    return FALSE;
}

//=============================================================================

const char*  player_health::GetInfo( void )
{
    static big_string   Info;
    
    sml_string  LogicFlag ( GetFlagStr(m_Flag) );

    switch ( m_Code )
    {
    case CODE_GREATER_INCLUSIVE:   
        Info.Set(xfs("%s If the Players Health is Greater or Equal to %3.3f.", LogicFlag.Get(), m_Value));      break;
    case CODE_LESSER_INCLUSIVE:     
        Info.Set(xfs("%s If the Players Health is Less than or Equal to %3.3f.", LogicFlag.Get(), m_Value));    break;
    case CODE_GREATER:              
        Info.Set(xfs("%s If the Players Health is Greater than %3.3f.", LogicFlag.Get(), m_Value));             break;
    case CODE_LESSER:               
        Info.Set(xfs("%s If the Players Health is Les than %3.3f.", LogicFlag.Get(), m_Value));                 break;
    case CODE_EQUAL:                
        Info.Set(xfs("%s If the Players Health is Equal to %3.3f.", LogicFlag.Get(), m_Value));                 break;
    case CODE_NOT_EQUAL:            
        Info.Set(xfs("%s If the Players Health is Not Equal to %3.3f.", LogicFlag.Get(), m_Value));             break;
    default:
        ASSERT(0);
        break;
    }

    return Info.Get();
}

//=========================================================================
// PLAYER_HAS  : checks if player has item...
//=========================================================================

condition_create_function player_has::m_Register( TYPE_CONDITION_PLAYER_HAS, player_has::Create );

//=============================================================================

player_has::player_has(  guid ParentGuid ):  conditional_base(  ParentGuid ),  m_ItemID(0), m_Code(CODE_HAS_ITEM)
{}

//=============================================================================

xbool player_has::Execute         ( trigger_object* pParent )
{  
    TRIGGER_CONTEXT( "CONDITION *player_has::Execute" );
    
    (void) pParent;

    switch (m_Code)
    {
    case CODE_HAS_ITEM:         break;
    case CODE_NOT_HAVE_ITEM:    break;
    default:
        ASSERT(0);
        break;
    }
    
    return FALSE; 
}    

//=============================================================================

void player_has::OnEnumProp ( prop_enum& rPropList )
{ 
    //object info
    rPropList.AddInt ( "Code" , "",  PROP_TYPE_DONT_SHOW  );
    
    rPropList.AddInt ( "Item ID" , 
        "ID of the item to check for." );
    
    rPropList.AddEnum( "Logic", 
        "HAS_ITEM\0NOT_HAVE_ITEM\0", "Logic available to this condtional.", PROP_TYPE_DONT_SAVE  );
    
    conditional_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool player_has::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;
    
    if ( rPropQuery.VarInt ( "Item ID" , m_ItemID ) )
        return TRUE;
    
    if ( rPropQuery.IsVar( "Logic") )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Code )
            {
            case CODE_HAS_ITEM:             rPropQuery.SetVarEnum( "HAS_ITEM" );            break;
            case CODE_NOT_HAVE_ITEM:        rPropQuery.SetVarEnum( "NOT_HAVE_ITEM" );       break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "HAS_ITEM" )==0) {          m_Code = CODE_NOT_HAVE_ITEM;}
            if( x_stricmp( pString, "NOT_HAVE_ITEM" )==0) {     m_Code = CODE_NOT_HAVE_ITEM;}
            
            return( TRUE );
        }
    }
    
    if( conditional_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char*  player_has::GetInfo( void )
{
    static big_string   Info;
    
    sml_string  LogicFlag ( GetFlagStr(m_Flag) );

    Info.Set(xfs("%s If the Player Has an item %i.", LogicFlag.Get(), m_ItemID));

    return Info.Get();
}

//=========================================================================
// OBJECT_EXIST : checks if an object exist...
//=========================================================================

condition_create_function object_exist::m_Register( TYPE_CONDITION_OBJECT_EXIST, object_exist::Create );

//=============================================================================

object_exist::object_exist(  guid ParentGuid ):  conditional_base(  ParentGuid ), m_ObjectGuid(NULL), m_Code(CODE_DOES_OBJECT_EXIST)
{}

//=============================================================================

xbool object_exist::Execute         ( trigger_object* pParent )
{  
    TRIGGER_CONTEXT( "CONDITION *object_exist::Execute" );
    
    (void) pParent;

    object* pObject = g_ObjMgr.GetObjectByGuid( m_ObjectGuid );
    
    switch (m_Code)
    {
    case CODE_DOES_OBJECT_EXIST:
        
        if ( pObject != NULL )
            return TRUE;
        
        return FALSE;
        
        break;

    case CODE_DOES_OBJECT_NOT_EXIST:
        
        if ( pObject != NULL )
            return FALSE;
        
        return TRUE;
        
        break;

    default:
        ASSERT(0);
        break;
    }
    
    return FALSE; 
}    

//=============================================================================

void object_exist::OnEnumProp ( prop_enum& rPropList )
{ 
    //object info 
    rPropList.AddInt ( "Code" , "",  PROP_TYPE_DONT_SHOW  );
    
    rPropList.AddGuid ( "Object Guid" , 
        "The GUID of the object to watch." );
    
    rPropList.AddEnum( "Logic", 
        "DOES_OBJECT_EXIST\0OBJECT_DOES_NOT_EXIST\0", "Logic available to this condtional.", PROP_TYPE_DONT_SAVE  );
    
    conditional_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool object_exist::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Code"  , m_Code ) )
        return TRUE;
    
    if ( rPropQuery.VarGUID ( "Object Guid"  , m_ObjectGuid ) )
        return TRUE;
    
    if ( rPropQuery.IsVar( "Logic" ) )
    {
        
        if( rPropQuery.IsRead() )
        {
            switch ( m_Code )
            {
            case CODE_DOES_OBJECT_EXIST:     rPropQuery.SetVarEnum( "DOES_OBJECT_EXIST" );   break;
            case CODE_DOES_OBJECT_NOT_EXIST: rPropQuery.SetVarEnum( "OBJECT_DOES_NOT_EXIST" );   break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "DOES_OBJECT_EXIST" )==0)       { m_Code = CODE_DOES_OBJECT_EXIST;}
            if( x_stricmp( pString, "OBJECT_DOES_NOT_EXIST" )==0)   { m_Code = CODE_DOES_OBJECT_NOT_EXIST;}
            
            return( TRUE );
        }
    }
 
    if( conditional_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char*  object_exist::GetInfo( void )
{
    static big_string   Info;
    
    sml_string  LogicFlag ( GetFlagStr(m_Flag) );

    switch ( m_Code )
    {
    case CODE_DOES_OBJECT_EXIST:     Info.Set(xfs("%s If Object %0x Exist.", LogicFlag.Get(), m_ObjectGuid));   break;
    case CODE_DOES_OBJECT_NOT_EXIST: Info.Set(xfs("%s If Object %0x Does Not Exist.", LogicFlag.Get(), m_ObjectGuid));   break;
    default:
        ASSERT(0);
        break;
    }

    return Info.Get();
}

//=========================================================================
// CHECK_STATE_VARS : checks the global variable is..
//=========================================================================

condition_create_function check_state_vars::m_Register( TYPE_CONDITION_CHECK_STATE_VARS, check_state_vars::Create );

check_state_vars::code_pair check_state_vars::s_PairTable[] = 
{
        code_pair("GREATER_INCLUSIVE", check_state_vars::CODE_GREATER_INCLUSIVE),
        code_pair("LESSER_INCLUSIVE",  check_state_vars::CODE_LESSER_INCLUSIVE),
        code_pair("GREATER",           check_state_vars::CODE_GREATER),
        code_pair("LESSER",            check_state_vars::CODE_LESSER),
        code_pair("EQUAL",             check_state_vars::CODE_EQUAL),
        code_pair("NOT_EQUAL",         check_state_vars::CODE_NOT_EQUAL),
        
        code_pair( k_EnumEndStringConst,check_state_vars::INVALID_CODES),  //**MUST BE LAST**//
};

check_state_vars::code_table check_state_vars::s_EnumTable( s_PairTable );

check_state_vars::code_pair check_state_vars::s_BoolPairTable[] = 
{
        code_pair("EQUAL",             check_state_vars::CODE_EQUAL),
        code_pair("NOT_EQUAL",         check_state_vars::CODE_NOT_EQUAL),
        
        code_pair( k_EnumEndStringConst,check_state_vars::INVALID_CODES),  //**MUST BE LAST**//
};

check_state_vars::code_table check_state_vars::s_BoolEnumTable( s_BoolPairTable );

//=============================================================================

check_state_vars::check_state_vars(  guid ParentGuid ):  conditional_base(  ParentGuid ), 
m_Code(CODE_EQUAL),
m_VarHandle(HNULL),     
m_Type(var_mngr::TYPE_NULL),          
m_VarRaw(0)
{
}

//=============================================================================

xbool check_state_vars::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "CONDITION *check_state_vars::Execute" );

    (void) pParent;

    if (m_VarHandle.IsNull())
        return FALSE;
    
    switch ( g_VarMgr.GetType( m_VarHandle ) )
    {
    case var_mngr::TYPE_FLOAT: 
        {
            f32  GlobalValue = g_VarMgr.GetFloat( m_VarHandle );
            f32& FloatValue = *((f32*) &m_VarRaw);
            
            switch (m_Code)
            {
            case CODE_GREATER_INCLUSIVE:    return GlobalValue >= FloatValue;
            case CODE_LESSER_INCLUSIVE:     return GlobalValue <= FloatValue;
            case CODE_GREATER:              return GlobalValue >  FloatValue;
            case CODE_LESSER:               return GlobalValue <  FloatValue;
            case CODE_EQUAL:                return GlobalValue == FloatValue;
            case CODE_NOT_EQUAL:            return GlobalValue != FloatValue;    
            default:
                ASSERT(0);
                break;
            }
        }
        break;
        
    case var_mngr::TYPE_INT: 
        {
            s32  GlobalValue = g_VarMgr.GetInt( m_VarHandle );
            s32& IntValue = *((s32*) &m_VarRaw);
            
            switch (m_Code)
            {
            case CODE_GREATER_INCLUSIVE:    return GlobalValue >= IntValue;
            case CODE_LESSER_INCLUSIVE:     return GlobalValue <= IntValue;
            case CODE_GREATER:              return GlobalValue >  IntValue;
            case CODE_LESSER:               return GlobalValue <  IntValue;
            case CODE_EQUAL:                return GlobalValue == IntValue;
            case CODE_NOT_EQUAL:            return GlobalValue != IntValue;    
            default:
                ASSERT(0);
                break;
            }
        }
        break;
        
    case var_mngr::TYPE_BOOL: 
        {
            xbool  GlobalValue = g_VarMgr.GetBool( m_VarHandle );
            xbool& BoolValue = *((xbool*) &m_VarRaw);
            
            switch (m_Code)
            {
            case CODE_GREATER_INCLUSIVE:    return FALSE;
            case CODE_LESSER_INCLUSIVE:     return FALSE;
            case CODE_GREATER:              return FALSE;
            case CODE_LESSER:               return FALSE;
            case CODE_EQUAL:                return GlobalValue == BoolValue;
            case CODE_NOT_EQUAL:            return GlobalValue != BoolValue;    
            default:
                ASSERT(0);
                break;
            }
        }
        break;
    }
    
    return FALSE; 
}    

//=============================================================================

void check_state_vars::OnEnumProp ( prop_enum& rPropList )
{ 
    //object info
    rPropList.AddInt ( "Code" , "",  PROP_TYPE_DONT_SHOW  );
    
    rPropList.AddString	( "Variable Name",
        "Name of the Global Variable.", PROP_TYPE_MUST_ENUM );
    
    rPropList.AddEnum( "Type",
        "FLOAT\0INT\0BOOL\0UNDETERMINED\0", "Variable Type." , PROP_TYPE_READ_ONLY | PROP_TYPE_DONT_SAVE );
 
    switch ( m_Type )
    { 
    case var_mngr::TYPE_FLOAT: 
        {
            rPropList.AddFloat ( "Float" , "Floating point value to test against." );
        }
        break; 
        
    case var_mngr::TYPE_INT: 
        {
            rPropList.AddInt ( "Int" , "Interger value to test against." );
        }
        break;
    case var_mngr::TYPE_BOOL: 
        {
            rPropList.AddBool( "Bool", "Boolean value to test against."  );
        }
        break;
    default:
        
        break;
    }
     
    switch (m_Type)
    {
    case var_mngr::TYPE_FLOAT: case  var_mngr::TYPE_INT:
        rPropList.AddEnum( "Logic", s_EnumTable.BuildString(),     "Logic available to this condtional.", PROP_TYPE_DONT_SAVE  );
        break;
    case var_mngr::TYPE_BOOL: 
        rPropList.AddEnum( "Logic", s_BoolEnumTable.BuildString(), "Logic available to this condtional.", PROP_TYPE_DONT_SAVE  );
        break;
    default:
        break;   
    }
    
    conditional_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool check_state_vars::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;
    
    if ( rPropQuery.IsVar( "Variable Name" ))
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarString( m_VariableName.Get(), m_VariableName.MaxLen() );
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarString();
            
            m_VariableName.Set( pString );

            if ( g_VarMgr.GetVarHandle( m_VariableName.Get(), &m_VarHandle ) == TRUE )
            {
                m_Type      = g_VarMgr.GetType( m_VarHandle );
            }
            else
            {
                m_Type      = var_mngr::TYPE_NULL;
                m_VarRaw    = 0;
                m_VarHandle = HNULL;
            }

            return( TRUE );
        }
    }
    
    if (rPropQuery.VarFloat ( "Float", *((f32*) &m_VarRaw) ))
    { 
        return TRUE; 
    }
    
    if (rPropQuery.VarInt ( "Int" , m_VarRaw ))
    { 
        return TRUE;
    }

    if (rPropQuery.VarBool ( "Bool" , *((xbool*) &m_VarRaw) ))
    { 
        return TRUE;
    }

    if ( rPropQuery.IsVar( "Type"))
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Type )
            { 
            case var_mngr::TYPE_FLOAT:  rPropQuery.SetVarEnum( "FLOAT" );           break; 
            case var_mngr::TYPE_INT:    rPropQuery.SetVarEnum( "INT" );             break;  
            case var_mngr::TYPE_BOOL:   rPropQuery.SetVarEnum( "BOOL" );            break;   
            case var_mngr::TYPE_NULL:   rPropQuery.SetVarEnum( "UNDETERMINED" );    break;   
            default:                    ASSERT(0);    break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "FLOAT" )==0)           {      m_Type = var_mngr::TYPE_FLOAT;   }
            if( x_stricmp( pString, "INT" )==0)             {      m_Type = var_mngr::TYPE_INT;     }
            if( x_stricmp( pString, "BOOL" )==0)            {      m_Type = var_mngr::TYPE_BOOL;    }
            if( x_stricmp( pString, "UNDETERMINED" )==0)    {      m_Type = var_mngr::TYPE_NULL;    }
            
            return( TRUE );
        }
    }
    
    switch (m_Type)
    {
    case var_mngr::TYPE_FLOAT: case  var_mngr::TYPE_INT:
        if ( SMP_UTIL_IsEnumVar<s32,codes>(rPropQuery, "Logic", m_Code, s_EnumTable ) )
            return TRUE;
        break;
    case var_mngr::TYPE_BOOL: 
        if ( SMP_UTIL_IsEnumVar<s32,codes>(rPropQuery, "Logic", m_Code, s_BoolEnumTable ) )
            return TRUE; break;
    default:
        break;   
    }
    
    if( conditional_base::OnProperty( rPropQuery ) )
        return TRUE;
    
    return FALSE;
}

//=============================================================================

const char*  check_state_vars::GetInfo( void )
{
    static big_string   Info;
    
    sml_string  VarType;
    sml_string  LogicFlag ( GetFlagStr(m_Flag) );

    switch ( m_Type )
    { 
    case var_mngr::TYPE_FLOAT:  VarType.Set("Float");         break; 
    case var_mngr::TYPE_INT:    VarType.Set("Int");           break;  
    case var_mngr::TYPE_BOOL:   VarType.Set("Bool");          break;   
    default:                     break;
    }

    if (VarType.IsEmpty())
        return "\0";
    
    if ( m_Type == var_mngr::TYPE_BOOL )
    {
        sml_string BoolStr("TRUE");

        if (*((xbool*) &m_VarRaw) == FALSE)
        {
            BoolStr.Set("FALSE");
        }

        switch ( m_Code )
        {
        case CODE_EQUAL:        Info.Set(xfs("%s If %s Variable '%s' is Equal to %s.", 
                                    LogicFlag.Get(), VarType.Get(), m_VariableName.Get(), BoolStr.Get())); break;
        case CODE_NOT_EQUAL:    Info.Set(xfs("%s If %s Variable '%s' is Not Equal to %s.", 
                                    LogicFlag.Get(), VarType.Get(), m_VariableName.Get(), BoolStr.Get())); break;
        default:
            //no-op : all other options are incompatiable for this type..
            break;
        }
    }
    else if ( m_Type == var_mngr::TYPE_INT )
    {
        switch ( m_Code )
        {
        case CODE_GREATER_INCLUSIVE:    Info.Set(xfs("%s If %s Variable '%s' is Greater or Equal to %i.", 
                                    LogicFlag.Get(), VarType.Get(), m_VariableName.Get(), m_VarRaw)); break;
        case CODE_LESSER_INCLUSIVE:     Info.Set(xfs("%s If %s Variable '%s' is Less than or Equal to %i.", 
                                    LogicFlag.Get(), VarType.Get(), m_VariableName.Get(), m_VarRaw)); break;
        case CODE_GREATER:              Info.Set(xfs("%s If %s Variable '%s' is Greather than %i.", 
                                    LogicFlag.Get(), VarType.Get(), m_VariableName.Get(), m_VarRaw)); break;
        case CODE_LESSER:               Info.Set(xfs("%s If %s Variable '%s' is Less than %i.", 
                                    LogicFlag.Get(), VarType.Get(), m_VariableName.Get(), m_VarRaw)); break;
        case CODE_EQUAL:                Info.Set(xfs("%s If %s Variable '%s' is Equal to %i.", 
                                    LogicFlag.Get(), VarType.Get(), m_VariableName.Get(), m_VarRaw)); break;
        case CODE_NOT_EQUAL:            Info.Set(xfs("%s If %s Variable '%s' is Not Equal to %i.", 
                                    LogicFlag.Get(), VarType.Get(), m_VariableName.Get(), m_VarRaw)); break;
        default:
            ASSERT(0);
            break;
        }
    }
    else if ( m_Type == var_mngr::TYPE_FLOAT )
    {
          switch ( m_Code )
        {
        case CODE_GREATER_INCLUSIVE:    Info.Set(xfs("%s If %s Variable '%s' is Greater or Equal to %3.3f.", 
                                    LogicFlag.Get(), VarType.Get(), m_VariableName.Get(), *((f32*) &m_VarRaw))); break;
        case CODE_LESSER_INCLUSIVE:     Info.Set(xfs("%s If %s Variable '%s' is Less than or Equal to %3.3f.", 
                                    LogicFlag.Get(), VarType.Get(), m_VariableName.Get(), *((f32*) &m_VarRaw))); break;
        case CODE_GREATER:              Info.Set(xfs("%s If %s Variable '%s' is Greather than %3.3f.", 
                                    LogicFlag.Get(), VarType.Get(), m_VariableName.Get(), *((f32*) &m_VarRaw))); break;
        case CODE_LESSER:               Info.Set(xfs("%s If %s Variable '%s' is Less than %3.3f.", 
                                    LogicFlag.Get(), VarType.Get(), m_VariableName.Get(), *((f32*) &m_VarRaw))); break;
        case CODE_EQUAL:                Info.Set(xfs("%s If %s Variable '%s' is Equal to %3.3f.", 
                                    LogicFlag.Get(), VarType.Get(), m_VariableName.Get(), *((f32*) &m_VarRaw))); break;
        case CODE_NOT_EQUAL:            Info.Set(xfs("%s If %s Variable '%s' is Not Equal to %3.3f.", 
                                    LogicFlag.Get(), VarType.Get(), m_VariableName.Get(), *((f32*) &m_VarRaw))); break;
        default:
            ASSERT(0);
            break;
        }
    }

    return Info.Get();
}

//=========================================================================
// CHECK_SCRIPT : checks on the state of a script
//=========================================================================

condition_create_function check_script::m_Register( TYPE_CONDITION_CHECK_SCRIPT, check_script::Create );

//=============================================================================

check_script::check_script(  guid ParentGuid ):  conditional_base(  ParentGuid ), 
m_ScriptID(0), 
m_Code(CODE_SCRIPT_EVAULATE) 
{}

//=============================================================================

xbool check_script::Execute         ( trigger_object* pParent )
{ 
    TRIGGER_CONTEXT( "CONDITION *check_script::Execute" );
    
    (void) pParent;

    switch (m_Code)
    {
    case CODE_SCRIPT_EVAULATE:    break; 
    default:
        ASSERT(0);
        break;
    }
    
    return FALSE; 
}    

//=============================================================================

void check_script::OnEnumProp ( prop_enum& rPropList )
{ 
    //object info
    rPropList.AddInt ( "Code" , "",  PROP_TYPE_DONT_SHOW  );
    
    rPropList.AddInt ( "Script ID" , 
        "ID of the script to evaultate." );
    
    rPropList.AddEnum( "Logic", 
        "SCRIPT_EVAULATE\0", "Logic available to this condtional.", PROP_TYPE_DONT_SAVE  );
    
    conditional_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool check_script::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Code"  , m_Code ) )
        return TRUE;
    
    if ( rPropQuery.VarInt ( "Script ID"  , m_ScriptID ) )
        return TRUE;
    
    if ( rPropQuery.IsVar( "Logic" ) ) 
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Code )
            {
            case CODE_SCRIPT_EVAULATE:    rPropQuery.SetVarEnum( "SCRIPT_EVAULATE" );   break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "SCRIPT_EVAULATE" )==0) { m_Code = CODE_SCRIPT_EVAULATE;}
            
            return( TRUE );
        }
    }
       
    if( conditional_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char*  check_script::GetInfo( void )
{
    static big_string   Info;
    
    sml_string  LogicFlag ( GetFlagStr(m_Flag) );

    Info.Set(xfs("%s If Script %i Evalulates to TRUE.", LogicFlag.Get() , m_ScriptID ));

    return Info.Get();
}

//=========================================================================
// CHECK_TRIGGER_STATE : checks in the state of an existing trigger
//=========================================================================

condition_create_function check_trigger_state::m_Register( TYPE_CONDITION_CHECK_TRIGGER_STATE, check_trigger_state::Create );

//=============================================================================

check_trigger_state::check_trigger_state(  guid ParentGuid ):  conditional_base(  ParentGuid ), 
m_TriggerGuid(NULL),
m_Code(CODE_TRIGGER_EVAULTATE)
{}

//=============================================================================

xbool check_trigger_state::Execute         ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "CONDITION *check_trigger_state::Execute" );

    (void) pParent;

    switch (m_Code)
    {
    case CODE_TRIGGER_EVAULTATE:    break; 
    default:
        ASSERT(0);
        break;
    }
    
    return FALSE; 
}    

//=============================================================================

void check_trigger_state::OnEnumProp	( prop_enum& rPropList )
{
    //object info 
    rPropList.AddInt ( "Code" , "",  PROP_TYPE_DONT_SHOW  );
    
    rPropList.AddGuid ("Trigger Guid" , 
        "GUID of the trigger object to evaluate." );
    
    rPropList.AddEnum( "Logic", 
        "TRIGGER_EVAULTATE\0", "Logic available to this condtional.", PROP_TYPE_DONT_SAVE  ); 
    
    conditional_base::OnEnumProp( rPropList );
    
}

//=============================================================================

xbool check_trigger_state::OnProperty	( prop_query& rPropQuery )
{
  
    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;
    
    if ( rPropQuery.VarGUID ( "Trigger Guid" , m_TriggerGuid ) )
        return TRUE;
    
    if ( rPropQuery.IsVar( "Logic" ) ) 
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Code )
            {
            case CODE_TRIGGER_EVAULTATE:    rPropQuery.SetVarEnum( "TRIGGER_EVAULTATE" );   break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "TRIGGER_EVAULTATE" )==0) { m_Code = CODE_TRIGGER_EVAULTATE;}
            
            return( TRUE );
        }
    }
  
    if( conditional_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char*  check_trigger_state::GetInfo ( void )
{
    static big_string   Info;
    
    sml_string  LogicFlag ( GetFlagStr(m_Flag) );

    Info.Set(xfs("%s If Trigger %x Evaultates to TRUE.", LogicFlag.Get(), m_TriggerGuid));

    return Info.Get();
}

//=========================================================================
// RANDOM_CHANCE : True based on a random chance..
//=========================================================================

condition_create_function random_chance::m_Register( TYPE_CONDITION_RANDOM_CHANCE, random_chance::Create );

//=============================================================================

random_chance::random_chance(  guid ParentGuid ):  conditional_base(  ParentGuid ), 
m_RandomPercent(0.0f),
m_Code(CODE_RANDOM_CHANCE)
{}

//=============================================================================

xbool random_chance::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "CONDITION *random_chance::Execute" );

    (void) pParent;

    switch (m_Code)
    {
    case CODE_RANDOM_CHANCE:    
        return ( ((f32)(m_RandomPercent/100)*100) > x_frand(0,100) );
        break; 
    default:
        ASSERT(0);
        break;
    }
    
    return FALSE; 
}    

//=============================================================================

void random_chance::OnEnumProp ( prop_enum& rPropList )
{ 
    //object info 
    rPropList.AddInt ( "Code" , "",  PROP_TYPE_DONT_SHOW  );
    
    rPropList.AddFloat ("Random Percent" , 
        "The random chance of this condtion being true." );
    
    rPropList.AddEnum( "Logic", 
        "RANDOM_CHANCE\0", "Logic available to this condtional.", PROP_TYPE_DONT_SAVE  ); 
    
    conditional_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool random_chance::OnProperty	( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;
    
    if ( rPropQuery.VarFloat ( "Random Percent" , m_RandomPercent ) )
        return TRUE;
    
    if ( rPropQuery.IsVar( "Logic" ) ) 
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Code )
            {
            case CODE_RANDOM_CHANCE:    rPropQuery.SetVarEnum( "RANDOM_CHANCE" );   break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "RANDOM_CHANCE" )==0) { m_Code = CODE_RANDOM_CHANCE;}
            
            return( TRUE );
        }
    }
  
    if( conditional_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char*  random_chance::GetInfo ( void )
{
    static big_string   Info;
    
    sml_string  LogicFlag ( GetFlagStr(m_Flag) );

    Info.Set(xfs("%s If A Random Chance of %3.3f%% is TRUE.", LogicFlag.Get(), m_RandomPercent));

    return Info.Get();
}

//=========================================================================
// ON_THIS_NPC : True based on a random chance..
//=========================================================================

condition_create_function on_this_npc::m_Register ( TYPE_CONDITION_ONLY_ON_THIS_NPC, on_this_npc::Create );

//=============================================================================

on_this_npc::on_this_npc(  guid ParentGuid ):  conditional_base(  ParentGuid ), 
m_NPC_Guid(NULL),
m_Code(CODE_ON_THIS_NPC)
{}

//=============================================================================

xbool on_this_npc::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "CONDITION *on_this_npc::Execute" );

    (void) pParent;

    ASSERT( pParent );
    
    const guid* pGuidActor = pParent->GetTriggerActor();
    
    if (pGuidActor == NULL)
        return FALSE;
    
    if (*pGuidActor == m_NPC_Guid)
        return TRUE;
   
    return FALSE; 
}    

//=============================================================================

void on_this_npc::OnEnumProp ( prop_enum& rPropList )
{ 
    //object info 
    rPropList.AddInt ( "Code" , "",  PROP_TYPE_DONT_SHOW  );
    
    rPropList.AddGuid ("NPC Guid" , "The guid of the npc of intrest." );
    
    rPropList.AddEnum( "Logic", 
        "ON_THIS_NPC\0", "Logic available to this condtional.", PROP_TYPE_DONT_SAVE  ); 
    
    conditional_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool on_this_npc::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;
    
    if ( rPropQuery.VarGUID ( "NPC Guid" , m_NPC_Guid ) )
        return TRUE;
    
    if ( rPropQuery.IsVar( "Logic" ) ) 
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Code )
            {
            case CODE_ON_THIS_NPC:    rPropQuery.SetVarEnum( "ON_THIS_NPC" );   break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "ON_THIS_NPC" )==0) { m_Code = CODE_ON_THIS_NPC;}
            
            return( TRUE );
        }
    }
  
    if( conditional_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char*  on_this_npc::GetInfo ( void )
{
    static big_string   Info;
    
    sml_string  LogicFlag ( GetFlagStr(m_Flag) );

    Info.Set(xfs("%s If NPC %x activates this trigger.", LogicFlag.Get(), m_NPC_Guid));

    return Info.Get();
}

//=========================================================================
// ON_THIS_ITEM : True based on a random chance..
//=========================================================================

condition_create_function on_this_item::m_Register ( TYPE_CONDITION_ONLY_ON_THIS_ITEM, on_this_item::Create );

//=============================================================================

on_this_item::on_this_item(  guid ParentGuid ):  conditional_base(  ParentGuid ), 
m_ItemID(0),
m_Code(CODE_ON_THIS_ITEM)
{}

//=============================================================================

xbool on_this_item::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "CONDITION *on_this_item::Execute" );
    
    (void) pParent;

    return FALSE; 
}    

//=============================================================================

void on_this_item::OnEnumProp ( prop_enum& rPropList )
{ 
    //object info 
    rPropList.AddInt ( "Code" , "",  PROP_TYPE_DONT_SHOW  );
    
    rPropList.AddInt ("Item ID" , "The ID of the item of intrest." );
    
    rPropList.AddEnum( "Logic", 
        "ON_THIS_ITEM\0", "Logic available to this condtional.", PROP_TYPE_DONT_SAVE  ); 
    
    conditional_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool on_this_item::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;
    
    if ( rPropQuery.VarInt ( "Item ID" , m_ItemID ) )
        return TRUE;
    
    if ( rPropQuery.IsVar( "Logic" ) ) 
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Code )
            {
            case CODE_ON_THIS_ITEM:    rPropQuery.SetVarEnum( "ON_THIS_ITEM" );   break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "ON_THIS_ITEM" )==0) { m_Code = CODE_ON_THIS_ITEM;}
            
            return( TRUE );
        }
    }
  
    if( conditional_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char*  on_this_item::GetInfo ( void )
{
    static big_string   Info;
    
    sml_string  LogicFlag ( GetFlagStr(m_Flag) );

    Info.Set(xfs("%s If item %d is on the NPC or Player which activates this trigger.", LogicFlag.Get(), m_ItemID));

    return Info.Get();
}

//=========================================================================
// COUNT_THINGS : True based on a random chance..
//=========================================================================

condition_create_function count_things::m_Register ( TYPE_CONDITION_COUNT_THINGS, count_things::Create );

count_things::code_pair count_things::s_PairTable[] = 
{
     code_pair("COUNT_ITEM_ON_PLAYER", count_things::CODE_COUNT_ITEM_ON_PLAYER),
     code_pair("COUNT_ITEM_IN_WORLD",  count_things::CODE_COUNT_ITEM_IN_WORLD ),
     code_pair("COUNT_ITEM_IN_VOLUME", count_things::CODE_COUNT_ITEM_IN_VOLUME),
     code_pair("COUNT_NPCS_IN_VOLUME", count_things::CODE_COUNT_NPCS_IN_VOLUME),
         
     code_pair( k_EnumEndStringConst,  count_things::INVALID_CODES),  //**MUST BE LAST**//
};

count_things::code_table count_things::s_EnumTable ( s_PairTable );

count_things::logic_code_pair count_things::s_LogicPairTable[] = 
{
        logic_code_pair("GREATER_INCLUSIVE", count_things::CODE_GREATER_INCLUSIVE),
        logic_code_pair("LESSER_INCLUSIVE",  count_things::CODE_LESSER_INCLUSIVE),
        logic_code_pair("GREATER",           count_things::CODE_GREATER),
        logic_code_pair("LESSER",            count_things::CODE_LESSER),
        logic_code_pair("EQUAL",             count_things::CODE_EQUAL),
        logic_code_pair("NOT_EQUAL",         count_things::CODE_NOT_EQUAL),
        
        logic_code_pair( k_EnumEndStringConst,count_things::INVALID_LOGIC_CODES),  //**MUST BE LAST**//
};

count_things::logic_code_table count_things::s_LogicEnumTable ( s_LogicPairTable );

//=============================================================================

count_things::count_things(  guid ParentGuid ):  conditional_base(  ParentGuid ), 
m_Value(0),
m_Code(CODE_COUNT_ITEM_ON_PLAYER),
m_LogicCode(CODE_GREATER_INCLUSIVE)
{}

//=============================================================================

xbool count_things::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "CONDITION *count_things::Execute" );
    
    (void) pParent;

    s32 Oject_Count = 0;

    switch ( m_Code )
    {
    case CODE_COUNT_ITEM_ON_PLAYER:    
        break;
    case CODE_COUNT_ITEM_IN_WORLD:     
        break;
    case CODE_COUNT_ITEM_IN_VOLUME:              
        break;
    case CODE_COUNT_NPCS_IN_VOLUME:      
        {
            bbox BBox = pParent->GetBBox();
            
            g_ObjMgr.SelectBBox ( object::ATTR_LIVING, BBox , object::TYPE_HAZMAT );
            
            slot_id	SlotID = SLOT_NULL;
            
            // Check all objects inside of the BBox for the trigger.
            for( SlotID = g_ObjMgr.StartLoop(); SlotID != SLOT_NULL; SlotID = g_ObjMgr.GetNextResult( SlotID ) )
            {
                Oject_Count++;
            }
            
            g_ObjMgr.EndLoop();
        }
        break;
    default:
        ASSERT(0);
        break;
    }
   
    switch ( m_LogicCode )
    {
    case CODE_GREATER_INCLUSIVE:    return Oject_Count >= m_Value;
    case CODE_LESSER_INCLUSIVE:     return Oject_Count <= m_Value;
    case CODE_GREATER:              return Oject_Count >  m_Value;
    case CODE_LESSER:               return Oject_Count <  m_Value;
    case CODE_EQUAL:                return Oject_Count == m_Value;
    case CODE_NOT_EQUAL:            return Oject_Count != m_Value;
    default:
        ASSERT(0);
        break;
    }

    return FALSE; 
}    

//=============================================================================

void count_things::OnEnumProp ( prop_enum& rPropList )
{ 
    //object info 
    rPropList.AddInt ( "Code" , "",  PROP_TYPE_DONT_SHOW  );
 
    rPropList.AddInt ( "Number" , "Number to test against."  );

    rPropList.AddEnum( "Logic", s_EnumTable.BuildString(), "Logic available to this condtional.", PROP_TYPE_DONT_SAVE  ); 
    
    rPropList.AddEnum( "Logic Operatoion", s_LogicEnumTable.BuildString(), "Logic operation available to this condtional.", PROP_TYPE_DONT_SAVE  ); 
    
    conditional_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool count_things::OnProperty	( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;

    if ( rPropQuery.VarInt ( "Number" , m_Value ) )
        return TRUE;

    if ( SMP_UTIL_IsEnumVar<s32,codes>(rPropQuery, "Logic", m_Code, s_EnumTable ) )
        return TRUE;
      
    if ( SMP_UTIL_IsEnumVar<s32,logic_codes>(rPropQuery, "Logic Operatoion", m_LogicCode, s_LogicEnumTable ) )
        return TRUE;

    if( conditional_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char*  count_things::GetInfo ( void )
{
    static big_string   Info;
    
    sml_string  LogicFlag ( GetFlagStr(m_Flag) );
  
    med_string  PreFix;
    med_string  PostFix;
    
    switch ( m_Code )
    {
    case CODE_COUNT_ITEM_ON_PLAYER:    
        PreFix.Set( "??? " );
        break;
    case CODE_COUNT_ITEM_IN_WORLD:     
        PreFix.Set( "??? " );
        break;
    case CODE_COUNT_ITEM_IN_VOLUME:              
        PreFix.Set( "??? " );
        break;
    case CODE_COUNT_NPCS_IN_VOLUME:      
           PreFix.Set( "If the number of NPCs within the trigger volume is " );
        break;
    default:
        ASSERT(0);
        break;
    }
   
    switch ( m_LogicCode )
    {
    case CODE_GREATER_INCLUSIVE:    PostFix.Set( "Greater or Equal to " ); break;
    case CODE_LESSER_INCLUSIVE:     PostFix.Set( "Less than or Equal to " ); break;
    case CODE_GREATER:              PostFix.Set( "Greater than " ); break;
    case CODE_LESSER:               PostFix.Set( "Less than " ); break;
    case CODE_EQUAL:                PostFix.Set( "Equal to " ); break;
    case CODE_NOT_EQUAL:            PostFix.Set( "Not Equal to " ); break;
    default:
        ASSERT(0);
        break;
    }

    Info.Set(xfs("%s %s %s %d", LogicFlag.Get(), PreFix.Get(), PostFix.Get(), m_Value ));

    return Info.Get();
}

//=========================================================================
// CHECK_TIMER : True based on a random chance..
//=========================================================================

condition_create_function check_timer::m_Register( TYPE_CONDITION_CHECK_TIMMER, check_timer::Create );

check_timer::code_pair check_timer::s_PairTable[] = 
{
        code_pair("GREATER_INCLUSIVE", check_timer::CODE_GREATER_INCLUSIVE),
        code_pair("LESSER_INCLUSIVE",  check_timer::CODE_LESSER_INCLUSIVE),
        code_pair("GREATER",           check_timer::CODE_GREATER),
        code_pair("LESSER",            check_timer::CODE_LESSER),
        code_pair("EQUAL",             check_timer::CODE_EQUAL),
        code_pair("NOT_EQUAL",         check_timer::CODE_NOT_EQUAL),
        
        code_pair( k_EnumEndStringConst,check_timer::INVALID_CODES),  //**MUST BE LAST**//
};

check_timer::code_table check_timer::s_EnumTable( s_PairTable );

//=============================================================================

check_timer::check_timer(  guid ParentGuid ):  conditional_base(  ParentGuid ), 
m_TimerHandle(HNULL),
m_Value(0.0f),
m_Code(CODE_GREATER_INCLUSIVE)
{}

//=============================================================================

xbool check_timer::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "CONDITION *check_timer::Execute" );
     
    (void) pParent;

    if ( m_TimerHandle.IsNull() )
        return FALSE;

    f32 CurrentTime = g_VarMgr.ReadTimer( m_TimerHandle );;

    xbool Rval = FALSE;

    switch ( m_Code )
    {
    case CODE_GREATER_INCLUSIVE:    Rval = CurrentTime >= m_Value;break;
    case CODE_LESSER_INCLUSIVE:     Rval = CurrentTime <= m_Value;break;
    case CODE_GREATER:              Rval = CurrentTime >  m_Value;break;
    case CODE_LESSER:               Rval = CurrentTime <  m_Value;break;
    case CODE_EQUAL:                Rval = CurrentTime == m_Value;break;
    case CODE_NOT_EQUAL:            Rval = CurrentTime != m_Value;break;
    default:
        ASSERT(0);
        break;
    }
  
    return Rval; 
}    

//=============================================================================

void check_timer::OnEnumProp ( prop_enum& rPropList )
{ 
    //object info 
    rPropList.AddInt ( "Code" , "",  PROP_TYPE_DONT_SHOW  );
    
    rPropList.AddString ( "Timer Name" , "Name of the timmer to operation upon.", PROP_TYPE_MUST_ENUM );
    
    rPropList.AddFloat ("Time" , "The amount of time in seconds to test the against the timer." );
    
    rPropList.AddEnum( "Logic", s_EnumTable.BuildString(), "Logic available to this condtional.", PROP_TYPE_DONT_SAVE  ); 

    conditional_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool check_timer::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;
    
    if ( rPropQuery.IsVar( "Timer Name" ) ) 
    {
        if( rPropQuery.IsRead() )
        {     
            rPropQuery.SetVarString( m_TimerName.Get() , m_TimerName.MaxLen() );
            
            if ( g_VarMgr.GetTimerHandle( m_TimerName.Get(), &m_TimerHandle) == FALSE )
                m_TimerHandle = HNULL;
        }
        else
        {
            m_TimerName.Set(rPropQuery.GetVarString());
        } 
        
        return( TRUE );
    }

    if ( rPropQuery.VarFloat ( "Time" , m_Value ) )
        return TRUE;
    
    if (m_TimerHandle.IsNonNull())
    {
        if ( SMP_UTIL_IsEnumVar<s32,codes>(rPropQuery, "Logic", m_Code, s_EnumTable ) )
            return TRUE;
    }
    else
    {
        if ( rPropQuery.IsVar( "Logic" ) ) 
        {
            if( rPropQuery.IsRead() )
            {     
                rPropQuery.SetVarEnum( "INVALID" );
            }
 
            return( TRUE );
        }
    }
 
    if( conditional_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char*  check_timer::GetInfo ( void )
{
    static big_string   Info;
    
    if (m_TimerHandle.IsNull())
    {
        Info.Set(xfs("Invalid timer %s, CANNOT EVALUATE.",  m_TimerName.Get() ));
        return Info.Get();
    }

    sml_string  LogicFlag ( GetFlagStr(m_Flag) );

    switch ( m_Code )
    {
    case CODE_GREATER_INCLUSIVE:   
        Info.Set(xfs("%s If '%s' current time is Greater or Equal to %3.3f.", LogicFlag.Get(), m_TimerName.Get(), m_Value));      break;
    case CODE_LESSER_INCLUSIVE:     
        Info.Set(xfs("%s If '%s' current time is Less than or Equal to %3.3f.", LogicFlag.Get(), m_TimerName.Get(), m_Value));    break;
    case CODE_GREATER:              
        Info.Set(xfs("%s If '%s' current time is Greater than %3.3f.", LogicFlag.Get(), m_TimerName.Get(), m_Value));             break;
    case CODE_LESSER:               
        Info.Set(xfs("%s If '%s' current time is Les than %3.3f.", LogicFlag.Get(), m_TimerName.Get(), m_Value));                 break;
    case CODE_EQUAL:                
        Info.Set(xfs("%s If '%s' current time is Equal to %3.3f.", LogicFlag.Get(), m_TimerName.Get(), m_Value));                 break;
    case CODE_NOT_EQUAL:            
        Info.Set(xfs("%s If '%s' current time is Not Equal to %3.3f.", LogicFlag.Get(), m_TimerName.Get(), m_Value));             break;
    default:
        ASSERT(0);
        break;
    }

    return Info.Get();
}


//=========================================================================
// COUNTER : Counts the number of times its been called
//=========================================================================

condition_create_function check_counter::m_Register( TYPE_CONDITION_COUNTER, check_counter::Create );

check_counter::code_pair check_counter::s_PairTable[] = 
{
        code_pair("GREATER_INCLUSIVE", check_counter::CODE_GREATER_INCLUSIVE),
        code_pair("LESSER_INCLUSIVE",  check_counter::CODE_LESSER_INCLUSIVE),
        code_pair("GREATER",           check_counter::CODE_GREATER),
        code_pair("LESSER",            check_counter::CODE_LESSER),
        code_pair("EQUAL",             check_counter::CODE_EQUAL),
        code_pair("NOT_EQUAL",         check_counter::CODE_NOT_EQUAL),
        
        code_pair( k_EnumEndStringConst,check_counter::INVALID_CODES),  //**MUST BE LAST**//
};

check_counter::code_table check_counter::s_EnumTable( s_PairTable );

//=============================================================================

check_counter::check_counter(  guid ParentGuid ):  conditional_base(  ParentGuid ), 
m_Value(0),
m_Count(0),
m_Code(CODE_GREATER_INCLUSIVE)
{}

//=============================================================================

xbool check_counter::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "CONDITION *check_counter::Execute" );
     
    (void) pParent;

    m_Count++;

    switch ( m_Code )
    {
    case CODE_GREATER_INCLUSIVE:    return m_Count >= m_Value;
    case CODE_LESSER_INCLUSIVE:     return m_Count <= m_Value;
    case CODE_GREATER:              return m_Count >  m_Value;
    case CODE_LESSER:               return m_Count <  m_Value;
    case CODE_EQUAL:                return m_Count == m_Value;
    case CODE_NOT_EQUAL:            return m_Count != m_Value;
    default:
        ASSERT(0);
        break;
    }
    
    return FALSE; 
}    

//=============================================================================

void check_counter::OnEnumProp ( prop_enum& rPropList )
{ 
    //object info 
    rPropList.AddInt ( "Code" , "",  PROP_TYPE_DONT_SHOW  );
   
    rPropList.AddInt ("Number" , "The number to check against the execution count." ); 
      
    rPropList.AddInt ("Count" , "",  PROP_TYPE_DONT_SHOW  );
  
    rPropList.AddEnum( "Logic", s_EnumTable.BuildString(), "Logic available to this condtional.", PROP_TYPE_DONT_SAVE  ); 
  
    conditional_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool check_counter::OnProperty	( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;
 
    if ( rPropQuery.VarInt ( "Count" , m_Count ) )
        return TRUE;

    if ( rPropQuery.VarInt ( "Number" , m_Value ) )
        return TRUE;
    
    if ( SMP_UTIL_IsEnumVar<s32,codes>(rPropQuery, "Logic", m_Code, s_EnumTable ) )
        return TRUE;
 
    if( conditional_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char*  check_counter::GetInfo ( void )
{
    static big_string   Info;
    
    sml_string  LogicFlag ( GetFlagStr(m_Flag) );

    switch ( m_Code )
    {
    case CODE_GREATER_INCLUSIVE:   
        Info.Set(xfs("%s If our execution count is Greater or Equal to %d.", LogicFlag.Get(), m_Value));         break;
    case CODE_LESSER_INCLUSIVE:     
        Info.Set(xfs("%s If our execution count is Less than or Equal to %d.", LogicFlag.Get(), m_Value));       break;
    case CODE_GREATER:              
        Info.Set(xfs("%s If our execution count is Greater than %d.", LogicFlag.Get(),  m_Value));               break;
    case CODE_LESSER:               
        Info.Set(xfs("%s If our execution count is Les than %d.", LogicFlag.Get(), m_Value));                    break;
    case CODE_EQUAL:                
        Info.Set(xfs("%s If our execution count is Equal to %d.", LogicFlag.Get(), m_Value));                    break;
    case CODE_NOT_EQUAL:            
        Info.Set(xfs("%s If our execution count is Not Equal to %d.", LogicFlag.Get(), m_Value));                break;
    default:
        ASSERT(0);
        break;
    }

    return Info.Get();
}