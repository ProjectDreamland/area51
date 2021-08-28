//==============================================================================
//
//  BluePrintBag.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "BluePrintBag.hpp"
#include "Dictionary\global_dictionary.hpp"

//==============================================================================
//  OBJECT DESCRIPTION
//==============================================================================

static struct blueprint_bag_desc : public object_desc
{
    blueprint_bag_desc( void ) 
        :   object_desc( object::TYPE_BLUEPRINT_BAG, 
                         "Blueprint Bag",
                         "Multiplayer",
                         object::ATTR_NULL + object::ATTR_NO_RUNTIME_SAVE,
                         FLAGS_GENERIC_EDITOR_CREATE | FLAGS_IS_DYNAMIC )
    {
        // Empty function body.
    }

    //--------------------------------------------------------------------------

    virtual object* Create( void ) 
    { 
        return( new blueprint_bag ); 
    }

    //--------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32 OnEditorRender( object& Object ) const
    {
        (void)Object;
        return( EDITOR_ICON_BLUEPRINT_BAG );
    }

#endif // X_EDITOR

} s_blueprint_bag_Desc;

//==============================================================================
//  FUNCTIONS
//==============================================================================

const object_desc& blueprint_bag::GetTypeDesc( void ) const
{
    return( s_blueprint_bag_Desc );
}

//==============================================================================

const object_desc& blueprint_bag::GetObjectType( void )
{
    return( s_blueprint_bag_Desc );
}

//==============================================================================

blueprint_bag::blueprint_bag( void )
{
    for( s32 i = 0; i < 32; i++ )
        m_TemplateIndex[i] = -1;
}

//==============================================================================

blueprint_bag::~blueprint_bag( void )
{
}

//==============================================================================

bbox blueprint_bag::GetLocalBBox( void ) const 
{ 
    return( bbox( vector3(0,0,0), 50.0f ) );
}

//==============================================================================

void blueprint_bag::OnEnumProp( prop_enum& rPropList )
{
    object::OnEnumProp( rPropList );
    rPropList.PropEnumHeader( "BlueprintBag", 
            "List of blueprints to be exported with this map.", 0 );

    // Pack all of the values down into the lower end of the array.
    for( s32 i = 0; i < 32; i++ )
    {
        if( m_TemplateIndex[i] < 0 )
        {
            for( s32 j = i+1; j < 32; j++ )
            {
                if( m_TemplateIndex[j] >= 0 )
                {
                    m_TemplateIndex[j-1] = m_TemplateIndex[j];
                    m_TemplateIndex[ j ] = -1;
                }
            }
        }
    }

    for( s32 i = 0; i < 32; i++ )
    {
        char Name[64];
        x_sprintf( Name, "BlueprintBag\\Blueprint%02d", i );

        rPropList.PropEnumFileName( Name, 
                            "template blueprints (*.bpx)|*.bpx|All Files (*.*)|*.*||", 
                            "Name of a blueprint to be compiled with this map.", PROP_TYPE_MUST_ENUM );

        if( m_TemplateIndex[i] < 0 )
            break;
    }
}
    
//==============================================================================

xbool blueprint_bag::OnProperty( prop_query& rPropQuery )
{
    if( object::OnProperty( rPropQuery ) )
    {
        return( TRUE );
    }

    for( s32 i = 0; i < 32; i++ )
    {
        char Name[64];
        x_sprintf( Name, "BlueprintBag\\Blueprint%02d", i );

        if( rPropQuery.IsVar( Name ) )
        {
            if( rPropQuery.IsRead() )
            {
                if ( m_TemplateIndex[i] < 0 )
                {
                    rPropQuery.SetVarFileName( "", 256 );
                    return( TRUE );
                }
                else
                {
                    rPropQuery.SetVarFileName( g_TemplateStringMgr.GetString( m_TemplateIndex[i] ), 256 );
                    return( TRUE );
                }
            }
            else
            {
                if( x_strlen( rPropQuery.GetVarFileName() ) > 0 )
                {
                    m_TemplateIndex[i] = g_TemplateStringMgr.Add( rPropQuery.GetVarFileName() );
                    return( TRUE );
                }
                else
                {
                    m_TemplateIndex[i] = -1;
                    return( TRUE );
                }
            }
        }
    }

    return( FALSE );
}

//==============================================================================
