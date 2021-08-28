///////////////////////////////////////////////////////////////////////////////
//
//  TriggerEx_Conditionals.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGEREX_CONDITIONALS_
#define _TRIGGEREX_CONDITIONALS_

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_types.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"

#ifdef X_EDITOR
#include "Obj_mgr\obj_mgr.hpp"
#endif

//=========================================================================
// CONDITIONAL_BASE
//=========================================================================
struct condition_ex_create_function;
class  conditional_affecter;
class object_affecter;

class  conditional_ex_base : public prop_interface
{  
    
public:

    //Note ** Do not remove depreciated enums from list. The trigger object saves out a list
    //of conditions it contains and uses the enum cast to a s32 to identify the action. Changing
    //the order of the enums or removing enums in the middle will break this listing..**//

    enum conditional_ex_types
    {
        INVALID_CONDITIONAL_TYPES = -1,
 
        TYPE_CONDITION_PLACEHOLDER      = 0,               
        TYPE_CONDITION_OBJECT_EXISTS    = 1,              
        TYPE_CONDITION_RANDOM_CHANCE    = 2,             
        TYPE_CONDITION_BUTTON_STATE     = 3,              
        TYPE_CONDITION_PLAYER_HAS       = 4,                
            
        TYPE_CONDITION_CHECK_GLOBAL     = 5,              
        TYPE_CONDITION_CHECK_PROPERTY   = 6,            
      
        TYPE_CONDITION_LOOKING_AT_FOCUS = 7,            
        TYPE_CONDITION_WITHIN_RANGE     = 8,
        TYPE_CONDITION_LINE_OF_SIGHT    = 9,

        TYPE_CONDITION_CHECK_FOCUS_OBJECT = 10,

        TYPE_CONDITION_CHECK_HEALTH = 11,

        TYPE_CONDITION_IS_CENSORED = 12,

        CONDITIONAL_TYPES_END
    };
    
    enum conditional_ex_flags
    {
        INVALID_CONDITIONAL_FLAGS = -1,
            
        FLAG_OR,
        FLAG_AND,
        
        CONDITIONAL_FLAGS_END
    };

    enum { MAX_CONDITION_ENUM_BUFFER = 255 };
    
                    conditional_ex_base( conditional_affecter* pParent );
    virtual        ~conditional_ex_base();
    
    virtual         conditional_ex_types    GetType             ( void )   { return INVALID_CONDITIONAL_TYPES;}
    virtual         const char*             GetTypeInfo         ( void )   { return "Base condition class, null funtionality"; } 
    virtual         const char*             GetDescription      ( void )   { return "\0"; }
    virtual         xbool                   Execute             ( guid TriggerGuid ) = 0;    
    virtual			void	                OnEnumProp	        ( prop_enum& rList );
    virtual			xbool	                OnProperty	        ( prop_query& rPropQuery );

#ifdef X_EDITOR
    // Implement any of these functions in your derived classes for property validation
    virtual         object_affecter*        GetObjectRef0       ( xstring& Desc )                    { (void)Desc; return NULL; }
    virtual         object_affecter*        GetObjectRef1       ( xstring& Desc )                    { (void)Desc; return NULL; }
    virtual         s32*                    GetAnimRef          ( xstring& Desc, s32& AnimName )     { (void)Desc; (void)AnimName; return NULL; }
    virtual         rhandle<char>*          GetSoundRef         ( xstring& Desc, s32& SoundName )    { (void)Desc; (void)SoundName; return NULL; }
    virtual         s32*                    GetGlobalRef        ( xstring& Desc )                    { (void)Desc; return NULL; }
    virtual         s32*                    GetPropertyRef      ( xstring& Desc, s32& PropertyType ) { (void)Desc; (void)PropertyType; return NULL; }
    virtual         s32*                    GetTemplateRef      ( xstring& Desc )                    { (void)Desc;  return NULL; }
    virtual         guid*                   GetGuidRef0         ( xstring& Desc )                    { (void)Desc;  return NULL; }
    virtual         guid*                   GetGuidRef1         ( xstring& Desc )                    { (void)Desc;  return NULL; }
#endif

#ifndef X_RETAIL
    virtual         void                    OnDebugRender       ( s32 Index ) { (void) Index; /*no-op*/ }
#endif // X_RETAIL

    inline          conditional_ex_flags    GetFlag             ( void ) { return m_LogicFlag; }
    inline          xbool                   GetElse             ( void ) { return m_ElseFlag; }
    inline          void                    SetElse             ( xbool ElseFlag ) { m_ElseFlag = ElseFlag; }
                    const char*             GetFlagStr          ( void );
    inline          void                    AllowElse           ( xbool bAllowElse ) { m_bAllowElse = bAllowElse; }

public:
    
    static          conditional_ex_base*        CreateCondition     ( const conditional_ex_types& rType , conditional_affecter* pParent );
      
    static enum_table<conditional_ex_types>     m_ConditionalAllEnum;       // Enumeration of the condtion types..
    static enum_table<conditional_ex_types>     m_ConditionalGeneralEnum;   // Enumeration of the condtion types..
    static enum_table<conditional_ex_types>     m_ConditionalPlayerEnum;    // Enumeration of the condtion types..

    static          void                        RegisterCreationFunction( condition_ex_create_function* pCreate );

protected:

    xbool                                       m_bAllowElse;       // Do we allow else conditions?
    conditional_ex_flags                        m_LogicFlag;        // Logic conditional flags for this condition
    xbool                                       m_ElseFlag;         // Flag if this condition is an else condition
    conditional_affecter*                       m_pParent;          // Guid of the object which holds this action
    static condition_ex_create_function*        m_CreateHead;
};

//=========================================================================
// CONDITION_CREATE_FUNCTION : used for automatic registeration of creation function for conditions..
//=========================================================================

typedef conditional_ex_base* create_conditon_ex_fn ( conditional_affecter* pParent );

struct condition_ex_create_function
{
    condition_ex_create_function( conditional_ex_base::conditional_ex_types Type, create_conditon_ex_fn* pCreateCondition )
    {
        m_Type              = Type;
        m_pCreateCondition  = pCreateCondition;
        m_Next              = NULL;
    }

    conditional_ex_base::conditional_ex_types    m_Type;
    create_conditon_ex_fn*                       m_pCreateCondition;
    condition_ex_create_function*                m_Next;
};

template < class ConditionExClass > 
    struct automatic_condition_ex_registeration
{
    automatic_condition_ex_registeration( void )
    {
        static condition_ex_create_function m_CreationObject( ConditionExClass::GetTypeStatic(), Create );

        conditional_ex_base::RegisterCreationFunction( &m_CreationObject );
    }
    
    static conditional_ex_base*  Create( conditional_affecter* pParent ) { return new ConditionExClass(pParent); }
};

#endif
