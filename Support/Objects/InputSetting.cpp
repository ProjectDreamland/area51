//==============================================================================
//
//  InputSetting.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "InputSetting.hpp"
#include "InputMgr\GamePad.hpp"

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================
static struct input_setting_desc : public object_desc
{
    input_setting_desc( void ) : object_desc( 
        object::TYPE_INPUT_SETTINGS, 
        "Input Setting", 
        "SYSTEM",

        object::ATTR_NULL,

        FLAGS_GENERIC_EDITOR_CREATE ) {}         

    //---------------------------------------------------------------------

    virtual object* Create          ( void )
    {
        return new input_setting;
    }

    //---------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32 OnEditorRender( object& Object ) const
    { 
        object_desc::OnEditorRender( Object );
        return EDITOR_ICON_INPUT_SETTINGS; 
    }

#endif // X_EDITOR

} s_InputSetting_Desc;

//=========================================================================

const object_desc& input_setting::GetTypeDesc( void ) const
{
    return s_InputSetting_Desc;
}

//=========================================================================

const object_desc& input_setting::GetObjectType( void )
{
    return s_InputSetting_Desc;
}


//==============================================================================
// InputSetting
//==============================================================================

input_setting::input_setting ( void )
{
}

//==============================================================================

input_setting::~input_setting ( void )
{
}

//==============================================================================
                                                    
void input_setting::OnInit( void )
{

}

//==============================================================================

void input_setting::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp( List );
   
    s32 i = 0;
    //for( i = 0; i < 2; i++ )
    //{
        List.PropEnumHeader( xfs( "Player %d", (i+1) ), "Players controls", 0 );
        s32 iPath = List.PushPath( xfs( "Player %d\\", (i+1) ) );

        g_IngamePad[i].OnEnumProp( List );

        List.PopPath( iPath );
    //}
}

//==============================================================================

xbool input_setting::OnProperty( prop_query& I )
{
    if( object::OnProperty( I ) )
        return TRUE;

    s32 iPath = I.PushPath( xfs( "Player %d\\", 1 ) );

    if( g_IngamePad[0].OnProperty( I ) )
    {
        g_IngamePad[1].SetAllLogical( g_IngamePad[0].GetLogical() );
        
        for( s32 i = 0; i < MAX_INPUT_PLATFORMS; i++ )
        {
            g_IngamePad[1].SetAllMap( i, g_IngamePad[0].GetMap( i ), g_IngamePad[0].GetNMaps( i ) );
            #ifdef TARGET_XBOX
            g_IngamePad[2].SetAllMap( i, g_IngamePad[0].GetMap( i ), g_IngamePad[0].GetNMaps( i ) );
            g_IngamePad[3].SetAllMap( i, g_IngamePad[0].GetMap( i ), g_IngamePad[0].GetNMaps( i ) );            
            #endif
        }
        return TRUE;
    }
    
    I.PopPath( iPath );
    
    return FALSE;
}
