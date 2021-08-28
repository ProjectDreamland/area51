#ifndef PROJECT_DATA_HPP
#define PROJECT_DATA_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Auxiliary\MiscUtils\Property.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"

//=========================================================================
// REFS
//=========================================================================
class project_data;


//=========================================================================
// KEEP_DIRECTORY
//=========================================================================
// A simple class that stores the current directory upon construction
// and restores that directory upon deconstruction.
// Can be used around functions that change the current directory.
//=========================================================================
class keep_directory
{
public:
    
    // Default constructor - stores the current directory
    keep_directory() ;  

    // Utility constructor - stores the current directory, then switches to
    // the passed in directory.
    keep_directory( const char* pDirectory ) ;  

    // Destructor - restores the current directory
    ~keep_directory() ;

private:
    char    m_KeepDirectory[512] ;  // Directory at construction
} ;


//=========================================================================
// SETTINGS
//=========================================================================
class editor_settings : public prop_interface
{
public:
                        editor_settings     ( void );

            void        LoadConfigs         ( const char* pFile ) ;

    virtual void        OnEnumProp          ( prop_enum&  List );
    virtual xbool       OnProperty          ( prop_query& I    );
    virtual void        OnSave              ( const char* FileName );
    virtual void        OnLoad              ( const char* FileName );

    const char*         GetCompilerPath     ( void ){ return m_Configs[m_iConfig].m_CompilerPath; }
    const char*         GetReleasePath      ( void ){ return m_Configs[m_iConfig].m_ReleasePath;  }
    const char*         GetSourcePath       ( void ){ return m_Configs[m_iConfig].m_SourcePath; }
    const char*         GetTempPath         ( void ){ return m_Configs[m_iConfig].m_TempPath; }
    
    void                CleanTemp           ( void );

    s32                 GetPlatformCount    ( void ) const;
    platform            GetPlatformType     ( const char* pString ) const;
    const char*         GetPlatformString   ( platform Type ) const;
    platform            GetPlatformTypeI    ( s32 Index ) const;
    const char*         GetPlatformStringI  ( s32 Index ) const;
    xbool               GetPlatfromExportI  ( s32 Imdex ) const;
    void                SetPlatfromExportI  ( s32 Imdex, xbool State );
    
protected:

    struct config
    {
        // Data
        char    m_Name[256] ;           // Name of config
        char    m_CompilerPath[256] ;   // Location of compilers
        char    m_ReleasePath[256] ;    // Location of compiled data
        char    m_SourcePath[256] ;     // Location of source assets
        char    m_TempPath[256] ;       // Directory for temp editor files

        // Constructor
        config()
        {
            m_Name[0] = 0 ;
            m_CompilerPath[0] ;
            m_ReleasePath[0] = 0 ;
            m_SourcePath[0] = 0 ;
            m_TempPath[0] = 0 ;
        }
    };

    struct platform_info
    {
        platform_info( void ){}
        platform_info( platform aPlatform, xbool aExport ) : 
            Platform(aPlatform), bExport(aExport) {}

        platform    Platform;
        xbool       bExport;
    };

protected:    

    s32                         m_iConfig ;                 // Current config
    xarray<config>              m_Configs ;                 // Array of configs 
    char                        m_ConfigEnum[1024] ;        // Config enums for properties
    enum_list<platform_info>    m_lPlatform;                // List of platforms
    char                        m_SettingsDirectory[512] ;  // Directory to save/load settings
    f32                         m_Volume;
};

//=========================================================================
// THEME
//=========================================================================
class project_theme : public prop_interface
{
public:
                        project_theme       ( void );

    virtual void        OnEnumProp          ( prop_enum&  List );
    virtual xbool       OnProperty          ( prop_query& I    );
    virtual void        OnNew               ( const char* pFileName );
    virtual void        OnClose             ( void );
    virtual void        GetFileName         ( char* pFileName );

    const char*         GetName             ( void );
    const char*         GetWorkingPath      ( void );
    const char*         GetBlueprintPath    ( void );
    const char*         GetResourcePath     ( void );
    const char*         GetParticlePath     ( void );

    void                Clear               ( void );

protected:

    char     m_Name[256];                       // Name of the theam. Should be the same as the file name.
    char     m_WorkingPath[256];                // Contains the directory where the theam is rooted.
    char     m_BlueprintPath[256];              // Name of the blue print directory. This is really constant.
    char     m_ResourcePath[256];               // Name of the resource directory. This is really constant.
    char     m_ParticlePath[256];               // Path for particle fxd files..
};

//=========================================================================
// PROJECT
//=========================================================================
class project_data : public project_theme
{
public:
                        project_data            ( void );

    void                Save                    ( void );
    void                Load                    ( const char* pFileName );
    void                Close                   ( void );
    xbool               IsProjectOpen           ( void );

    virtual void        GetFileName             ( char* pFileName );
    void                CreateTheme             ( const char* pFileName );
    void                InsertTheme             ( const char* pFileName );
    void                RemoveTheme             ( const char* pFileName );

    s32                 GetThemeCount           ( void );
    const char*         GetThemeName            ( s32 Index );
    void                GetThemePath            ( s32 Index, char* pPath );
    const char*         GetThemeBlueprintDir    ( s32 Index );
    const char*         GetBlueprintDirForTheme ( const char* pTheme );
    const char*         GetResourceDirForTheme  ( const char* pTheme );
    
    s32                 GetFirstResourceDir     ( char* pPath );
    s32                 GetNextResourceDir      ( s32 Index, char* pPath );

    s32                 GetFirstBlueprintDir    ( char* pPath );
    s32                 GetNextBlueprintDir     ( s32 Index, char* pPath );

//    xbool               GetFirstResource        ( file_search& Data );
//    xbool               GetNextResource         ( file_search& Data );

//    xbool               GetFirstBlueprint       ( file_search& Data );
//    xbool               GetNextBlueprint        ( file_search& Data );

    virtual void        OnLoad                  ( text_in&  TextIn );
    virtual void        OnSave                  ( text_out& TextOut );
    virtual void        OnEnumProp              ( prop_enum&  List );
    virtual xbool       OnProperty              ( prop_query& I    );
    virtual void        OnNew                   ( const char* pFileName );

protected:
    
    void                Clear               ( void );

protected:

    xarray<project_theme>   m_lTheme;
    xbool                   m_bProjectOpen;
    xbool                   m_bLoadError;
    xbool                   m_bFileAccess;

public:
    char                    m_DFSName[256];
    char                    m_DFSDirectory[256];
};

//=========================================================================
// VARIABLES
//=========================================================================
extern project_data         g_Project;
extern editor_settings      g_Settings;

//=========================================================================
// INLINE
//=========================================================================

//=========================================================================

inline xbool project_data::IsProjectOpen( void )
{
    return m_bProjectOpen;
}

//=========================================================================

inline const char* project_theme::GetName( void )
{
    return m_Name;
}

//=========================================================================

inline const char* project_theme::GetWorkingPath( void )
{
    return m_WorkingPath;
}

//=========================================================================

inline const char* project_theme::GetBlueprintPath( void )
{
    return m_BlueprintPath;
}

//=========================================================================

inline const char* project_theme::GetResourcePath( void )
{
    return m_ResourcePath;
}

//=========================================================================

inline const char* project_theme::GetParticlePath( void )
{
    return m_ParticlePath;
}

//=========================================================================
// END
//=========================================================================
#endif
