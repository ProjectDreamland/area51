///////////////////////////////////////////////////////////////////////////////
//
//  action_ai_pathto_guid.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_ai_pathto_guid_
#define _action_ai_pathto_guid_

//=========================================================================
// INCLUDES
//=========================================================================

#include "action_ai_base.hpp"
#include "..\support\loco\loco.hpp"

//=========================== ==============================================
// action_ai_pathto_guid
//=========================================================================

class action_ai_pathto_guid : public action_ai_base
{
public:
                    action_ai_pathto_guid               ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_AI_PATHTO_GUID;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Path to a particular guid."; } 
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    

    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         object_affecter*    GetObjectRef1   ( xstring& Desc ) { Desc = "Location object error: "; return &m_LocationAffecter; }
#endif

    virtual         ai_action_types     GetAIActionType ( void ) { return AI_PATHFIND_TO_GUID; }

                    guid                GetPathToGuid   ( void );
    inline          f32                 GetDistance     ( void ) { return m_Distance; }
    inline          radian              GetYawThreshold ( void ) { return m_YawThreshold; }
    inline          loco::move_style    GetMoveStyle    ( void ) { return m_MoveToStyle; }
    inline          s32                 GetNextState    ( void ) { return m_NextAiState; }
    inline          xbool               GetAlignExact   ( void ) { return m_bAlignExact; }
    inline          xbool               GetGotoExact    ( void ) { return m_bGotoExact; }
    inline          xbool               GetRetreating   ( void ) { return m_Retreating; }

protected:

    object_affecter     m_LocationAffecter;
    f32                 m_Distance;
    radian              m_YawThreshold;  
    loco::move_style    m_MoveToStyle;
    s32                 m_NextAiState;             // AI State to change too at end..    
    xbool               m_Retreating;
    xbool               m_bAlignExact;
    xbool               m_bGotoExact;
};


#endif
