//=========================================================================
// INCLUDES
//=========================================================================

#ifdef TARGET_PC
#include <crtdbg.h>
#endif

#include "RawSettings.hpp"
#include "Parsing\Tokenizer.hpp"


//=========================================================================
// FUNCTIONS
//=========================================================================

// Sorts properties by section
class ComparePropertiesForSort : public x_compare_functor< const raw_settings::property& >
{
public:
    s32 operator()( const raw_settings::property& Prop1, const raw_settings::property& Prop2 )
    {
        if( Prop1.iSection < Prop2.iSection )
            return -1;
        else if( Prop1.iSection > Prop2.iSection )
            return 1;
        else 
            return 0;
    }
};

//=========================================================================

raw_settings::raw_settings( void )
{
    m_pSettingsFile = NULL;
}

//=========================================================================

raw_settings::~raw_settings( void )
{
}

//=========================================================================

void raw_settings::AddBoneMask( const xstring&  GroupName, 
                                const xstring&  BoneName, 
                                      f32       Weight )
{
    // Search for group
    s32 iGroup;
    for( iGroup = 0; iGroup < m_BoneMasks.GetCount(); iGroup++ )
    {
        if( m_BoneMasks[iGroup].Name == GroupName )
            break;
    }

    // Create a new group?
    if( iGroup == m_BoneMasks.GetCount() )
    {
        bone_masks& BoneMasks = m_BoneMasks.Append();
        BoneMasks.Name = GroupName;
    }        

    // Add to group
    bone_masks&       BoneMasks = m_BoneMasks[ iGroup ];
    bone_masks::mask& Mask      = BoneMasks.Masks.Append();
    Mask.BoneName = BoneName;
    Mask.Weight   = Weight;
}

//=========================================================================

void raw_settings::AddProperty( const xstring&  SectionName,
                                const xstring&  PropertyName,
                                const xstring&  PropertyType,
                                const xstring&  PropertyValue )
{
    // Search for section
    s32 iSection;
    for( iSection = 0; iSection < m_PropertySections.GetCount(); iSection++ )
    {
        if( m_PropertySections[iSection].Name == SectionName )
            break;
    }

    // Create a new section?
    if( iSection == m_PropertySections.GetCount() )
    {
        property_section& Section = m_PropertySections.Append();
        Section.Name = SectionName;
    }        

    // Update section
    property_section& Section = m_PropertySections[ iSection ];
    Section.nProperties++;

    // Make sure property is not already present
    for( s32 iProp = 0; iProp < m_Properties.GetCount(); iProp++ )
    {
        // Lookup property
        const property& CheckProp = m_Properties[iProp];
        
        // Match?
        if(     ( CheckProp.iSection == iSection) 
            &&  ( CheckProp.Name     == PropertyName )
            &&  ( CheckProp.Type     == PropertyType ) )
        {
            x_throw( xfs( "Duplicate property: %s %s %s found!\nReferenced in settings file [%s]\n\n", 
                          SectionName, PropertyName, PropertyType, m_pSettingsFile ) );
        }                    
    }
    
    // Create new property
    property& Property = m_Properties.Append();
    Property.iSection = iSection;
    Property.Name     = PropertyName;
    Property.Type     = PropertyType;
    Property.Value    = PropertyValue;
}

//=========================================================================

xbool raw_settings::Load( const char* pSettingsFile )
{
    // Try open file
    m_pSettingsFile = pSettingsFile;
    token_stream TOK;
    if( !TOK.OpenFile( pSettingsFile ) )
    {
        x_throw( xfs( "Settings file [%s] not found!", pSettingsFile ) );
    }

    // Loop through all tokens
    while( TOK.IsEOF() == FALSE )
    {
        // Read next token
        TOK.Read();

        // Found bone masks?
        if( x_stricmp( TOK.String(), "BeginBoneMasks" ) == 0 )
        {
            // Fill in bone masks
            xstring GroupName;
            while( TOK.IsEOF() == FALSE )
            {
                // Read next token
                TOK.Read();

                // Found name?
                if( x_stricmp( TOK.String(), "GroupName" ) == 0 )
                {
                    GroupName = TOK.ReadString();
                }
                else if( x_stricmp( TOK.String(), "BoneMask" ) == 0 )
                {
                    // Read info
                    xstring BoneName = TOK.ReadString();
                    f32     Weight   = TOK.ReadFloat();

                    // Group not setup?
                    if( GroupName.GetLength() == 0 )
                    {
                        x_throw( xfs( "Bone group name not specified.\nReferenced in settings file [%s] line %d\n\n", (const char*)BoneName, pSettingsFile, TOK.GetLineNumber() ) );
                    }
                    else
                    {
                        // Add
                        AddBoneMask( GroupName, BoneName, Weight );
                    }                                                
                }
                else if( x_stricmp( TOK.String(), "EndBoneMasks" ) == 0 )
                {
                    // Exit while loop
                    break;
                }
            }
        }
        // Found properties?
        else if( x_stricmp( TOK.String(), "BeginProperties" ) == 0 )
        {
            // Fill in bone masks
            while( TOK.IsEOF() == FALSE )
            {
                // Read next token
                TOK.Read();

                // Found property?
                if( x_stricmp( TOK.String(), "Property" ) == 0 )
                {
                    // Read info
                    xstring Section = TOK.ReadString();
                    xstring Name    = TOK.ReadString();

                    TOK.Read();
                    xstring Type = TOK.String();

                    TOK.Read();
                    xstring Value = TOK.String();

                    // Add property
                    Type.MakeUpper();
                    AddProperty( Section, Name, Type, Value );
                }
                else if( x_stricmp( TOK.String(), "EndProperties" ) == 0 )
                {
                    // Exit while loop
                    break;
                }
            }
        }
    }

    // Close file        
    TOK.CloseFile();

    // Found properties?
    if( m_Properties.GetCount() )
    {
        // Sort properties by section            
        x_qsort( &m_Properties[0], m_Properties.GetCount(), ComparePropertiesForSort() );
        
        // Set section property start indices
        s32 iProperty = 0;
        for( s32 iSection = 0; iSection < m_PropertySections.GetCount(); iSection++ )
        {
            // Set section start property
            property_section& Section = m_PropertySections[ iSection ];
            Section.iProperty = iProperty;
            
            // Validate
            for( s32 iProp = 0; iProp < Section.nProperties; iProp++ )
            {
                // Section properties should all be part of this section
                ASSERT( m_Properties[ Section.iProperty + iProp ].iSection == iSection );
            }
            
            // Next set of properties
            iProperty += Section.nProperties;
        }
    }
    
    // Success
    return TRUE;
}

//=========================================================================

