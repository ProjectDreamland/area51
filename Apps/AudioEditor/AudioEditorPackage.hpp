#ifndef AUDIO_EDITOR_PACKAGE_HPP
#define AUDIO_EDITOR_PACKAGE_HPP

//=========================================================================
// AUDIO EDITOR
//=========================================================================
#include "../EDRscDesc\RSCDesc.hpp"

enum{ EDITOR_BLANK, EDITOR_SIMPLE, EDITOR_COMPLEX, EDITOR_RANDOM, EDITOR_WEIGHTED };

#define LINEAR_ROLLOFF  (1<<1)
#define FAST_ROLLOFF    (1<<2)
#define SLOW_ROLLOFF    (1<<3)
//=========================================================================
// GLOBALS
//=========================================================================

extern char* g_pFaderList;

class editor_element;


//=========================================================================
// DESCRIPTOR
//=========================================================================

class editor_descriptor
{
public:
    xarray<editor_element> m_pElements;
    xarray<editor_element*> m_pReferencingElement;
    char            m_Label[128];    
    f32             m_Volume;
    f32             m_VolumeVar;
    f32             m_Pitch;
    f32             m_PitchVar;
    f32             m_Pan;
    s32             m_Priority;
    f32             m_EffectSend;
    f32             m_NearFalloff;
    f32             m_FarFalloff;
    f32             m_VolumeLfe;
    f32             m_VolumeCenter;
    u8              m_RollOff;
    f32             m_NearDiffuse;    
    f32             m_FarDiffuse;
    s32             m_PlayPercent;
    f32             m_ReplayDelay;

    s32             m_Type;

                    editor_descriptor   ();    
    void            Clear               ( void );
    void            OnEnumProp          ( prop_enum& List, const char* pParent );
    xbool           OnProperty          ( prop_query& I, const char* pParent );
    void            OnCheckIntegrity    ( const char* pRscName );
};

//=========================================================================
// ELEMENT
//=========================================================================

class editor_element
{
public:    
    enum{ HOT, WARM, COLD };


    editor_descriptor      m_ParentDesc;
    s32             m_ReferenceDescIndex;
    s32             m_Temperature;
    char            m_pSampleName[128];
    char            m_pSamplePathName[128];
    char            m_pReferenceLabel[128];

    f32             m_Volume;
    f32             m_VolumeVar;
    f32             m_Pitch;
    f32             m_PitchVar;
    f32             m_Pan;
    s32             m_Priority;
    f32             m_EffectSend;
    f32             m_NearFalloff;
    f32             m_FarFalloff;
    f32             m_VolumeLfe;
    f32             m_VolumeCenter;
    u8              m_RollOff;
    f32             m_NearDiffuse;    
    f32             m_FarDiffuse;
    s32             m_PlayPercent;
    f32             m_ReplayDelay;

    f32             m_Delta;

                    editor_element();
    void            Clear               ( void );
    void            OnEnumProp          ( prop_enum& List, const char* pParent, s32 Count, s32 Type = EDITOR_BLANK );
    xbool           OnProperty          ( prop_query& I, const char* pParent, s32 Count,  s32 Type = EDITOR_BLANK );
    void            OnCheckIntegrity    ( const char* pRscName, const char* pDescName, s32 Index );
};

//=========================================================================
// INTENSITY
//=========================================================================

class editor_intensity
{
public:

    s32                 m_Level;
    xarray< xstring >   m_pDescriptors;

                    editor_intensity();
    void            OnEnumProp          ( prop_enum& List, const char* pParent );
    xbool           OnProperty          ( prop_query& I, const char* pParent );
    void            OnCheckIntegrity    ( const char* pRscName );
};

//=========================================================================
// AUDIO PACKAGES
//=========================================================================
class editor_audio_package : public rsc_desc
{
public:

    CREATE_RTTI( editor_audio_package, rsc_desc, rsc_desc )


                            editor_audio_package     ( void );
                            ~editor_audio_package    ( void );

    virtual void            OnEnumProp               ( prop_enum& List            );
            void            OnEditorEnumProp         ( prop_enum& List            );

    virtual xbool           OnProperty               ( prop_query& I              );
            xbool           OnEditorProperty         ( prop_query& I              );

    virtual void            OnGetCompilerDependencies( xarray<xstring>& List      );
    virtual void            OnGetCompilerRules       ( xstring& CompilerRules     );
    virtual void            OnCheckIntegrity         ( void );
    virtual void            OnGetFinalDependencies   ( xarray<xstring>& List, platform Platform, const char* pDirectory );

    editor_audio_package&   operator =              ( const editor_audio_package& Package   );            
    
    xarray<editor_descriptor>  m_pDescriptorList;
    xarray<editor_intensity>   m_pIntensity;

    char            m_pVolumeFader[128];
    char            m_pPitchFader[128];
    char            m_pNearFalloffFader[128];
    char            m_pFarFalloffFader[128];
    char            m_pEffectSendFader[128];

    f32             m_Volume;
    f32             m_VolumeVar;
    f32             m_Pitch;
    f32             m_PitchVar;
    f32             m_Pan;
    s32             m_Priority;
    f32             m_EffectSend;
    f32             m_NearFalloff;
    f32             m_FarFalloff;
    s32             m_DescCount;
    f32             m_VolumeLfe;
    f32             m_VolumeCenter;
    u8              m_RollOff;
    f32             m_NearDiffuse;    
    f32             m_FarDiffuse;
    s32             m_PlayPercent;
    f32             m_ReplayDelay;

    char            m_pVirtualDirectory[128];
    char            m_pType[128];
    
    s32             m_ElementSelected;
    s32             m_DescriptorSelected;
    s32             m_Selected;
    s32             m_PrevSelected;
    s32             m_IntensitySelected;

    xbool           m_RebuildPackage;
    xbool           m_PackageLoaded;
};

#endif