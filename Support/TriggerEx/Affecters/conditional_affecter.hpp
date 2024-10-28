///////////////////////////////////////////////////////////////////////////////
//
//  conditional_affecter.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _conditional_affecter_
#define _conditional_affecter_

//=========================================================================
// INCLUDES
//=========================================================================

#include "MiscUtils\SimpleUtils.hpp"
#include "..\Support\TriggerEx\TriggerEx_Conditionals.hpp"

//=========================================================================
// Check Property
//=========================================================================

class trigger_ex_object;

class conditional_affecter
{
public:

    CREATE_RTTI_BASE( conditional_affecter );

    enum evaluation
    {
        EVAL_COMMIT_IF,
        EVAL_COMMIT_ELSE,
        EVAL_NOT_MET
    };    
    
                conditional_affecter        ( xbool bAllowElse, s32 PropertyLevel );
    virtual     ~conditional_affecter       ( void );

    void        EnumPropSelector            ( prop_enum& rList );
    xbool       OnPropertySelector          ( prop_query& rPropQuery );

    void        EnumPropConditionData       ( prop_enum& rList );
    xbool       OnPropertyConditionData     ( prop_query& rPropQuery );

    void        EnumPropConditions          ( prop_enum& rList );
    void        EnumPropElseConditions      ( prop_enum& rList );
    xbool       OnPropertyConditions        ( prop_query& rPropQuery );

    xbool       CheckElseState              ( void );
    evaluation  EvaluateConditions          ( guid TriggerGuid );

    inline s32  GetConditionCount           ( void ) { return m_NumIfConditions; }
    inline s32  GetElseConditionCount       ( void ) { return m_NumElseConditions; }
    s32         GetConditionIndex           ( conditional_ex_base* pCondition );
    void        SetConditionIndex           ( conditional_ex_base* pCondition, s32 Index );
    void        RemoveCondition             ( conditional_ex_base* pCondition, xbool bAndDelete = TRUE );
    xbool       IsFirstInConditionSet       ( conditional_ex_base* pCondition );

    xbool       CanSwitchElse               ( conditional_ex_base* pCondition );
    void        SwitchElse                  ( conditional_ex_base* pCondition );
   
    void        RemoveAllConditions         ( void );

#ifndef X_RETAIL
    void        OnDebugRender               ( void );
#endif // X_RETAIL

#ifdef X_EDITOR
    virtual         s32     OnValidateConditions        ( trigger_ex_object& Trigger, xstring& ConditionType, s32 nConditions, conditional_ex_base* Conditions[], xstring& ErrorMsg );
    virtual         s32     OnValidateProperties        ( trigger_ex_object& Trigger, xstring& ErrorMsg );
#endif

protected:
    
    enum { MAX_CONDITION_ARRAY_SIZE = 12 };


    xbool       AddCondition                ( conditional_ex_base::conditional_ex_types ConditionType , s32 Number, xbool bElse );

    xbool       EvaluateMainConditions      ( guid TriggerGuid );
    xbool       EvaluateElseConditions      ( guid TriggerGuid );

protected:

//=========================================================================
// Internal data structures and classes...
//=========================================================================

    class  condition_selector : public prop_interface
    {
    public:
                                condition_selector( void );
                        void    Init                                 ( conditional_affecter* pParent );
        virtual			void	OnEnumProp				             ( prop_enum& rList );
        virtual			xbool	OnProperty				             ( prop_query& rPropQuery );

        conditional_ex_base::conditional_ex_types   m_ConditionType;     // Condtion type currently selected
        xbool                                       m_Active;            // State Flag to determine if the selector is active..
        conditional_affecter*                       m_Parent;            // Parent of the selector
    };

    friend condition_selector;
    
protected:

    xbool                   m_AllowElseInfo;                //does this condition allow else info?
    s32                     m_PropIndexLevel;               //in the property grid, low many arrays deep is this?

    conditional_ex_base*    m_IfConditions[MAX_CONDITION_ARRAY_SIZE];   // Ptr array for condtions
    conditional_ex_base*    m_ElseConditions[MAX_CONDITION_ARRAY_SIZE]; // Ptr array for condtions
    s32                     m_NumIfConditions;              // Number of if conditions..
    s32                     m_NumElseConditions;            // Number of else conditions..

#ifdef X_EDITOR
    condition_selector      m_Selector;                     // Selector object used to actually determine the type of conditions to create..
#endif // X_EDITOR
};

#endif
