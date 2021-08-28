#ifndef AUDIO_PACKAGE_HPP
#define AUDIO_PACKAGE_HPP

#include "x_types.hpp"
#include "audio_private.hpp"

struct music_intensity
{
    u8  Intensity;
    u8  Descriptor[31];
};

class audio_package
{

//------------------------------------------------------------------------------
// Public defines.

public:

friend class audio_voice_mgr;
friend class audio_hardware;
friend class audio_stream_mgr;

#define AUDIO_PACKAGE_FILENAME_LENGTH (128)

//------------------------------------------------------------------------------
// Private structs

private:

struct package_link     
{                       
    package_link*  pPrev;
    package_link*  pNext;
    audio_package* pPackage;
};

//------------------------------------------------------------------------------
// Public functions.

public:

                            audio_package               ( void );
                           ~audio_package               ( void );
            xbool           Init                        ( const char* pFilename );
            void            Kill                        ( void );
                                   
            void            SetUserVolume               ( f32 Volume );
            void            SetUserPitch                ( f32 Pitch );
            void            SetUserEffectSend           ( f32 EffectSend );
            void            SetUserNearFalloff          ( f32 NearFalloff );
            void            SetUserFarFalloff           ( f32 FarFalloff );
            void            SetUserNearDiffuse          ( f32 NearDiffuse );
            void            SetUserFarDiffuse           ( f32 FarDiffuse );

            char*           GetMusicType                ( void );
            s32             GetMusicIntensity           ( music_intensity* & Intensity );
                                    
            void            ComputeVolume               ( void );
            void            ComputePitch                ( void );
            void            ComputeEffectSend           ( void );
            void            ComputeNearFalloff          ( void );
            void            ComputeFarFalloff           ( void );
            void            ComputeNearDiffuse          ( void );
            void            ComputeFarDiffuse           ( void );
                            
inline      f32             GetUserVolume               ( void )                { return m_UserVolume; }
inline      f32             GetUserPitch                ( void )                { return m_UserPitch; }
inline      f32             GetUserEffectSend           ( void )                { return m_UserEffectSend; }
inline      f32             GetUserNearFalloff          ( void )                { return m_UserNearFalloff; }
inline      f32             GetUserFarFalloff           ( void )                { return m_UserFarFalloff; }
                                                                                
inline      f32             GetComputedVolume           ( void )                { return m_Volume; }
inline      f32             GetComputedPitch            ( void )                { return m_Pitch; }
inline      f32             GetComputedEffectSend       ( void )                { return m_EffectSend; }
inline      f32             GetComputedNearFalloff      ( void )                { return m_NearFalloff; }
inline      f32             GetComputedFarFalloff       ( void )                { return m_FarFalloff; } 
inline      f32             GetComputedNearDiffuse      ( void )                { return m_NearDiffuse; }
inline      f32             GetComputedFarDiffuse       ( void )                { return m_FarDiffuse; }
                                                                                
inline      char*           GetPackageIdentifier        ( void )                { return m_Filename; }

//------------------------------------------------------------------------------
// Private functions.

private:

            u32             LoadHotSample               ( X_FILE* f, 
                                                          hot_sample* pHotSample, 
                                                          u32 Aram );

//------------------------------------------------------------------------------
// Private data.

private:

    char                    m_Filename[AUDIO_PACKAGE_FILENAME_LENGTH];    // Filename
    package_link            m_Link;                         // Link
    package_header          m_Header;                       // Package header.
                                                            
    xbool                   m_IsLoaded;                     // Flag: Is the package loaded?
                                                            
    f32*                    m_MasterVolumeFader;            // Pointer to master volume fader.
    f32                     m_DuckVolume;                   // Package duck volume.
    f32                     m_UserVolume;                   // Application settable modifier.
    f32                     m_Volume;                       // The actual volume of the package.
                                                            
    f32*                    m_MasterPitchFader;             // Pointer to master volume fader.
    f32                     m_UserPitch;                    // Application settable modifier.
    f32                     m_Pitch;                        // The actual pitch of the package.
                                                            
    f32*                    m_MasterEffectSendFader;        // Pointer to master effect send fader.
    f32                     m_UserEffectSend;               // Application settable modifier.
    f32                     m_EffectSend;                   // The actual effect send of the package.
                                                            
    f32*                    m_MasterNearFalloffFader;       // Pointer to master near falloff fader.
    f32                     m_UserNearFalloff;              // Application settable modifier.
    f32                     m_NearFalloff;                  // Near Falloff
                                                            
    f32*                    m_MasterFarFalloffFader;        // Pointer to master far falloff fader.
    f32                     m_UserFarFalloff;               // Application settable modifier.
    f32                     m_FarFalloff;                   // Near Falloff

    f32*                    m_MasterNearDiffuseFader;       // Pointer to master near diffuse fader.
    f32                     m_UserNearDiffuse;              // Application settable modifier.
    f32                     m_NearDiffuse;                  // Near Diffusion.

    f32*                    m_MasterFarDiffuseFader;        // Pointer to master far diffuse fader.
    f32                     m_UserFarDiffuse;               // Application settable modifier.
    f32                     m_FarDiffuse;                   // Far Diffusion.
                                                            
    descriptor_identifier*  m_IdentifierTable;              // Pointer to the identifier table
    char*                   m_IdentifierStringTable;        // Pointer to the identifier strings
    char*                   m_LipSyncTable;                 // Base address of the lip sync data.
    char*                   m_BreakPointTable;              // Base address of the breakpoint data.
    char*                   m_MusicData;                    // Base address of the music data
    u32*                    m_DescriptorTable;              // Pointer to the descriptor table
    u16*                    m_DescriptorBuffer;             // Pointer to the descriptor buffer
    void*                   m_HotSamples;                   // Table of hot samples.
    void*                   m_WarmSamples;                  // Table of warm samples.
    void*                   m_ColdSamples;                  // Table of cold samples.

    u16*                    m_SampleIndices[NUM_TEMPERATURES];
    u32                     m_AudioRam;

friend class audio_mgr;

};

#endif // AUDIO_PACKAGE_HPP
