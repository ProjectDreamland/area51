///////////////////////////////////////////////////////////////////////////////
//
//  conditional_affecter.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "conditional_affecter.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "..\Support\Globals\Global_Variables_Manager.hpp"

#ifdef X_EDITOR
#include "TriggerEx\TriggerEx_Object.hpp"
#endif

static const s32        MAX_BIT_SIZE_OF_FLAGS = 32;
static const s32        MAX_STRING_LEN = 255;

//=========================================================================
// INTERNAL CLASSES
//=========================================================================

conditional_affecter::condition_selector::condition_selector( ) : 
m_ConditionType( conditional_ex_base::TYPE_CONDITION_OBJECT_EXISTS ),
m_Active( FALSE ), 
m_Parent( NULL )
{
}

//=============================================================================

void  conditional_affecter::condition_selector::Init( conditional_affecter* pParent )
{
    m_Parent = pParent;
}

//=============================================================================

void  conditional_affecter::condition_selector::OnEnumProp ( prop_enum& rPropList )
{
    rPropList.PropEnumHeader  ( "ConditionSelect",                 "Select the type of condition to add.", PROP_TYPE_HEADER);
     
    rPropList.PropEnumEnum    ( "ConditionSelect\\General Conditions", conditional_ex_base::m_ConditionalGeneralEnum.BuildString(),    "Types of General condtions available." , PROP_TYPE_DONT_SAVE );  
    rPropList.PropEnumEnum    ( "ConditionSelect\\Player Conditions",  conditional_ex_base::m_ConditionalPlayerEnum.BuildString(),     "Types of Player Specific condtions available." , PROP_TYPE_DONT_SAVE );  
    rPropList.PropEnumButton  ( "ConditionSelect\\If Condition",  "Adds a new Condition into the list.",  PROP_TYPE_MUST_ENUM );
    rPropList.PropEnumButton  ( "ConditionSelect\\Else Condition","Adds a new Condition into the list.",  PROP_TYPE_MUST_ENUM );
}

//=============================================================================

xbool conditional_affecter::condition_selector::OnProperty ( prop_query& rPropQuery )
{
    /////////////////////////////////////////////////////////////////////////////////////////////////
    // Selector to add conditions and actions
    /////////////////////////////////////////////////////////////////////////////////////////////////

    if( rPropQuery.IsVar( "ConditionSelect\\If Condition" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Add as If Condition" );
        }
        else
        {
            ASSERT( m_Parent );
            if (m_Parent->AddCondition( m_ConditionType, m_Parent->m_NumIfConditions, FALSE ) )
            {
                m_Parent->m_NumIfConditions++;
            }
        }
        
        return TRUE;
    }

    if( rPropQuery.IsVar( "ConditionSelect\\Else Condition" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarButton( "Add as Else Condition" );
        }
        else
        {
            ASSERT( m_Parent );
            if (m_Parent->AddCondition( m_ConditionType, m_Parent->m_NumElseConditions, TRUE ) )
            {
                m_Parent->m_NumElseConditions++;
            }
        }
        
        return TRUE;
    }

    if ( 
        rPropQuery.IsVar( "ConditionSelect\\General Conditions" )     |  
        rPropQuery.IsVar( "ConditionSelect\\Player Conditions"  )       
        )
    {
        if( rPropQuery.IsRead() )
        {  
            if ( conditional_ex_base::m_ConditionalAllEnum.DoesValueExist( m_ConditionType ) )
            {
                rPropQuery.SetVarEnum( conditional_ex_base::m_ConditionalAllEnum.GetString( m_ConditionType ) );
            }
            else
            {
                rPropQuery.SetVarEnum( "INVALID" );
            }           
        }
        else
        {
            conditional_ex_base::conditional_ex_types ConditionType;
            if( conditional_ex_base::m_ConditionalAllEnum.GetValue( rPropQuery.GetVarEnum(), ConditionType ) )
            {
                m_ConditionType = ConditionType;
            }
        }
        
        return( TRUE );
    } 
    
    return FALSE;
}

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

conditional_affecter::conditional_affecter( xbool bAllowElse, s32 PropertyLevel ) :
m_AllowElseInfo(bAllowElse),
m_PropIndexLevel(PropertyLevel),
m_NumIfConditions(0),
m_NumElseConditions(0)
{
#ifdef X_EDITOR
    m_Selector.Init( this );
#endif // X_EDITOR
   
    for( s32 i=0; i< MAX_CONDITION_ARRAY_SIZE ; i++ )
    {
        m_IfConditions[i] = NULL;
        m_ElseConditions[i] = NULL;
    }
}

//=========================================================================

conditional_affecter::~conditional_affecter(void)
{
    for( s32 j=0; j< MAX_CONDITION_ARRAY_SIZE ; j++ )
    { 
        if (m_IfConditions[j] != NULL)
        {
            delete m_IfConditions[j];
            m_IfConditions[j] = NULL;
        }

        if (m_ElseConditions[j] != NULL)
        {
            delete m_ElseConditions[j];
            m_ElseConditions[j] = NULL;
        }
    }
}

//=============================================================================
//=============================================================================

#ifdef X_EDITOR
void conditional_affecter::EnumPropSelector ( prop_enum& rPropList )
{
    m_Selector.OnEnumProp(rPropList);
}

xbool conditional_affecter::OnPropertySelector( prop_query& rPropQuery )
{
    return m_Selector.OnProperty(rPropQuery);
}
#endif // X_EDITOR

//=============================================================================
//=============================================================================

void conditional_affecter::EnumPropConditionData ( prop_enum& rPropList )
{
    s32 iHeader = -1;
    if (m_PropIndexLevel == 0)
    {
        //Root Data
        rPropList.PropEnumHeader( "RootConditionData", "", PROP_TYPE_DONT_SHOW );
        iHeader = rPropList.PushPath( "RootConditionData\\" );        
    }
    else
    {
        //Child Data
        rPropList.PropEnumHeader( "MetaConditionData", "", PROP_TYPE_DONT_SHOW );
        iHeader = rPropList.PushPath( "MetaConditionData\\" );        
    }

    for( s32 i=0; i< MAX_CONDITION_ARRAY_SIZE ; i++ )
    { 
        if (m_IfConditions[i] != NULL)
        {
            rPropList.PropEnumInt( xfs("CIfType[%d]", i ), "", PROP_TYPE_DONT_SHOW );
        }

        if (m_ElseConditions[i] != NULL)
        {
            rPropList.PropEnumInt( xfs("CElseType[%d]", i ), "", PROP_TYPE_DONT_SHOW );
        }
    }

    rPropList.PropEnumInt     ( "NumIfConditions",    "", PROP_TYPE_DONT_SHOW );
    rPropList.PropEnumInt     ( "NumElseConditions",  "", PROP_TYPE_DONT_SHOW );

    rPropList.PopPath( iHeader );
}

//===========================================================================

xbool conditional_affecter::OnPropertyConditionData( prop_query& rPropQuery )
{
    s32 iHeader = -1;
    if (m_PropIndexLevel == 0)
    {
        //Root Data
        if( rPropQuery.IsSimilarPath( "RootConditionData" ) )
            iHeader = rPropQuery.PushPath( "RootConditionData\\" );   
        else
            return FALSE;
    }
    else
    {
        //Child Data
        if( rPropQuery.IsSimilarPath( "MetaConditionData" ) )
            iHeader = rPropQuery.PushPath( "MetaConditionData\\" );   
        else
            return FALSE;
    }
       
    if( rPropQuery.IsSimilarPath( "CIfType[" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(m_PropIndexLevel);
        ASSERT( iIndex < MAX_CONDITION_ARRAY_SIZE && iIndex >= 0 );
    
        if( m_IfConditions[iIndex] != NULL && rPropQuery.IsRead() )
        {
            rPropQuery.SetVarInt( m_IfConditions[iIndex]->GetType() );
        }
        else if( !rPropQuery.IsRead() )
        { 
            s32 ConditionType = -1;
            ConditionType = rPropQuery.GetVarInt();
            AddCondition( (conditional_ex_base::conditional_ex_types) ConditionType, iIndex, FALSE );
        }
    
        return TRUE;
    }

    if( rPropQuery.IsSimilarPath( "CElseType[" ) )
    {
        s32 iIndex = rPropQuery.GetIndex(m_PropIndexLevel);
        ASSERT( iIndex < MAX_CONDITION_ARRAY_SIZE && iIndex >= 0 );
    
        if( m_ElseConditions[iIndex] != NULL && rPropQuery.IsRead() )
        {
            rPropQuery.SetVarInt( m_ElseConditions[iIndex]->GetType() );
        }
        else if( !rPropQuery.IsRead() )
        { 
            s32 ConditionType = -1;
            ConditionType = rPropQuery.GetVarInt();
            AddCondition( (conditional_ex_base::conditional_ex_types) ConditionType, iIndex, TRUE );
        }
    
        return TRUE;
    }

    if ( rPropQuery.VarInt( "NumIfConditions",        m_NumIfConditions ))
        return TRUE; 
  
    if ( rPropQuery.VarInt( "NumElseConditions",      m_NumElseConditions ))
        return TRUE;
    
    rPropQuery.PopPath( iHeader );

    return FALSE;
}

//=============================================================================

void conditional_affecter::EnumPropConditions ( prop_enum& rPropList )
{
    if ( GetConditionCount() > 0 )
    {
        //we have at least 1 condition

        if (m_PropIndexLevel == 0)
        {
            //only add this for the root conditions
            rPropList.PropEnumHeader  ( "If", "All the Conditions for this Item.", PROP_TYPE_HEADER);

            for( s32 i = 0; i< MAX_CONDITION_ARRAY_SIZE ; i++ )
            { 
                if (m_IfConditions[i]) 
                {
                    ASSERT(m_IfConditions[i]->GetElse() == FALSE);

                    rPropList.PropEnumString( xfs("If\\Condition[%d]", i) , 
                        m_IfConditions[i]->GetTypeInfo(), PROP_TYPE_HEADER );        
            
                    s32 iHeader = rPropList.PushPath( xfs("If\\Condition[%d]\\", i) );        

                    m_IfConditions[i]->OnEnumProp(rPropList);

                    rPropList.PopPath( iHeader );
                }
            }
        }
        else //non-root
        {
            for( s32 i = 0; i< MAX_CONDITION_ARRAY_SIZE ; i++ )
            { 
                if (m_IfConditions[i]) 
                {
                    ASSERT(m_IfConditions[i]->GetElse() == FALSE);

                    rPropList.PropEnumString( xfs("Conditional[%d]", i) , 
                        m_IfConditions[i]->GetTypeInfo(), PROP_TYPE_HEADER );        
            
                    s32 iHeader = rPropList.PushPath( xfs("Conditional[%d]\\", i) );        

                    m_IfConditions[i]->OnEnumProp(rPropList);

                    rPropList.PopPath( iHeader );
                }
            }
        }
    }
}

//=============================================================================

void conditional_affecter::EnumPropElseConditions  ( prop_enum& rPropList )
{
    if ( GetElseConditionCount() > 0 )
    {
        //we have at least 1 condition

        if (m_PropIndexLevel == 0)
        {
            //only add this for the root conditions
            rPropList.PropEnumHeader  ( "Else If", "All the Else Conditions for this Item.", PROP_TYPE_HEADER);

            for( s32 i = 0; i< MAX_CONDITION_ARRAY_SIZE ; i++ )
            { 
                if ( m_ElseConditions[i] )
                {
                    ASSERT(m_ElseConditions[i]->GetElse() == TRUE);

                    rPropList.PropEnumString( xfs("Else If\\Else Condition[%d]", i) , 
                        m_ElseConditions[i]->GetTypeInfo(), PROP_TYPE_HEADER );        
            
                    s32 iHeader = rPropList.PushPath( xfs("Else If\\Else Condition[%d]\\", i) );        
            
                    m_ElseConditions[i]->OnEnumProp(rPropList);
            
                    rPropList.PopPath( iHeader );
                }
            } 
        }
        else //non - root
        {
            for( s32 i = 0; i< MAX_CONDITION_ARRAY_SIZE ; i++ )
            { 
                if ( m_ElseConditions[i] )
                {
                    ASSERT(m_ElseConditions[i]->GetElse() == TRUE);

                    rPropList.PropEnumString( xfs("Else Conditional[%d]", i) , 
                        m_ElseConditions[i]->GetTypeInfo(), PROP_TYPE_HEADER );        
            
                    s32 iHeader = rPropList.PushPath( xfs("Else Conditional[%d]\\", i) );        
            
                    m_ElseConditions[i]->OnEnumProp(rPropList);
            
                    rPropList.PopPath( iHeader );
                }
            } 
        }
    }
}

//=============================================================================

xbool conditional_affecter::OnPropertyConditions ( prop_query& rPropQuery )
{  
    if ( m_PropIndexLevel == 0 )
    {
        if( rPropQuery.IsSimilarPath( "If\\Condition" ) )
        {
            s32 iIndex = rPropQuery.GetIndex(m_PropIndexLevel);
        
            ASSERT( iIndex < MAX_CONDITION_ARRAY_SIZE && iIndex >= 0 && m_IfConditions[iIndex] );
        
            if ( rPropQuery.IsVar( "If\\Condition[]" ) )
            { 
                if( rPropQuery.IsRead() )
                {
                    if ( IsFirstInConditionSet(m_IfConditions[iIndex]) )
                        rPropQuery.SetVarString(m_IfConditions[iIndex]->GetDescription(), MAX_STRING_LEN );               
                    else
                        rPropQuery.SetVarString(xfs("%s %s", m_IfConditions[iIndex]->GetFlagStr(), m_IfConditions[iIndex]->GetDescription()), MAX_STRING_LEN);
                }
            
                return TRUE;
            }
        
            if (m_IfConditions[iIndex])
            {
                s32 iHeader = rPropQuery.PushPath( "If\\Condition[]\\" );        
            
                if( m_IfConditions[iIndex]->OnProperty(rPropQuery) )
                {
                    rPropQuery.PopPath( iHeader );
                    return TRUE;
                }  
            
                rPropQuery.PopPath( iHeader );
            }
        }
    
        if( rPropQuery.IsSimilarPath( "Else If\\Else Condition" ) )
        {
            s32 iIndex = rPropQuery.GetIndex(m_PropIndexLevel);
        
            ASSERT( iIndex < MAX_CONDITION_ARRAY_SIZE && iIndex >= 0 && m_ElseConditions[iIndex] );
        
            if ( rPropQuery.IsVar( "Else If\\Else Condition[]" ) )
            { 
                if( rPropQuery.IsRead() )
                {
                    if ( IsFirstInConditionSet(m_ElseConditions[iIndex]) )
                        rPropQuery.SetVarString(m_ElseConditions[iIndex]->GetDescription(), MAX_STRING_LEN );               
                    else
                        rPropQuery.SetVarString(xfs("%s %s", m_ElseConditions[iIndex]->GetFlagStr(), m_ElseConditions[iIndex]->GetDescription()), MAX_STRING_LEN);
                }
            
                return TRUE;
            }
        
            if (m_ElseConditions[iIndex])
            {
                s32 iHeader = rPropQuery.PushPath( "Else If\\Else Condition[]\\" );        
            
                if( m_ElseConditions[iIndex]->OnProperty(rPropQuery) )
                {
                    rPropQuery.PopPath( iHeader );
                    return TRUE;
                }  
            
                rPropQuery.PopPath( iHeader );
            }
        }
    }
    else //non-root
    {
        if( rPropQuery.IsSimilarPath( "Conditional" ) )
        {
            s32 iIndex = rPropQuery.GetIndex(m_PropIndexLevel);
        
            ASSERT( iIndex < MAX_CONDITION_ARRAY_SIZE && iIndex >= 0 && m_IfConditions[iIndex] );
        
            if ( rPropQuery.IsVar( "Conditional[]" ) )
            { 
                if( rPropQuery.IsRead() )
                {
                    if ( IsFirstInConditionSet(m_IfConditions[iIndex]) )
                        rPropQuery.SetVarString(m_IfConditions[iIndex]->GetDescription(), MAX_STRING_LEN );               
                    else
                        rPropQuery.SetVarString(xfs("%s %s", m_IfConditions[iIndex]->GetFlagStr(), m_IfConditions[iIndex]->GetDescription()), MAX_STRING_LEN);
                }
            
                return TRUE;
            }
        
            if (m_IfConditions[iIndex])
            {
                s32 iHeader = rPropQuery.PushPath( "Conditional[]\\" );        
            
                if( m_IfConditions[iIndex]->OnProperty(rPropQuery) )
                {
                    rPropQuery.PopPath( iHeader );
                    return TRUE;
                }  
            
                rPropQuery.PopPath( iHeader );
            }
        }
    
        if( rPropQuery.IsSimilarPath( "Else Conditional" ) )
        {
            s32 iIndex = rPropQuery.GetIndex(m_PropIndexLevel);
        
            ASSERT( iIndex < MAX_CONDITION_ARRAY_SIZE && iIndex >= 0 && m_ElseConditions[iIndex] );
        
            if ( rPropQuery.IsVar( "Else Conditional[]" ) )
            { 
                if( rPropQuery.IsRead() )
                {
                    if ( IsFirstInConditionSet(m_ElseConditions[iIndex]) )
                        rPropQuery.SetVarString(m_ElseConditions[iIndex]->GetDescription(), MAX_STRING_LEN );               
                    else
                        rPropQuery.SetVarString(xfs("%s %s", m_ElseConditions[iIndex]->GetFlagStr(), m_ElseConditions[iIndex]->GetDescription()), MAX_STRING_LEN);
                }
            
                return TRUE;
            }
        
            if (m_ElseConditions[iIndex])
            {
                s32 iHeader = rPropQuery.PushPath( "Else Conditional[]\\" );        
            
                if( m_ElseConditions[iIndex]->OnProperty(rPropQuery) )
                {
                    rPropQuery.PopPath( iHeader );
                    return TRUE;
                }  
            
                rPropQuery.PopPath( iHeader );
            }
        }
    }
    
    return FALSE;
}

//=============================================================================

xbool conditional_affecter::CheckElseState( void )
{
    //return true if we are a trigger which uses an else block...
    if (m_NumElseConditions > 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//=============================================================================

conditional_affecter::evaluation conditional_affecter::EvaluateConditions ( guid TriggerGuid )
{
    //evaluate all conditions with respect to the else flags...
    if (EvaluateMainConditions(TriggerGuid))
    {
        return EVAL_COMMIT_IF;
    }
    else if ( EvaluateElseConditions(TriggerGuid) )
    {
        return EVAL_COMMIT_ELSE;
    }

    return EVAL_NOT_MET;
}

//=============================================================================

xbool conditional_affecter::EvaluateMainConditions   ( guid TriggerGuid )
{
    xbool   Rval = TRUE;

    for (s32 i = 0 ; i < MAX_CONDITION_ARRAY_SIZE; i++ )
    {
        ASSERT( i < MAX_BIT_SIZE_OF_FLAGS );
        
        if ( m_IfConditions[i] == NULL )
            continue;
        
        ASSERT(m_IfConditions[i]->GetElse() == FALSE);

        if ( ( !Rval ) && ( m_IfConditions[i]->GetFlag() == conditional_ex_base::FLAG_AND ) )
        {
            //this is an and condition, and we the current state is failure, so
            //no reason to perform this condition check (save some cpu)
            continue;
        }

        if (m_IfConditions[i]->Execute(TriggerGuid))
        {
            //condition met, if and leave alone, if or the set return val true
            if ( m_IfConditions[i]->GetFlag() == conditional_ex_base::FLAG_OR )
            {
                Rval = TRUE;
            }
        }
        else
        {
            //condition failure, if and set FALSE, if or no change unless it is first in set
            if ( ( m_IfConditions[i]->GetFlag() == conditional_ex_base::FLAG_AND ) ||
                 ( ( i == 0 ) && ( m_IfConditions[i]->GetFlag() == conditional_ex_base::FLAG_OR ) )
               )
            {
                Rval = FALSE;
            }
        }
    }

    return Rval;
}

//=============================================================================

xbool conditional_affecter::EvaluateElseConditions   ( guid TriggerGuid )
{  
    xbool   Rval = TRUE;

    for (s32 i = 0 ; i < MAX_CONDITION_ARRAY_SIZE; i++ )
    {
        ASSERT( i < MAX_BIT_SIZE_OF_FLAGS );
        
        if ( m_ElseConditions[i] == NULL )
            continue;

        ASSERT(m_ElseConditions[i]->GetElse() == TRUE);

        if ( ( !Rval ) && ( m_ElseConditions[i]->GetFlag() == conditional_ex_base::FLAG_AND ) )
        {
            //this is an and condition, and we the current state is failure, so
            //no reason to perform this condition check (save some cpu)
            continue;
        }

        if (m_ElseConditions[i]->Execute(TriggerGuid))
        {
            //condition met, if and leave alone, if or the set return val true
            if ( m_ElseConditions[i]->GetFlag() == conditional_ex_base::FLAG_OR )
            {
                Rval = TRUE;
            }
        }
        else
        {
            //condition failure, if and set FALSE, if or no change unless it is first in set
            if ( ( m_ElseConditions[i]->GetFlag() == conditional_ex_base::FLAG_AND ) ||
                 ( ( i == 0 ) && ( m_ElseConditions[i]->GetFlag() == conditional_ex_base::FLAG_OR ) )
               )
            {
                Rval = FALSE;
            }
        }    
    }
    
    return Rval;
}

//=============================================================================

xbool conditional_affecter::AddCondition ( conditional_ex_base::conditional_ex_types ConditionType, s32 Number, xbool bElse )
{
    if ( Number >= 0 && Number < MAX_CONDITION_ARRAY_SIZE )
    {
        if (!bElse)
        {
            if (m_IfConditions[Number])
            {
                delete m_IfConditions[Number];
                m_IfConditions[Number] = NULL;
            }

            m_IfConditions[Number] = conditional_ex_base::CreateCondition( ConditionType, this );
            ASSERT(m_IfConditions[Number]);
            m_IfConditions[Number]->SetElse(FALSE);
            m_IfConditions[Number]->AllowElse(m_AllowElseInfo); 
        }
        else
        {
            if (m_ElseConditions[Number])
            {
                delete m_ElseConditions[Number];
                m_ElseConditions[Number] = NULL;
            }

            m_ElseConditions[Number] = conditional_ex_base::CreateCondition( ConditionType, this );
            ASSERT(m_ElseConditions[Number]);
            m_ElseConditions[Number]->SetElse(TRUE);
            m_ElseConditions[Number]->AllowElse(m_AllowElseInfo); 
        }
        return TRUE;
    }
    else
    {
        x_try;
        x_throw("You can not add any more conditions to this item!");
        x_catch_display;
        return FALSE;
    }
}

//=============================================================================

s32 conditional_affecter::GetConditionIndex ( conditional_ex_base* pCondition )
{
    for( s32 i = 0; i< MAX_CONDITION_ARRAY_SIZE ; i++ )
    { 
        if (!pCondition->GetElse())
        {
            if (m_IfConditions[i] == pCondition)
            {
                return i;
            }
        }
        else
        {
            if (m_ElseConditions[i] == pCondition)
            {
                return i;
            }
        }
    }
    
    return -1;
}

//=============================================================================

void conditional_affecter::SetConditionIndex ( conditional_ex_base* pCondition, s32 Index )
{
    if (Index < 0 || Index >= MAX_CONDITION_ARRAY_SIZE )
    {
        ASSERT(FALSE);
        return;
    }

    if (!pCondition)
        return;

    if (!pCondition->GetElse())
    {
        //if the index is too high, set it to the highest used index
        if (Index >= m_NumIfConditions)
            Index = m_NumIfConditions - 1;

        for( s32 i = 0; i< MAX_CONDITION_ARRAY_SIZE ; i++ )
        { 
            if (m_IfConditions[i] == pCondition)
            {
                //found the right condition, first shift the array to fill in the empty spot
                for ( i++; i < MAX_CONDITION_ARRAY_SIZE; i++)
                {
                    m_IfConditions[i-1] = m_IfConditions[i];
                }
                m_IfConditions[MAX_CONDITION_ARRAY_SIZE-1] = NULL;
    
                //now clear out the desired index
                for ( i = MAX_CONDITION_ARRAY_SIZE-1; i > Index; i--)
                {
                    m_IfConditions[i] = m_IfConditions[i-1];
                }
                m_IfConditions[Index] = pCondition;

                return;
            }
        }
    }
    else
    {
        //if the index is too high, set it to the highest used index
        if (Index >= m_NumElseConditions)
            Index = m_NumElseConditions - 1;

        for( s32 i = 0; i< MAX_CONDITION_ARRAY_SIZE ; i++ )
        { 
            if (m_ElseConditions[i] == pCondition)
            {
                //found the right condition, first shift the array to fill in the empty spot
                for ( i++; i < MAX_CONDITION_ARRAY_SIZE; i++)
                {
                    m_ElseConditions[i-1] = m_ElseConditions[i];
                }
                m_ElseConditions[MAX_CONDITION_ARRAY_SIZE-1] = NULL;
    
                //now clear out the desired index
                for ( i = MAX_CONDITION_ARRAY_SIZE-1; i > Index; i--)
                {
                    m_ElseConditions[i] = m_ElseConditions[i-1];
                }
                m_ElseConditions[Index] = pCondition;

                return;
            }
        }
    }
    
    ASSERT(FALSE);
}

//=============================================================================

void  conditional_affecter::RemoveCondition ( conditional_ex_base* pCondition, xbool bAndDelete )
{
    s32 i=0;
    
    if (!pCondition)
        return;

    if (!pCondition->GetElse())
    {
        for( i = 0; i< MAX_CONDITION_ARRAY_SIZE ; i++ )
        { 
            if (m_IfConditions[i] == pCondition)
            {
                if (bAndDelete) 
                    delete m_IfConditions[i];

                m_IfConditions[i] = NULL;
                break;
            }
        }
    
        if ( i == MAX_CONDITION_ARRAY_SIZE )
        {
            x_DebugMsg("conditional_affecter::RemoveCondition, Cannot find condition in table.");
            ASSERT(0);
            return;
        }

        //Shift the array to remove the empty slot...
        for ( i++; i < MAX_CONDITION_ARRAY_SIZE; i++)
        {
            m_IfConditions[i-1] = m_IfConditions[i];
        }

        m_IfConditions[MAX_CONDITION_ARRAY_SIZE-1] = NULL;
        m_NumIfConditions--;
    }
    else
    {
        for( i = 0; i< MAX_CONDITION_ARRAY_SIZE ; i++ )
        { 
            if (m_ElseConditions[i] == pCondition)
            {
                if (bAndDelete) 
                    delete m_ElseConditions[i];

                m_ElseConditions[i] = NULL;
                break;
            }
        }
    
        if ( i == MAX_CONDITION_ARRAY_SIZE )
        {
            x_DebugMsg("conditional_affecter::RemoveCondition, Cannot find condition in table.");
            ASSERT(0);
            return;
        }

        //Shift the array to remove the empty slot...
        for ( i++; i < MAX_CONDITION_ARRAY_SIZE; i++)
        {
            m_ElseConditions[i-1] = m_ElseConditions[i];
        }

        m_ElseConditions[MAX_CONDITION_ARRAY_SIZE-1] = NULL;
        m_NumElseConditions--;
    }
}

//=============================================================================

xbool conditional_affecter::IsFirstInConditionSet( conditional_ex_base* pCondition )
{
    if (pCondition)
    {
        if (!pCondition->GetElse())
        {
            if (m_IfConditions[0] == pCondition)
            {
                return TRUE;
            }
        }
        else
        {
            if (m_ElseConditions[0] == pCondition)
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

xbool conditional_affecter::CanSwitchElse( conditional_ex_base* pCondition )
{
    ASSERT(pCondition);
    if ( pCondition )
    {
        if ( !pCondition->GetElse() )
        {
            if ( GetElseConditionCount() < MAX_CONDITION_ARRAY_SIZE )
            {
                return TRUE;
            }
        }
        else
        {
            if ( GetConditionCount() < MAX_CONDITION_ARRAY_SIZE )
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

//=============================================================================

void conditional_affecter::SwitchElse( conditional_ex_base* pCondition )
{
    ASSERT(pCondition);
    if ( pCondition && CanSwitchElse(pCondition) )
    {
        RemoveCondition(pCondition, FALSE);
        pCondition->SetElse(!pCondition->GetElse());

        if ( !pCondition->GetElse() )
        {
            ASSERT (!m_IfConditions[m_NumIfConditions]);
            m_IfConditions[m_NumIfConditions] = pCondition;
            m_NumIfConditions++;
        }
        else
        {
            ASSERT (!m_ElseConditions[m_NumElseConditions]);
            m_ElseConditions[m_NumElseConditions] = pCondition;
            m_NumElseConditions++;
        }
    }
}

//=============================================================================

void conditional_affecter::RemoveAllConditions( void )
{
    for( s32 i = 0; i< MAX_CONDITION_ARRAY_SIZE ; i++ )
    { 
        if (m_IfConditions[i])
        {
            delete m_IfConditions[i];
            m_IfConditions[i] = NULL;
        }

        if (m_ElseConditions[i])
        {
            delete m_ElseConditions[i];
            m_ElseConditions[i] = NULL;
        }
    }

    m_NumElseConditions = 0;
    m_NumIfConditions   = 0;
}

//=============================================================================

#ifndef X_RETAIL
void conditional_affecter::OnDebugRender( void )
{
    for( s32 i=0; i< MAX_CONDITION_ARRAY_SIZE ; i++ )
    { 
        if (m_IfConditions[i] != NULL)
        {
            m_IfConditions[i]->OnDebugRender(i);
        }

        if (m_ElseConditions[i] != NULL)
        {
            m_ElseConditions[i]->OnDebugRender(i);
        }
    }
}
#endif // X_RETAIL

//=============================================================================

#ifdef X_EDITOR

s32 conditional_affecter::OnValidateConditions( trigger_ex_object&   Trigger, 
                                                xstring&             ConditionType, 
                                                s32                  nConditions, 
                                                conditional_ex_base* Conditions[], 
                                                xstring&             ErrorMsg )
{
    s32 i;
    s32 nErrors = 0;

    // Check all conditions
    for( i = 0 ; i < nConditions ; i++ )
    {
        // Lookup action
        conditional_ex_base* pCondition = Conditions[i];
        ASSERT( pCondition );

        // Clear errors
        xstring ConditionErrorMsg;
        s32     nConditionErrors = 0;

        // ObjectRef0
        xstring          ObjectRef0;
        object_affecter* pObjectRef0 = pCondition->GetObjectRef0( ObjectRef0 );
        nConditionErrors += Trigger.OnValidateObject( ObjectRef0, pObjectRef0, ConditionErrorMsg );

        // ObjectRef1
        xstring          ObjectRef1;
        object_affecter* pObjectRef1 = pCondition->GetObjectRef0( ObjectRef1 );
        nConditionErrors += Trigger.OnValidateObject( ObjectRef1, pObjectRef1, ConditionErrorMsg );

        // AnimRef
        xstring          AnimRef;
        s32              AnimName = -1;
        s32*             pAnimGroupName = pCondition->GetAnimRef( AnimRef, AnimName );
        nConditionErrors += Trigger.OnValidateAnim( AnimRef, pAnimGroupName, AnimName,  ConditionErrorMsg );

        // SoundRef
        xstring          SoundRef;
        s32              SoundName = -1;
        rhandle<char>*   pSoundPackage = pCondition->GetSoundRef( SoundRef, SoundName );
        nConditionErrors += Trigger.OnValidateSound ( SoundRef, pSoundPackage, SoundName, ConditionErrorMsg );

        // GlobalRef
        xstring          GlobalRef;
        s32*             pGlobalName = pCondition->GetGlobalRef( GlobalRef );
        nConditionErrors += Trigger.OnValidateGlobal  ( GlobalRef, pGlobalName, ConditionErrorMsg );

        // PropertyRef
        xstring          PropertyRef;
        s32              PropertyType = -1;
        s32*             pPropertyName = pCondition->GetPropertyRef( PropertyRef, PropertyType );
        nConditionErrors += Trigger.OnValidateProperty( PropertyRef, pObjectRef0, pPropertyName, PropertyType, ConditionErrorMsg );

        // TemplateRef
        xstring          TemplateRef;
        s32*             pTemplateName = pCondition->GetTemplateRef( TemplateRef );
        nConditionErrors += Trigger.OnValidateTemplate( TemplateRef, pTemplateName, ConditionErrorMsg );

        // Guid0Ref
        xstring          Guid0Ref;
        guid*            pGuid0 = pCondition->GetGuidRef0( Guid0Ref );
        nConditionErrors += Trigger.OnValidateObject( Guid0Ref, pGuid0, ConditionErrorMsg );

        // Guid1Ref
        xstring          Guid1Ref;
        guid*            pGuid1 = pCondition->GetGuidRef1( Guid1Ref );
        nConditionErrors += Trigger.OnValidateObject( Guid1Ref, pGuid1, ConditionErrorMsg );

        // Any errors?
        if( nConditionErrors )
        {
            // Setup condition info
            xstring ConditionNumber;
            ConditionNumber.Format( "%d", i );

            // Add condition error to list
            ErrorMsg += ConditionType + "[" + ConditionNumber + "] " + pCondition->GetDescription() + "\n";
            ErrorMsg += ConditionErrorMsg + "\n";
            nErrors  += nConditionErrors;
        }
    }

    return nErrors;
}

//=============================================================================

s32 conditional_affecter::OnValidateProperties( trigger_ex_object& Trigger, xstring& ErrorMsg )
{
    s32 nErrors = 0;

    // Validate conditions
    nErrors += OnValidateConditions( Trigger, xstring( "IfCondition" ),   m_NumIfConditions,   m_IfConditions,   ErrorMsg );
    nErrors += OnValidateConditions( Trigger, xstring( "ElseCondition" ), m_NumElseConditions, m_ElseConditions, ErrorMsg );

    return nErrors;
}

#endif
