#include "BaseStdAfx.h"
#include "Project.hpp"
#include "Parsing\TextIn.hpp"
#include "Parsing\TextOut.hpp"

//==============================================================================
//  Global declarations
//==============================================================================
project_data        g_Project;   
editor_settings     g_Settings;

#if defined( mreed )
xbool g_right_stick_swap_xy = TRUE;
#else
xbool g_right_stick_swap_xy = FALSE;
#endif

xbool g_EditorInvertY = FALSE;
extern xbool g_MirrorWeapon;

xbool g_bCensoredBuild = FALSE;

//==============================================================================
// FUNCTIONS
//==============================================================================

//==============================================================================

project_data::project_data( void )
{
    m_bProjectOpen  = FALSE;
    m_bFileAccess   = FALSE;
}

//==============================================================================

void project_data::Save( void )
{  
    CFileStatus status;
    char FullFileName[256];
    GetFileName( FullFileName );

    if( CFile::GetStatus( FullFileName, status ) )   // static function
    {
        if (status.m_attribute & CFile::readOnly)
        {
            x_DebugMsg("WARNING: The Project File could not be saved, it is marked readonly!\n");
            return;
        }
    }

    prop_interface::OnSave( FullFileName );
}

//==============================================================================

void project_data::Load( const char* pFileName )
{
    ASSERT( pFileName );
    m_bLoadError = FALSE;

    char Drive[256];
    char Path[256];
    char FileName[256];
    char Ext[256];

    x_splitpath( pFileName, Drive, Path, FileName, Ext );
    if( x_stricmp( Ext, ".project" ) )
        x_throw( xfs("Unable to open project. It as the wrong extension [%s] expected [.project]", Ext ));

    //
    // Read the actual file
    //

    // Make sure that all is clear
    Clear();

    prop_interface::OnLoad( pFileName );
    m_bProjectOpen = TRUE;
}

//==============================================================================

void  project_data::OnLoad( text_in&  TextIn )
{
    m_bFileAccess = TRUE;

    x_try;
        prop_interface::OnLoad( TextIn );
    x_catch_begin;
        m_bFileAccess = FALSE;
    x_catch_end_ret;

    m_bFileAccess = FALSE;
}

//==============================================================================

void project_data::OnSave( text_out& TextOut )
{
    m_bFileAccess = TRUE;

    x_try;
        prop_interface::OnSave( TextOut );
/* not needed
        for( s32 i=0; i<m_lTheme.GetCount(); i++ )
        {
            char FileName[256];
            m_lTheme[i].GetFileName( FileName );
            m_lTheme[i].OnSave( FileName );  
        }
*/
    x_catch_begin;
        m_bFileAccess = FALSE;
    x_catch_end_ret;

    m_bFileAccess = FALSE;
}

//==============================================================================

void project_data::OnEnumProp( prop_enum&  List )
{
    List.PropEnumString( "Project", "Header which contains information about the working project", PROP_TYPE_HEADER );

    List.PropEnumString( "Project\\Level Directory", "", 0 );
    List.PropEnumString( "Project\\Level Name",      "", 0 );

    {
        s32 SubID = List.PushPath( "Project\\" );
        project_theme::OnEnumProp( List );
        List.PopPath( SubID );
    }

    for( s32 i=0; i<m_lTheme.GetCount(); i++ )
    {
        List.PropEnumString( xfs("Project\\Theme[%d]",i),  "Theme header", PROP_TYPE_HEADER|PROP_TYPE_READ_ONLY );
        List.PropEnumString( xfs("Project\\Theme[%d]\\FileName",i), "", PROP_TYPE_DONT_SHOW );

        if( m_bFileAccess == FALSE )
        {
            s32 SubID = List.PushPath( xfs("Project\\Theme[%d]\\", i) );
            m_lTheme[i].OnEnumProp( List );
            List.PopPath( SubID );
        }
    }
}

//==============================================================================

xbool project_data::OnProperty( prop_query& I )
{
    s32 Id;

    project_theme::OnProperty( I );

    if (I.VarString("Project\\Level Directory", m_DFSDirectory, 256))
        return TRUE;

    if (I.VarString("Project\\Level Name", m_DFSName, 256))
        return TRUE;


    //
    // Get properties from the parent class
    //
    {
        Id = I.PushPath( "Project\\" );
        if( project_theme::OnProperty( I ) )
        {
            return TRUE;
        }
        I.PopPath( Id );
    }

    //
    // This is a cheat so that we don't need to look at the properties of the theams in a seperate place
    //
    if( m_bFileAccess == FALSE )
    {
        if( m_lTheme.GetCount() && I.IsSimilarPath( "Project\\Theme[]\\" ) )
        {
            Id = I.PushPath( "Project\\Theme[]\\" );

            if( m_lTheme[I.GetIndex(0)].OnProperty( I ) )
            {
                return TRUE;
            }

            I.PopPath( Id );
        }
    }

    //
    // Now check the normal properties
    //
    if( I.IsVar( "Project\\Theme[]" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarString( m_lTheme[ I.GetIndex(0) ].GetName(), 256 );
        }
        else
        {
            ASSERT( 0 );
        }
    }
    else if( I.IsVar( "Project\\Theme[]\\FileName" ) )
    {
        if( I.IsRead() )
        {
            char FileName[256];
            m_lTheme[ I.GetIndex(0) ].GetFileName( FileName );
            I.SetVarString( FileName, 256 );
        }
        else
        {
            ASSERT( m_bFileAccess == TRUE );
            x_try;
            InsertTheme( I.GetVarString() );
            x_catch_display_msg(xfs("Could not load theme:\n%s",I.GetVarString()));
        }
    }
    else if( I.VarString( "Project", m_Name, 256 ) )
    {
        // Nothing to do
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}


//==============================================================================

void project_data::CreateTheme( const char* pFileName )
{
    project_theme& Theme = m_lTheme.Append();

    char Drive[256];
    char Path[256];
    char FileName[256];
    char Ext[256];
    char NewPath[X_MAX_PATH];

    x_splitpath( pFileName, Drive, Path, FileName, Ext );
    x_makepath ( NewPath, Drive, Path, FileName, "theme" );

    x_try;
        Theme.OnNew     ( NewPath );
    x_catch_begin;
        m_lTheme.Delete( m_lTheme.GetCount()-1 );  
    x_catch_end_ret;
}

//==============================================================================

void project_data::Clear( void )
{
    project_theme::Clear();

    for( s32 i=0; i<m_lTheme.GetCount(); i++ )
    {
        m_lTheme[i].Clear();
    }
    
    m_lTheme.Clear();
    m_bProjectOpen = FALSE;
    m_bFileAccess  = FALSE;
}

//==============================================================================

void project_data::Close( void )
{
    Clear();
}

//==============================================================================

void project_data::OnNew( const char* pFileName )
{
    char Drive[256];
    char Path[256];
    char FileName[256];
    char Ext[256];
    char NewPath[X_MAX_PATH];

    x_splitpath( pFileName, Drive, Path, FileName, Ext );

    if( x_stricmp( Ext,".Project") )
        x_throw( xfs("The extension of the file name must be a [.project] I got a [%s]", Ext ));

    x_makepath ( NewPath, Drive, Path, FileName, Ext );

    // Mkae sure that all is clear
    Clear();
    project_theme::OnNew( NewPath );

    m_bProjectOpen = TRUE;
}

//==============================================================================

void project_data::InsertTheme( const char* pFileName )
{
    project_theme& Theme = m_lTheme.Append();

    x_try;

        Theme.OnLoad( pFileName );

    x_catch_begin;        

        x_display_exception_msg( xfs("Could not load theme:\n%s",pFileName) );

        m_lTheme.Delete( m_lTheme.GetCount()-1 );
        m_bLoadError = TRUE;

    x_catch_end_ret;
}

//==============================================================================

void project_data::RemoveTheme( const char* pFileName )
{
    for (s32 i = 0; i < m_lTheme.GetCount(); i++)
    {
        project_theme& Theme = m_lTheme.GetAt(i);
        if (x_stricmp(Theme.GetName(), pFileName) == 0)
        {
            //Found it! Remove it!
            m_lTheme.Delete(i);
            return;
        }
    }
}

//==============================================================================

void project_data::GetFileName( char* pFileName )
{
    x_sprintf( pFileName, "%s%s%s", m_WorkingPath, m_Name, ".project" );
}

//==============================================================================

s32 project_data::GetFirstResourceDir( char* pWorkingDir )
{
    x_strcpy( pWorkingDir, GetResourcePath() );

    return 0;
}

//==============================================================================

s32 project_data::GetNextResourceDir( s32 Index, char* pWorkingDir )
{
    if( Index >= m_lTheme.GetCount() )
        return -1;
    x_strcpy( pWorkingDir, m_lTheme[Index].GetResourcePath() );
    Index++;
    return Index;
}

//==============================================================================

s32 project_data::GetFirstBlueprintDir( char* pWorkingDir )
{
    x_strcpy( pWorkingDir, GetBlueprintPath() );

    return 0;
}

//==============================================================================

s32 project_data::GetNextBlueprintDir( s32 Index, char* pWorkingDir )
{
    if( Index >= m_lTheme.GetCount() )
        return -1;
    x_strcpy( pWorkingDir, m_lTheme[Index].GetBlueprintPath() );
    Index++;
    return Index;
}

//==============================================================================

s32 project_data::GetThemeCount( void ) 
{ 
    return m_lTheme.GetCount(); 
}

//==============================================================================

const char* project_data::GetThemeName ( s32 Index )
{
    if (m_lTheme.GetCount() > Index)
    {
        return m_lTheme.GetAt(Index).GetName();
    }
    else
    {
        ASSERT(FALSE);
    }
    return "";
}

//==============================================================================

void project_data::GetThemePath ( s32 Index, char* pPath )
{
    if (m_lTheme.GetCount() > Index)
    {
        char FullFileName[256];
        m_lTheme.GetAt(Index).GetFileName(FullFileName);
        x_sprintf( pPath, "%s", FullFileName );
    }
    else
    {
        ASSERT(FALSE);
    }
}

//==============================================================================

const char* project_data::GetThemeBlueprintDir ( s32 Index )
{
    if (m_lTheme.GetCount() > Index)
    {
        return m_lTheme.GetAt(Index).GetBlueprintPath();
    }
    else
    {
        ASSERT(FALSE);
    }
    return "";
}

//==============================================================================

const char* project_data::GetBlueprintDirForTheme ( const char* pTheme )
{
    if (x_stricmp(pTheme, GetName()) == 0 )
    {
        //its the project
        return GetBlueprintPath();
    }

    for (s32 i = 0; i < m_lTheme.GetCount(); i++ )
    {
        project_theme& Theme = m_lTheme.GetAt(i);
        if (x_stricmp(Theme.GetName(), pTheme) == 0)
        {
            //found it
            return Theme.GetBlueprintPath();
        }
    }

    return ""; //found nothing
}


//==============================================================================

const char* project_data::GetResourceDirForTheme ( const char* pTheme )
{
    if (x_stricmp(pTheme, GetName()) == 0 )
    {
        //its the project
        return GetBlueprintPath();
    }

    for (s32 i = 0; i < m_lTheme.GetCount(); i++ )
    {
        project_theme& Theme = m_lTheme.GetAt(i);
        if (x_stricmp(Theme.GetName(), pTheme) == 0)
        {
            //found it
            return Theme.GetResourcePath();
        }
    }

    return ""; //found nothing
}

//==============================================================================
//==============================================================================
//==============================================================================
// PROJECT THEME
//==============================================================================
//==============================================================================
//==============================================================================
 
//==============================================================================

project_theme::project_theme( void )
{
    m_Name[0]           =0;        
    m_WorkingPath[0]    =0; 
    m_BlueprintPath[0]  =0;
    m_ResourcePath[0]   =0;
    m_ParticlePath[0]   =0;
}
  
//==============================================================================

void project_theme::OnNew( const char* pFileName )
{
    char Drive[256];
    char Path[256];
    char FileName[256];
    char Ext[256];

    if( x_strlen( pFileName ) >= 256 )
        x_throw( "The path was too long. Please talk to programmers about this error" );

    x_splitpath( pFileName, Drive, Path, FileName, Ext );

    x_sprintf( m_Name,          "%s",           FileName );
    x_sprintf( m_WorkingPath,   "%s%s%s\\",     Drive, Path, FileName );
    x_sprintf( m_BlueprintPath, "%sBlueprint",  m_WorkingPath );
    x_sprintf( m_ResourcePath,  "%sResource",   m_WorkingPath );
    x_sprintf( m_ParticlePath,  "%s\\PC", g_Settings.GetReleasePath() );

    if( CreateDirectory( m_WorkingPath,     NULL ) == FALSE ||
        CreateDirectory( m_BlueprintPath,   NULL ) == FALSE || 
        CreateDirectory( m_ResourcePath,    NULL ) == FALSE )
    {
        LPVOID lpMsgBuf;
        FormatMessage( 
            FORMAT_MESSAGE_ALLOCATE_BUFFER | 
            FORMAT_MESSAGE_FROM_SYSTEM | 
            FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            GetLastError(),
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
            (LPTSTR) &lpMsgBuf,
            0,
            NULL 
        );
        
        x_throw( (const char*)lpMsgBuf );
    }

    OnSave( xfs( "%s%s%s", m_WorkingPath, FileName, Ext ) );
}

//==============================================================================

void project_theme::OnEnumProp( prop_enum& List )
{
    List.PropEnumString( "Name",          "Name of the Theme", PROP_TYPE_DONT_SHOW );
    List.PropEnumString( "WorkingPath",   "Path in which the project is base on", PROP_TYPE_DONT_SHOW );
    List.PropEnumString( "BlueprintPath", "", PROP_TYPE_DONT_SHOW );
    List.PropEnumString( "ResourcePath",  "", PROP_TYPE_DONT_SHOW );
}

//==============================================================================

xbool project_theme::OnProperty( prop_query& I )
{
    if( I.VarString( "Name", m_Name, 256 ) )
    {
        // Nothing to do...
    }
    else if( I.VarString( "WorkingPath", m_WorkingPath, 256 ) )
    {
        // Nothing to do...
    }
    else if( I.VarString( "BlueprintPath", m_BlueprintPath, 256 ) )
    {
        // Nothing to do...
    }
    else if( I.VarString( "ResourcePath", m_ResourcePath, 256 ) )
    {
        // Nothing to do...
    } 
    else if( I.VarString( "ParticlePath", m_ParticlePath, 256 ) )
    {
        // Nothing to do...
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//==============================================================================

void project_theme::OnClose( void )
{
    // make sure to save it all
}

//==============================================================================

void project_theme::GetFileName( char* pFileName )
{
    x_sprintf( pFileName, "%s%s%s", m_WorkingPath, m_Name, ".theme" );
}

//==============================================================================

void project_theme::Clear( void )
{
    m_Name[0]=0;         
    m_WorkingPath[0]=0;  
    m_BlueprintPath[0]=0;
    m_ResourcePath[0]=0; 
}


//==============================================================================
//==============================================================================
//==============================================================================
// KEEP_DIRECTORY
//==============================================================================
//==============================================================================
//==============================================================================

// Default constructor - stores the current directory
keep_directory::keep_directory()
{
    // Grab the current directory
    ::GetCurrentDirectory(sizeof(m_KeepDirectory), m_KeepDirectory) ;
}

//==============================================================================

// Utility constructor - stores the current directory, then switches to
// the passed in directory.
keep_directory::keep_directory( const char* pDirectory )
{
    // MUST HAVE THIS!
    ASSERT(pDirectory) ;

    // Grab the current directory
    ::GetCurrentDirectory(sizeof(m_KeepDirectory), m_KeepDirectory) ;

    // Switch to new directory
    ::SetCurrentDirectory(pDirectory) ;
}

//==============================================================================

// Destructor - restores the current directory
keep_directory::~keep_directory()
{
    // Set the current directory back to what it was at construction
    ::SetCurrentDirectory(m_KeepDirectory) ;
}


//==============================================================================
//==============================================================================
//==============================================================================
// EDITOR SETTINGS
//==============================================================================
//==============================================================================
//==============================================================================

//==============================================================================

editor_settings::editor_settings( void )
{
    // No config yet...
    m_iConfig = -1 ;
    m_Volume = 1.0f;

    // NOTE: for now put PC at end!
    // When exporting lighting, it will reset color table info back to PC
    m_lPlatform.Add( "XBOX", platform_info( PLATFORM_XBOX, FALSE ) );
    m_lPlatform.Add( "PS2",  platform_info( PLATFORM_PS2, TRUE )  );
    m_lPlatform.Add( "PC",   platform_info( PLATFORM_PC, FALSE )   );

    // Lookup the current directory and keep this for saving/loading
    m_SettingsDirectory[0] = 0 ;
    ::GetCurrentDirectory(sizeof(m_SettingsDirectory), m_SettingsDirectory) ;
}

//==============================================================================

void editor_settings::LoadConfigs( const char* pFile )
{
    // Switch to settings directory (destructor restores previous directory)
    keep_directory KeepDirectory(m_SettingsDirectory) ;

    // Delete all current configs
    m_Configs.Clear() ;

    // Create and add default config
    config& Config = m_Configs.Append() ;
    x_strcpy(Config.m_Name,         "CUSTOM") ;
    
    // NOTE: All of this is okay to be hard coded paths - it's for when
    //       you run the editor for the first time on your computer and
    //       have not setup the paths/config yet.

    // A51 artists-designers?
    if (x_stristr(m_SettingsDirectory, "C:\\GameData\\A51\\Apps"))
    {
        x_strcpy(Config.m_CompilerPath, "C:\\GameData\\A51\\Apps\\Compilers") ;
        x_strcpy(Config.m_ReleasePath,  "C:\\GameData\\A51\\Release"        ) ;
        x_strcpy(Config.m_SourcePath,   "C:\\GameData\\A51\\Source"         ) ;
        x_strcpy(Config.m_TempPath,     "C:\\GameData\\A51\\Apps\\Temp"     ) ;
    }
    else
    // A51 programmers?
    if (x_stristr(m_SettingsDirectory, "C:\\Projects\\A51\\Apps\\Editor"))
    {
        x_strcpy(Config.m_CompilerPath, "C:\\GameData\\A51\\Apps\\Compilers_Dev") ;
        x_strcpy(Config.m_ReleasePath,  "C:\\GameData\\A51\\Release"        ) ;
        x_strcpy(Config.m_SourcePath,   "C:\\GameData\\A51\\Source"         ) ;
        x_strcpy(Config.m_TempPath,     "C:\\GameData\\A51\\Apps\\Temp"     ) ;
    }
    else
    // AOTK artists-designers?
    if (x_stristr(m_SettingsDirectory, "C:\\GameData\\AoTK\\Apps"))
    {
        x_strcpy(Config.m_CompilerPath, "C:\\GameData\\AoTK\\Apps\\Compilers") ;
        x_strcpy(Config.m_ReleasePath,  "C:\\GameData\\AoTK\\Release"        ) ;
        x_strcpy(Config.m_SourcePath,   "C:\\GameData\\AoTK\\Source"         ) ;
        x_strcpy(Config.m_TempPath,     "C:\\GameData\\AoTK\\Apps\\Temp"     ) ;
    }
    else
    // AOTK programmers?
    if (x_stristr(m_SettingsDirectory, "C:\\Projects\\AoTK\\Apps\\Editor"))
    {
        x_strcpy(Config.m_CompilerPath, "C:\\GameData\\AoTK\\Apps\\Compilers_Dev") ;
        x_strcpy(Config.m_ReleasePath,  "C:\\GameData\\AoTK\\Release"        ) ;
        x_strcpy(Config.m_SourcePath,   "C:\\GameData\\AoTK\\Source"         ) ;
        x_strcpy(Config.m_TempPath,     "C:\\GameData\\AoTK\\Apps\\Temp"     ) ;
    }
    else
    {
        // Who knows!
        x_strcpy(Config.m_CompilerPath, "C:\\GameData\\A51\\Apps\\Compilers") ;
        x_strcpy(Config.m_ReleasePath,  "C:\\GameData\\A51\\Release"        ) ;
        x_strcpy(Config.m_SourcePath,   "C:\\GameData\\A51\\Source"         ) ;
        x_strcpy(Config.m_TempPath,     "C:\\GameData\\A51\\Apps\\Temp"     ) ;
    }

    // Use custom config by default for backwards compatability
    m_iConfig = 0 ;

    // Build extra set of configs from the file "EditorConfigs.ini"
    token_stream TOK ;
    TOK.SetDelimeter(",[]{}()<>") ;
    if (TOK.OpenFile(pFile))
    {
        // Look until all read
        while(TOK.IsEOF() == FALSE)
        {
            // Read next token
            TOK.Read() ;

            // New config?
            if (x_stricmp("BeginConfig", TOK.String()) == 0)
            {
                // Create new config and setup name
                config& Config = m_Configs.Append() ;
                x_strcpy(Config.m_Name, TOK.ReadString()) ;

                // Read in the config values
                while( (x_stricmp("EndConfig", TOK.String()) != 0) && (TOK.IsEOF() == FALSE) )
                {
                    // Read next token
                    TOK.Read() ;

                    // CompilerPath?
                    if (x_stricmp("CompilerPath", TOK.String()) == 0)
                        x_strcpy(Config.m_CompilerPath, TOK.ReadString()) ;
                    else 
                    // ReleasePath?
                    if (x_stricmp("ReleasePath", TOK.String()) == 0)
                        x_strcpy(Config.m_ReleasePath, TOK.ReadString()) ;
                    else 
                    // SourcePath?
                    if (x_stricmp("SourcePath", TOK.String()) == 0)
                        x_strcpy(Config.m_SourcePath, TOK.ReadString()) ;
                    else 
                    // TempPath?
                    if (x_stricmp("TempPath", TOK.String()) == 0)
                        x_strcpy(Config.m_TempPath, TOK.ReadString()) ;
                }
            }
        }
        
        // Close
        TOK.CloseFile() ;
    }

    // Build config enum
    char* pEnum = m_ConfigEnum ;
    for (s32 i = 0 ; i < m_Configs.GetCount() ; i++)
    {
        x_strcpy(pEnum, m_Configs[i].m_Name) ;
        pEnum += x_strlen(m_Configs[i].m_Name)+1 ;
    }
    *pEnum++ = 0 ;
    *pEnum++ = 0 ;
    ASSERTS( (pEnum <= &m_ConfigEnum[sizeof(m_ConfigEnum)]), "Increase size of m_ConfigEnum array") ;
}

//==============================================================================

s32 editor_settings::GetPlatformCount( void ) const
{
    return( m_lPlatform.GetCount() );
}

//==============================================================================

platform editor_settings::GetPlatformType( const char* pString ) const
{
    return( m_lPlatform.GetValue( pString ).Platform );
}

//==============================================================================

const char* editor_settings::GetPlatformString( platform Type ) const
{
    for( s32 i=0; i<m_lPlatform.GetCount(); i++ )
    {
        if( m_lPlatform.GetValueFromIndex(i).Platform == Type )
            return m_lPlatform.GetStringFromIndex( i );
    }

    return NULL;
}

//==============================================================================

platform editor_settings::GetPlatformTypeI( s32 Index ) const
{
    return( m_lPlatform.GetValueFromIndex( Index ).Platform );
}

//==============================================================================

const char* editor_settings::GetPlatformStringI( s32 Index ) const
{
    return( m_lPlatform.GetStringFromIndex( Index ) );
}

//==============================================================================

xbool editor_settings::GetPlatfromExportI( s32 Index ) const
{
    return( m_lPlatform.GetValueFromIndex( Index ).bExport );
}

//==============================================================================

void editor_settings::SetPlatfromExportI( s32 Index, xbool State )
{
    m_lPlatform[Index].bExport = State;
}

//==============================================================================

void  editor_settings::OnEnumProp( prop_enum&  List )
{
    List.PropEnumHeader( "Settings", "Header for the editor settings", 0 );
    
    List.PropEnumEnum  ( "Settings\\Config", m_ConfigEnum, "Current config to use.", PROP_TYPE_MUST_ENUM );

    // Only allow settings to be modified if using custom config
    u32 Flags = PROP_TYPE_READ_ONLY ;
    if (m_iConfig == 0)
        Flags = 0 ;

    List.PropEnumString( "Settings\\CompilerPath", "This is where the compiler system will try to load all its compilers from", Flags );
    List.PropEnumString( "Settings\\ReleasePath",  "This is where all the compiled resources will end up.",                     Flags );
    List.PropEnumString( "Settings\\SourcePath",   "This is where all the source assets are located.",                          Flags );
    List.PropEnumString( "Settings\\TempPath",     "This is where all temp files created by the editor will go.",               Flags );
    
    List.PropEnumBool  ( "Settings\\Controller swap right X/Y", "Set this flag on if you have a pink controller.", 0 ) ;
    List.PropEnumBool  ( "Settings\\Controller Invert Y", "Set this flag to invert the Y axis.", 0 ) ;
    List.PropEnumBool  ( "Settings\\Controller left handed weapons", "Player uses left handed weapons.", 0 ) ;
    List.PropEnumFloat ( "Settings\\Volume", "Set the overall volume for the app 0 -> 1", 0 ) ;

    List.PropEnumBool  ( "Settings\\Censored", "Set this flag to make Censored Build conditions true.", 0 );

    for( s32 i=0; i<GetPlatformCount(); i++ )
    {
        List.PropEnumBool  ( xfs("Settings\\Export %s", GetPlatformStringI(i)), "Export level throw the world editor for the specify platform.", 0 ) ;
    }
}

//==============================================================================

xbool editor_settings::OnProperty( prop_query& I )
{
    // Config
    if( I.IsVar("Settings\\Config" ) )
    {
        if( I.IsRead () )
        {
            I.SetVarEnum(m_Configs[m_iConfig].m_Name) ;
        }
        else
        {
            // Search for config index
            for (s32 i = 0 ; i < m_Configs.GetCount() ; i++)
            {
                if (x_stricmp(I.GetVarEnum(), m_Configs[i].m_Name) == 0)
                {
                    m_iConfig = i ;
                    return TRUE ;
                }
            }
        }
        return TRUE;
    }

    // Lookup current config
    config* pConfig = &m_Configs[m_iConfig] ;
    
    // If writing to property, always right to custom config (config 0)
    if (I.IsRead() == FALSE)
        pConfig = &m_Configs[0] ;

    // Read/Write settings
    if( I.VarString( "Settings\\CompilerPath", pConfig->m_CompilerPath, sizeof(pConfig->m_CompilerPath) ) )
    {
        return TRUE ;
    }
    else if( I.VarString( "Settings\\ReleasePath", pConfig->m_ReleasePath, sizeof(pConfig->m_ReleasePath) ) )
    {
        return TRUE ;
    }
    else if( I.VarString( "Settings\\SourcePath", pConfig->m_SourcePath, sizeof(pConfig->m_SourcePath) ) )
    {
        return TRUE ;
    }
    else if( I.VarString( "Settings\\TempPath", pConfig->m_TempPath, sizeof(pConfig->m_TempPath) ) )
    {
        return TRUE ;
    }

    // PS2 controller
    if( I.VarBool( "Settings\\Inverted PS2 Controller", g_right_stick_swap_xy ) )
        return TRUE ;
    if( I.VarBool( "Settings\\Controller swap right X/Y", g_right_stick_swap_xy ) )
        return TRUE ;
    if( I.VarBool( "Settings\\Controller Invert Y", g_EditorInvertY ) )
        return TRUE ;
    if( I.VarBool( "Settings\\Controller left handed weapons", g_MirrorWeapon ) )
        return TRUE ;

    // Volume
    if( I.IsVar( "Settings\\Volume" ))
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_Volume );
        }
        else
        {
            m_Volume = I.GetVarFloat();
            m_Volume = min( m_Volume, 1.0f );
            m_Volume = max( m_Volume, 0.0f );
            IAL_SetSystemVolume( m_Volume );
        }
        return TRUE;
    }

    // Censored build
    if( I.VarBool( "Settings\\Censored", g_bCensoredBuild ) )
        return TRUE;

    // export properties
    for( s32 i=0; i<GetPlatformCount(); i++ )
    {
        if( I.VarBool( xfs("Settings\\Export %s", GetPlatformStringI(i)), m_lPlatform[i].bExport ))
            return TRUE;
    }

    return FALSE;
}

//==============================================================================

void editor_settings::OnSave( const char* FileName )
{
    // Switch to settings directory (destructor restores previous directory)
    keep_directory KeepDirectory(m_SettingsDirectory) ;

    // Call base class
    prop_interface::OnSave(FileName) ;
}

//==============================================================================

void editor_settings::OnLoad( const char* FileName )
{
    // Switch to settings directory (destructor restores previous directory)
    keep_directory KeepDirectory(m_SettingsDirectory) ;

    // If the file does not exist, then continue (defaults are setup in constructor)
    X_FILE* pFile = x_fopen(FileName, "rb") ;
    if (!pFile)
        return ;
    x_fclose(pFile) ;   // Let's not forget this!

    // Call base class
    prop_interface::OnLoad(FileName) ;

}

//==============================================================================

void editor_settings::CleanTemp( void )
{
    CFileFind Find;

    CString Path = GetTempPath();
    Path += "\\*";

    if( Find.FindFile( Path ) != 0 )
    {
        while( Find.FindNextFile() )
        {
            if( !Find.IsDirectory() )
            {
                CString File = Find.GetFilePath();
                DeleteFile( File );
            }
        }
    }
}
