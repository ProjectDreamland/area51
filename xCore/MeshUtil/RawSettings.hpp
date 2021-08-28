
#ifndef __RAW_SETTINGS_HPP__
#define __RAW_SETTINGS_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_files.hpp"
#include "Animation\AnimData.hpp"



//=========================================================================
// CLASS
//=========================================================================
struct raw_settings
{
    //=========================================================================
    
    struct bone_masks
    {
        // Structures
        struct mask
        {
            xstring BoneName;
            f32     Weight;
        };

        // Data
        xstring         Name;
        xarray<mask>    Masks;
        
        // Functions
        bone_masks()
        {
            Masks.SetCapacity( MAX_ANIM_BONES );
        }
    };

    //=========================================================================

    struct property
    {
        // Data
        s32         iSection;
        xstring     Name;
        xstring     Type;
        xstring     Value;    
    };

    //=========================================================================

    struct property_section
    {
        // Data
        xstring     Name;
        s32         iProperty;
        s32         nProperties;

        // Functions
        property_section()
        {
            iProperty   = 0;
            nProperties = 0;
        }
    };

    //=========================================================================

    // Data
    const char*                 m_pSettingsFile;
    xarray<bone_masks>          m_BoneMasks;
    xarray<property>            m_Properties;
    xarray<property_section>    m_PropertySections;

    //=========================================================================
    
    // Functions    
            raw_settings    ( void );
            ~raw_settings   ( void );

    void AddBoneMask    ( const xstring&  GroupName, 
                          const xstring&  BoneName, 
                                f32       Weight );

    void AddProperty    ( const xstring&  SectionName,
                          const xstring&  PropertyName,
                          const xstring&  PropertyType,
                          const xstring&  PropertyValue );

    xbool   Load        ( const char*     pSettingsFile );
};

//=========================================================================
#endif

