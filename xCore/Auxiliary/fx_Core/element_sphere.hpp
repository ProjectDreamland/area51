#ifndef __ELEMENT_SPHERE_HPP
#define __ELEMENT_SPHERE_HPP

#include "element.hpp"

namespace fx_core
{

//============================================================================
//  element_sphere
//============================================================================

class element_sphere : public element
{

protected:
    // Sphere element data
    xstring             m_BitmapName;
    s32                 m_CombineMode;
    xbool               m_MapPlanar;
    f32                 m_MappingTileU;
    f32                 m_MappingTileV;
    f32                 m_MappingScrollU;
    f32                 m_MappingScrollV;
    s32                 m_nSegmentsU;
    s32                 m_nSegmentsV;
    f32                 m_TopPos;
    f32                 m_BottomPos;
    xcolor              m_ColorTop;
    xcolor              m_ColorBottom;

public:
                        element_sphere      ( );
                       ~element_sphere      ( );

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
struct element_sphere_export
{
    xbool               m_MapPlanar;
    f32                 m_MappingTileU;
    f32                 m_MappingTileV;
    f32                 m_MappingScrollU;
    f32                 m_MappingScrollV;
    s32                 m_nSegmentsU;
    s32                 m_nSegmentsV;
    f32                 m_TopPos;
    f32                 m_BottomPos;
    xcolor              m_ColorTop;
    xcolor              m_ColorBottom;
};


//============================================================================

} // namespace fx_core

#endif
