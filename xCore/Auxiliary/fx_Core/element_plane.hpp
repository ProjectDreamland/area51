#ifndef __ELEMENT_PLANE_HPP
#define __ELEMENT_PLANE_HPP

#include "element.hpp"

namespace fx_core
{

//============================================================================
//  element_plane
//============================================================================

class element_plane : public element
{

protected:
    // Plane element data
    xstring             m_BitmapName;
    s32                 m_CombineMode;
    f32                 m_MappingTileU;
    f32                 m_MappingTileV;
    f32                 m_MappingScrollU;
    f32                 m_MappingScrollV;

public:
                        element_plane       ( );
                       ~element_plane       ( );

    void                Create              ( const char* pElementID, effect& Effect );
    virtual element*    Duplicate           ( void );

    virtual void        PostCopyPtrFix      ( void );

    virtual xbool       GetLocalBBoxAtTime  ( f32 T, bbox& BBox ) const;

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
// Export structure
struct element_plane_export
{
    f32                 m_MappingTileU;
    f32                 m_MappingTileV;
    f32                 m_MappingScrollU;
    f32                 m_MappingScrollV;
};

//============================================================================

} // namespace fx_core

#endif
