#ifndef __ELEMENT_SPRITE_HPP
#define __ELEMENT_SPRITE_HPP

#include "element.hpp"

namespace fx_core
{

//============================================================================
//  element_sprite
//============================================================================

class element_sprite : public element
{

protected:
    // Sprite element data
    xstring             m_BitmapName;
    s32                 m_CombineMode;
    f32                 m_ZBias;

public:
                        element_sprite      ( );
                       ~element_sprite      ( );

    void                Create              ( const char* pElementID, effect& Effect );
    virtual element*    Duplicate           ( void );

    virtual void        PostCopyPtrFix      ( void );

    virtual xbool       GetWorldBBoxAtTime  ( f32 T, bbox& BBox ) const; // Specialized BBox for Sprites

    virtual void        Render              ( f32 T );

    //virtual void        ShowProperties      ( s32 T, CProperties* pProps );
    //virtual xbool       OnPropertyChanged   ( s32 T, CString& Field, CString& Value );

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

} // namespace fx_core

#endif
