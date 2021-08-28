#ifndef BIN_LEVEL_HPP
#define BIN_LEVEL_HPP

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

class bin_level
{
public:
                bin_level                ( void );
               ~bin_level                ( void );

    //---------------------------------------------------------------------
    // LoadStream     - Upon level load in the PS2 we need to load the template stream
    //---------------------------------------------------------------------
    void        ClearData                   ( xbool bClearDictionary );
    xbool       LoadData                    ( const char* pFile, const char* pDictionary );
    xbool       LoadLevel                   ( const char* pLevelName, const char* pDictionaryName, const char* pLoadOrderName );
    void        SetRigidColor               ( const char* pFileName );
    void        PreloadDataFiles            ( const char* pLoadOrderName );

    void        SetDesireToSave             ( xbool bDesire ) { m_bWantsToSave = bDesire; }
    xbool       WantsToSave                 ( void ) { return m_bWantsToSave; }
    void        SetDesireToLoad             ( xbool bDesire ) { m_bWantsToLoad = bDesire; }
    xbool       WantsToLoad                 ( void ) { return m_bWantsToLoad; }
    xbool       SaveRuntimeDynamicData      ( void );
    xbool       LoadRuntimeDynamicData      ( void );

    dictionary*             m_pDictionary;

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
        s16                 nProperty;       // Number of properties
        s32                 iProperty;       // Index to the first property
        u64                 Guid;            // Unique guid for the object
    };


    //helper functions
    void        NullData                    ( xbool bClearDictionary ); 
    void        AddPropertyToObject         ( prop_entry& pe, object* pObject ); 
    void        AddDataToBitStream          ( prop_container& pc );

protected:

    bitstream               m_BitStream;      // Actual data for the properties\
    
    s32                     m_nObjects;
    obj_entry*              m_pObject;

    s32                     m_nProperties;
    prop_entry*             m_pProperties;

    xbool                   m_bIsRuntimeDynamicData;
    xbool                   m_bWantsToSave;
    xbool                   m_bWantsToLoad;
    

// the following functions are for editor use only
#ifdef X_EDITOR
public:

    //Create the necessary game data to run the game in the editor (mimic LoadStream for PS2)
    xbool       EditorCreateGameData        ( xarray<guid> lstGuidsToExport  );
    //Used for exporting the streamed data when the level is exported
    xbool       EditorSaveData              ( const char* pFile, const char* pDictionary, xarray<guid> lstGuidsToExport  );



protected:
//  s32         LoadPropertyData            ( text_in& TextIn, xarray<prop_container>& pcArray );

    struct BlueprintReference 
    {
        char cName[MAX_PATH];
        char cFullPath[MAX_PATH];
        s16 iRefCount;
    };

    //Editor list of data referencing the blueprints(templates) to be used in the game
    xarray<BlueprintReference>  m_lstBlueprints;
    
    xbool                       m_bDirty;

#endif // X_EDITOR
};

//=========================================================================

extern bin_level    g_BinLevelMgr;

// HACKOMOTRON - need to stop creating player objects that are in the level export
extern u64          g_PlayerGuid;

//=========================================================================
// END
//=========================================================================


//=============================================================================


#endif//    BIN_LEVEL_HPP
