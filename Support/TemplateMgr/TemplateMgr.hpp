#ifndef template_mgr_HPP
#define template_mgr_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "Entropy.hpp"
#include "Auxiliary\MiscUtils\Dictionary.hpp"
#include "x_BitStream.hpp"
#include "Objects\object.hpp"

//=========================================================================
// PRE-DECL
//=========================================================================

class text_in;
class prop_container;

//=========================================================================
// TYPES
//=========================================================================

class template_mgr
{
public:
                template_mgr                ( void );
               ~template_mgr                ( void );

    //---------------------------------------------------------------------
    // LoadStream           - Upon level load in the PS2 we need to load the template stream
    // CreateTemplate       - Create a named template from the loaded data stream
    // CreateSingleTemplate - Used to create template objects from blueprints with exactly one object in them (not includint the anchor)
    //---------------------------------------------------------------------
    void        ClearData                   ( void );
    xbool       LoadData                    ( const char* pFile, const char* pDictionary );
    xbool       CreateTemplate              ( const char* pName, const vector3& Pos, const radian3& Rot , u16 Zone1, u16 Zone2, guid ObjectGuid = NULL_GUID );
    guid        CreateSingleTemplate        ( const char* pName, const vector3& Pos, const radian3& Rot , u16 Zone1, u16 Zone2 );
    xbool       IsSingleTemplate            ( const char* pName );
    void        DisplayTemplates            ( void );
    xbool       IsTemplateAvailable         ( const char* pName );
    void        ApplyTemplateToObject       ( const char* pName, guid GUID );

protected:

    // structure to represent a single property
    struct prop_entry 
    {
        u16                 PropType;        // Prop_enum defined in Property.hpp
        s16                 NameIndex;       // Dictionary Index
    };

    // structure to represent an object in a template containing multiple properties
    struct obj_entry 
    {
        s16                 TypeIndex;       // Dictionary Index
        s32                 iProperty;       // Index to the first property
        s16                 nProperty;       // Number of properties
    };

    // structure to represent a blueprint template containg multiple objects
    struct template_entry 
    {
        vector3             AnchorPos;       // Position for the blueprint anchor

        s32                 iStartBitStream; // Byte/bit offset to the first property
        s16                 NameIndex;       // Dictionary Index
        s16                 iObject;         // Index to the first object
        s16                 nObjects;        // Number of object for the template
    };

    //helper functions
    void        NullData                    ( void ); 
    void        AddPropertyToObject         ( prop_entry& pe, object* pObject ); 
    
protected:

    dictionary*             m_pDictionary;
    bitstream               m_BitStream;      // Actual data for the properties\
    
    s32                     m_nTemplates;
    template_entry*         m_pTemplate;

    s32                     m_nObjects;
    obj_entry*              m_pObject;

    s32                     m_nProperties;
    prop_entry*             m_pProperties;
    

// the following functions are for editor use only
#ifdef X_EDITOR
public:
    //create a pre-game template (ONLY CALL FROM OBJECT EditorPreGame)
    xbool       EditorCreateTemplateFromPath( const char* pName, const vector3& Pos, const radian3& Rot , u16 Zone1, u16 Zone2, guid ObjectGuid = NULL );    
    guid        EditorCreateSingleTemplateFromPath( const char* pName, const vector3& Pos, const radian3& Rot , u16 Zone1, u16 Zone2 );    

    //create a list of used blueprints
    xbool       EditorBuildTemplateArray    ( void );

    //Create the necessary game data to run the game in the editor (mimic LoadStream for PS2)
    xbool       EditorCreateGameData        ( xbool bPreGameDetermination = FALSE );
    //Used for exporting the streamed data when the level is exported
    xbool       EditorSaveData              ( const char* pFile, const char* pDictionary );

protected:
    void        AddDataToBitStream          ( prop_container& pc );

    struct BlueprintReference 
    {
        char cFullPath[MAX_PATH];
    };

    //Editor list of data referencing the blueprints(templates) to be used in the game
    xarray<BlueprintReference>  m_lstBlueprints;
    char                        m_PreGamePathing[256];

#endif // X_EDITOR
};

//=========================================================================

extern template_mgr g_TemplateMgr;

//=========================================================================
// END
//=========================================================================
#endif