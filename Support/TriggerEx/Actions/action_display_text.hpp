///////////////////////////////////////////////////////////////////////////////
//
//  Action_Display_Text.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ACTION_DISPLAY_TEXT
#define ACTION_DISPLAY_TEXT

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"


//=========================================================================
// MUSIC_INTENSITY
//=========================================================================

class action_display_text : public actions_ex_base
{
public:
    

                    action_display_text                     ( guid ParentGuid );

    virtual         action_ex_types         GetType         ( void )   { return GetTypeStatic(); }
    static          action_ex_types         GetTypeStatic   ( void )   { return TYPE_ACTION_DISPLAY_TEXT;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Display the text from the table."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( f32 DeltaTime );
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );
   
protected:
    
    s32     m_TableName;
    s32     m_TitleName;
    xbool   m_Objective;
    xbool   m_RenderNow;

};

#endif
