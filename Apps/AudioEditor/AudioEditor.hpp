#ifndef AUDIO_EDITOR_HPP
#define AUDIO_EDITOR_HPP

//=========================================================================
// AUDIO EDITOR
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "../EDRscDesc\RSCDesc.hpp"
#include "AudioEditorPackage.hpp"

#define FOLDER_SELECTED      (1<<1)
#define PACKAGE_SELECTED     (1<<2)
#define DESCRIPTOR_SELECTED  (1<<3)
#define ELEMENT_SELECTED     (1<<4)
#define INTENSITY_SELECTED   (1<<5)

//=========================================================================
//  EXTERNALS
//=========================================================================
extern f32  g_DescVolume;
extern f32  g_DescVolumeVar;
extern f32  g_DescPitch;
extern f32  g_DescPitchVar;
extern f32  g_DescPan;
extern s32  g_DescPriority;
extern f32  g_DescEffectSend;
extern f32  g_DescNearFalloff;
extern f32  g_DescFarFalloff;
extern f32  g_DescNearDiffuse;
extern f32  g_DescFarDiffuse;
extern u8   g_DescRollOff;

extern f32  g_ElementVolume;
extern f32  g_ElementVolumeVar;
extern f32  g_ElementPitch;
extern f32  g_ElementPitchVar;
extern f32  g_ElementPan;
extern s32  g_ElementPriority;
extern f32  g_ElementEffectSend;
extern f32  g_ElementNearFalloff;
extern f32  g_ElementFarFalloff;
extern f32  g_ElementNearDiffuse;
extern f32  g_ElementFarDiffuse;
extern s32  g_ElementTemperature;
extern u8   g_ElementRollOff;

typedef void update_label( void* pObject, const char* pLabel );

//=========================================================================
struct multi_sel
{
    s32     m_SelectedType;
    xhandle m_PackageHandle;
    s32     m_DescriptorIndex;
    s32     m_ElementIndex;

    multi_sel (){ m_SelectedType = 0; m_PackageHandle = -1; 
                m_DescriptorIndex = -1; m_ElementIndex = -1; }
    
    ~multi_sel (){ }            
};

//=========================================================================
// AUDIO EDITOR
//=========================================================================

class audio_editor : public prop_interface
{
//=========================================================================
public:
    enum multisel_dirty
    {
        VOLUME_DIRTY,
        VOLUME_VAR_DIRTY,
        PITCH_DIRTY,
        PITCH_VAR_DIRTY,
        PAN_DIRTY,
        PRIORITY_DIRTY,
        EFFECTSEND_DIRTY,
        NEARFALLOFF_DIRTY,
        FARFALLOFF_DIRTY,
        ROLLOFF_DIRTY,
        NEARDIFFUSE_DIRTY,
        FARDIFFUSE_DIRTY,
        TEMPERATURE_DIRTY
    };

                    audio_editor                    ( void                     );
                    ~audio_editor                   ( void                     );

    virtual void    OnEnumProp                      ( prop_enum& List          );
    virtual xbool   OnProperty                      ( prop_query& I            );

            void    MultiSelectEnumProp             ( prop_enum& List          );
            xbool   MultiSelectProperty             ( prop_query& I            );
    inline  void    SetMultiSelect                  ( xbool Enable             );
    inline  xbool   GetMultiSelect                  ( void                     );
            void    InsertMultiSelIndex             ( multi_sel& SelectionIndex);
            void    ClearMultiSelIndex              ( void                     );
    
            void    SetDefaultParamsMode            ( xbool Mode               );
            void    DefaultParamsEnumProp           ( prop_enum& List          );
            xbool   DefaultParamsProperty           ( prop_query& I            );

            void    SetDefaultElementParamsMode     ( xbool Mode               );
            void    DefaultElementParamsEnumProp    ( prop_enum& List          );
            xbool   DefaultElementParamsProperty    ( prop_query& I            );

            void    ClearParamsMode                 ( void );

            void    Save                            ( void                     );
            void    Load                            ( const char* pFileName    );

            void    NewPackage                      ( void                     );
            s32     NewDescriptor                   ( void                     );
            s32     NewElement                      ( s32 DescriptorIndex      );
            s32     NewIntensity                    ( void                     );

            xbool   RefrenceDescriptor              ( s32 DescIndex, s32 ElementIndex, s32 RefDescIndex );
            
            xbool   DeleteDescriptor                ( s32 DescIndex            );
            void    DeleteIntensity                 ( s32 IntensityIndex       );
            void    DeleteIntensityDesc             ( s32 IntensityIndex, s32 DescIndex );
            void    Delete                          ( xhandle PackageIndex     );
            void    DeleteAll                       ( void                     );
            void    DeleteElement                   ( s32 DescIndex, s32 ElementIndex );
            
            xbool   NeedSave                        ( void                     );
            void    BeginEdit                       ( editor_audio_package& Audio     );
            void    EndEdit                         ( void                     );      
            
            void    CompileAll                      ( void                     );
            void    CompilePackage                  ( xhandle PackageIndex     );
            void    ReBuildAll                      ( void                     );
            void    ReBuildPackage                  ( xhandle PackageIndex     );
                        
            void    ApplyChangesToProperty          ( multisel_dirty DirtyFlag );

    inline  void    SetUpdateLabelFn                ( void* pCallObject, update_label* fnpUpdate );

//=========================================================================
public:
    xharray<editor_audio_package>   m_pDesc;
    xarray<multi_sel>               m_MultipleSelection;
    xhandle                         m_PackageSelected;
    xhandle                         m_AddedPackage;

    xbool                           m_MultiSelect;
    xbool                           m_DefaultParamsMode;
    xbool                           m_DefaultElementParamsMode;

    f32                             m_MultiSelectionVolume;
    f32                             m_MultiSelectionVolumeVar;
    f32                             m_MultiSelectionPitch;
    f32                             m_MultiSelectionPitchVar;
    f32                             m_MultiSelectionPan;
    s32                             m_MultiSelectionPriority;
    f32                             m_MultiSelectionEffectSend;
    f32                             m_MultiSelectionNearFalloff;
    f32                             m_MultiSelectionFarFalloff;
    f32                             m_MultiSelectionNearDiffuse;
    f32                             m_MultiSelectionFarDiffuse;
    u8                              m_MultiSelectionRollOff;
    s32                             m_MultiSelectionTemperature;

    update_label*                   m_fnpUpdateLabel;
    void*                           m_pCallbackObject;
};

//=========================================================================

inline
void audio_editor::SetMultiSelect( xbool Enable )
{
    m_MultiSelect = Enable;
}

//=========================================================================

inline
xbool audio_editor::GetMultiSelect( void )
{
    return m_MultiSelect;
}

//=========================================================================

inline  
void audio_editor::SetUpdateLabelFn( void* pCallObject, update_label* fnpUpdate )
{
    m_fnpUpdateLabel    = fnpUpdate;
    m_pCallbackObject   = pCallObject;
}

//=========================================================================

inline
void audio_editor::SetDefaultParamsMode ( xbool Mode )
{
    m_DefaultParamsMode = Mode;
}

//=========================================================================

inline
void audio_editor::SetDefaultElementParamsMode ( xbool Mode )
{
    m_DefaultElementParamsMode = Mode;
}

//=========================================================================

inline
void audio_editor::ClearParamsMode ( void )
{
    m_DefaultParamsMode         = FALSE;
    m_DefaultElementParamsMode  = FALSE;
}

//=========================================================================
#endif
