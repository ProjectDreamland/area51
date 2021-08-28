#ifndef __ELEMENT_MESH_HPP
#define __ELEMENT_MESH_HPP

#include "element.hpp"
#include "MeshViewer.hpp"

namespace fx_core
{

//============================================================================
//  element_mesh
//============================================================================

class element_mesh : public element
{

protected:
    xstring             m_MeshName;
    mesh_viewer         m_MeshViewer;

public:
                        element_mesh        ( void );
                        element_mesh        ( const element_mesh& mesh );
                       ~element_mesh        ( void );

    void                Create              ( const char* pElementID, effect& Effect );
    virtual element*    Duplicate           ( void );

    virtual void        PostCopyPtrFix      ( void );

    virtual void        Render              ( f32 T );

    //virtual void        ShowProperties      ( s32 T, CProperties* pProps );
    //virtual xbool       OnPropertyChanged   ( s32 T, CString& Field, CString& Value );

    virtual void        FlagExportTextures  ( void ) { }
    virtual void        ActivateTextures    ( void ) { }

    virtual void        Save                ( igfmgr& Igf );
    virtual void        Load                ( igfmgr& Igf );

    virtual void        LoadMesh            ( void );

    virtual void        ExportData          ( export::fx_elementhdr& ElemHdr, 
                                              xstring& Type,
                                              xbytestream& Stream, 
                                              s32 ExportTarget );

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

} // namespace fx_core

#endif
