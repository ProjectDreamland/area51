#ifndef LEVEL_LOADER_HPP
#define LEVEL_LOADER_HPP

class map_entry;

class level_loader
{
public:
                level_loader                ( void );
               ~level_loader                ( void );
    void        LoadLevel                   ( xbool         bFullLoad );
    void        LoadLevelFinish             ( void );
    void        UnloadLevel                 ( xbool         bFullUnload );
    void        LoadDFS                     ( const char*   pDFS );
    void        LoadInfo                    ( const char*   pPath );
    void        InitSlideShow               ( const char*   pSlideShowScriptFile );
    void        KillSlideShow               ( void );
    void        MountDefaultFilesystems     ( void );
    void        UnmountDefaultFilesystems   ( void );
private:
    xbool       LoadContent                 ( const map_entry& MapEntry );
    void        UnloadContent               ( void );
    void        LoadContentComplete         ( void );
    void        FetchManifest               ( void );
    void        OnPollReturn                ( void );

    s32         m_VoiceID;
    xbool       m_bFullLoad;
    xbool       m_LoadInProgress;
};

extern level_loader g_LevelLoader;

#endif // LEVEL_LOADER_HPP
