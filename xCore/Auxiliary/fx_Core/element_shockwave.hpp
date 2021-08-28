#ifndef __ELEMENT_SHOCKWAVE_HPP
#define __ELEMENT_SHOCKWAVE_HPP

#include "element.hpp"

namespace fx_core
{

//============================================================================
//  element_shockwave
//============================================================================

class element_shockwave : public element
{

protected:
    // Shockwave element data
    xstring             m_BitmapName;
    s32                 m_CombineMode;
    xbool               m_MapPlanar;
    f32                 m_MappingTileU;
    f32                 m_MappingTileV;
    f32                 m_MappingScrollU;
    f32                 m_MappingScrollV;
    f32                 m_MappingCenterV;
    s32                 m_nSegments;
    f32                 m_Width;
    f32                 m_Center;
    //f32                 m_NoiseStrengthIn;
    //f32                 m_NoiseStrengthOut;
    //f32                 m_NoiseFrequencyIn;
    //f32                 m_NoiseFrequencyOut;
    //f32                 m_NoiseSpeedIn;
    //f32                 m_NoiseSpeedOut;
    xbool               m_IsFlat;
    xcolor              m_InnerColor;
    xcolor              m_OuterColor;
    xbool               m_FancyScrolling;
    
    

public:
                        element_shockwave   ( );
                       ~element_shockwave   ( );

    void                Create              ( const char* pElementID, effect& Effect );
    virtual element*    Duplicate           ( void );

    virtual void        PostCopyPtrFix      ( void );

    virtual void        Render              ( f32 T );

    virtual void        Save                ( igfmgr& Igf );
    virtual void        Load                ( igfmgr& Igf );

    virtual void        ExportData          ( export::fx_elementhdr& ElemHdr, 
                                              xstring& Type,
                                              xbytestream& Stream, 
                                              s32 ExportTarget );

    virtual void        FlagExportTextures  ( void );
    virtual void        ActivateTextures    ( void );

    virtual xbool       OnPropertyChanged   ( s32 T, xstring& Field, xstring& Value );


    //========================================================================
    // Function for enumerating element properties
    //========================================================================
    virtual xbool       GetProperty         ( s32               Idx,
                                              s32               T,
                                              xcolor&           UIColor,
                                              xstring&          Name,
                                              xstring&          Value,
                                              xbool&            IsDisabled,
                                              base::prop_type&  Type );

};

//============================================================================
// Export data
struct element_shockwave_export
{
    xbool               m_MapPlanar;
    f32                 m_MappingTileU;
    f32                 m_MappingTileV;
    f32                 m_MappingScrollU;
    f32                 m_MappingScrollV;
    f32                 m_MappingCenterV;
    s32                 m_nSegments;
    f32                 m_Width;
    f32                 m_Center;
    xbool               m_IsFlat;
    xcolor              m_InnerColor;
    xcolor              m_OuterColor;
};


//============================================================================

} // namespace fx_core

#endif
