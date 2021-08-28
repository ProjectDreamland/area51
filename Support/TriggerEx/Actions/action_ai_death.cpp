///////////////////////////////////////////////////////////////////////////
//
//  action_ai_death.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_ai_death.hpp"
#include "Loco\LocoUtil.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_ai_death::action_ai_death ( guid ParentGuid ) : action_ai_base( ParentGuid ),
m_DeathType( action_ai_death::DEATH_TYPE_RAGDOLL ),
m_AnimName(-1),
m_AnimGroupName(-1),
m_RagdollForceAmount(5000.0f),
m_RagdollForceRadius(500.0f)
{
}

//=============================================================================

void action_ai_death::OnEnumProp	( prop_enum& rPropList )
{
    action_ai_base::OnEnumProp( rPropList );

    // Death type
    rPropList.PropEnumEnum( "DeathType", 
                            "RAGDOLL\0ANIM\0RAGDOLL_INACTIVE\0", 
                            "What kind of death?\nRAGDOLL = Straight to ragdoll using specified force\nANIM = Play specified anim, switching to ragdoll on last frame (will inherit anim velocities)\nRAGDOLL_INACTIVE = Go straight to frozen ragdoll", 
                            PROP_TYPE_MUST_ENUM );
    
    // Show different properties depending upon death type currently selected
    switch( m_DeathType )
    {
        case action_ai_death::DEATH_TYPE_ANIM:
        {
            // Animation properties (no flags needed so using local)
            u32 AnimFlags = 0;
            LocoUtil_OnEnumPropAnimFlags( rPropList,
                                          0,
                                          AnimFlags );
        }
        break;
    
        case action_ai_death::DEATH_TYPE_RAGDOLL:
        {                                  
            // Ragdoll properties
            m_RagdollForceLocation.OnEnumProp( rPropList,   "RagdollForceLocation", 0 );
            rPropList.PropEnumFloat( "RagdollForceAmount",  "Force amount", 0 );  
            rPropList.PropEnumFloat( "RagdollForceRadius",  "Force radius", 0 );  
        }
        break;
    }                    
}

//=============================================================================

xbool action_ai_death::OnProperty	( prop_query& rPropQuery )
{
    // Check base class
    if( action_ai_base::OnProperty( rPropQuery ) )
        return TRUE;

    // Death type?
    if( rPropQuery.IsVar( "DeathType" ) )
    {
        if( rPropQuery.IsRead() )
        {
            switch( m_DeathType )
            {
            default:
            case DEATH_TYPE_RAGDOLL:            rPropQuery.SetVarEnum( "RAGDOLL" );          break;
            case DEATH_TYPE_ANIM:               rPropQuery.SetVarEnum( "ANIM" );             break;
            case DEATH_TYPE_RAGDOLL_INACTIVE:   rPropQuery.SetVarEnum( "RAGDOLL_INACTIVE" ); break;
            }            
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            if( x_stricmp( pString, "RAGDOLL" ) == 0 )
                m_DeathType = DEATH_TYPE_RAGDOLL;
            else if( x_stricmp( pString, "ANIM" ) == 0 )
                m_DeathType = DEATH_TYPE_ANIM;
            else if( x_stricmp( pString, "RAGDOLL_INACTIVE" ) == 0 )
                m_DeathType = DEATH_TYPE_RAGDOLL_INACTIVE;
        }
        return TRUE;            
    }

    // Animation properties? (no flags needed so using local)
    u32 AnimFlags = 0;
    if( LocoUtil_OnPropertyAnimFlags( rPropQuery, 
                                      m_AnimGroupName, 
                                      m_AnimName, 
                                      AnimFlags ) )
    {
        return TRUE ;
    }
    
    // Ragdoll properties?
    if( m_RagdollForceLocation.OnProperty( rPropQuery, "RagdollForceLocation" ) )
    {
        return TRUE;            
    }
    if( rPropQuery.VarFloat( "RagdollForceAmount", m_RagdollForceAmount, 0.0f, 100000.0f ) )
    {
        return TRUE;    
    }
    if( rPropQuery.VarFloat( "RagdollForceRadius", m_RagdollForceRadius, 0.0f, 100000.0f ) )
    {
        return TRUE;    
    }

    return FALSE;
}

//=============================================================================

const char* action_ai_death::GetDescription( void )
{
    static big_string   Info;
    static med_string   AnimText;

    if (m_AnimName == -1 )
    {
        if( m_DeathType == DEATH_TYPE_RAGDOLL )
            AnimText.Set( xfs( "by ragdoll with force amount %.2f and radius %.2f", m_RagdollForceAmount, m_RagdollForceRadius ) ); 
        else            
            AnimText.Set( "by ragdoll inactive" );
    }
    else
    {
        AnimText.Set( xfs( "by anim %s", g_StringMgr.GetString( m_AnimName ) ) );
    }

    Info.Set(xfs("%s Death %s", GetAIName(), AnimText.Get()));          
    return Info.Get();
}

//=============================================================================

#ifdef X_EDITOR

object_affecter* action_ai_death::GetObjectRef1( xstring& Desc )
{
    // Death by ragdoll with a marker specified as location?
    if( ( m_DeathType == DEATH_TYPE_RAGDOLL ) && ( m_RagdollForceLocation.GetGuid() != 0 ) )
    {
        Desc = "Ragdoll force location marker error: "; 
        return &m_RagdollForceLocation;
    }
    else
    {
        return NULL;    
    }
}

//=============================================================================

s32* action_ai_death::GetAnimRef( xstring& Desc, s32& AnimName )
{
    // Death by animation?
    if( m_DeathType == DEATH_TYPE_ANIM )
    {
        Desc = "Animation error: "; 
        AnimName = m_AnimName; 
        return &m_AnimGroupName;
    }
    else
    {
        return NULL;
    }        
}

#endif

//=============================================================================
