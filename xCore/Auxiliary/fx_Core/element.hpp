#ifndef __ELEMENT_HPP
#define __ELEMENT_HPP

//============================================================================
//  INCLUDES
//============================================================================

#include "base.hpp"
#include "export.hpp"
#include "TextureMgr.hpp"

namespace fx_core
{

//============================================================================
// Element Registry
class   element;
typedef element* (*elem_factory_fn)( void );

struct elem_reg
{
    const char*         m_pTypeName;
    elem_factory_fn     m_pFactoryFn;   


                        elem_reg( const char*       pTypeName,
                                  elem_factory_fn   FactoryFn );
};

//============================================================================
// Make an element using the factory
element* MakeElement( const char* pType );


//============================================================================
//  element
//============================================================================
class element : public base
{
    friend class effect;

protected:
    xbool               m_IsSelected;                   // Element is currently selected
    xbool               m_ShowTrajectory;               // Should we render the translation path of the element?

    xstring             m_ID;                           // Element ID
    xstring             m_CustomType;                   // Custom type for element
    effect*             m_pEffect;                      // Pointer back to effect this element belongs to

    xbool               m_Export;                       // should allow export?
    xbool               m_Hide;                         // temporarily hide
    
public:

    // Life parameters
    xbool               m_IsImmortal;
    s32                 m_LifeStartFrame;
    s32                 m_LifeStopFrame;

    // Material parameters
    xbool               m_ZRead;

    // Keyframeable base parameters
    ctrl_key*           m_pScale;
    ctrl_key*           m_pRotation;                     
    ctrl_key*           m_pTranslation;
    ctrl_key*           m_pColor;
    ctrl_key*           m_pAlpha;

                        element             ( );
    virtual            ~element             ( );

    void                Create              ( const char* pElementID, effect& Effect );
    virtual element*    Duplicate           ( void ) = 0;

    virtual void        MakeDuplicateName   ( const char* pOriginalName );

    virtual void        PostCopyPtrFix      ( void ) = 0;

    void                AddKey_SRT          ( s32 T, const vector3& Scale, 
                                                     const radian3& Rotation, 
                                                     const vector3& Translation );

    void                AddKey_Pos          ( s32 T, const vector3& Pos );
    void                AddKey_Rotation     ( s32 T, const radian3& Rotation );
    void                AddKey_Scale        ( s32 T, const vector3& Scale );

    void                AddKey_Color        ( s32 T, const xcolor& Color );

    void                Translate           (        const vector3& Delta );
    void                Rotate              (        const radian3& Delta );
    void                Scale               (        const vector3& Delta );

    void                PrepareToChangeColor( void );   // clears flag on controllers
    void                ChangeColorKeys     ( s32 DeltaR, s32 DeltaG, s32 DeltaB, s32 DeltaA );

    virtual xbool       ExistsAtTime        ( f32 T ) const;
    virtual xbool       GetLocalBBoxAtTime  ( f32 T, bbox&    BBox     ) const;
    virtual xbool       GetWorldBBoxAtTime  ( f32 T, bbox&    BBox     ) const;

    xbool               GetScaleAtTime      ( f32 T, vector3& Scale    ) const;
    xbool               GetRotationAtTime   ( f32 T, vector3& Rot      ) const;
    xbool               GetPositionAtTime   ( f32 T, vector3& Position ) const;
    virtual xbool       GetL2WAtTime        ( f32 T, matrix4& L2W      ) const;
    virtual xbool       GetColorAtTime      ( f32 T, xcolor&  Color    ) const;

    s32                 GetBookends         ( s32& FirstKey, s32& LastKey );
    void                GetElementID        ( char* pElementID )        { strcpy( pElementID, m_ID );       }
    void                SetElementID        ( const char* pElementID )  { m_ID.Format("%s", pElementID);    }

    void                SetSelected         ( xbool IsSelected );
    xbool               IsSelected          ( void ) const;

    void                RenderBBox          ( f32 T ) const;

    void                RenderTrajectory    ( void ) const;
    xbool               GetShowTrajectory   ( void ) const              { return m_ShowTrajectory; }
    void                SetShowTrajectory   ( xbool ShowTrajectory )    { m_ShowTrajectory = ShowTrajectory; }

    xbool               ShouldExport        ( void )                    { return m_Export;     }
    void                SetExportFlag       ( bool ExportIt )           { m_Export = ExportIt; }
    virtual void        FlagExportTextures  ( void ) = 0;
    virtual void        ActivateTextures    ( void ) = 0;

    virtual void        Save                ( igfmgr& Igf );
    virtual void        Load                ( igfmgr& Igf );
    virtual void        ExportData          ( export::fx_elementhdr& ElemHdr, 
                                              xstring& Type,
                                              xbytestream& Stream, 
                                              s32 ExportTarget );

    // show/hide operations
    void                Show                ( void )                    { m_Hide = FALSE; }
    void                Hide                ( void )                    { m_Hide = TRUE;  }
    xbool               IsHidden            ( void )                    { return m_Hide;  }

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


// utility to function to compensate for lack of x_sscanf
s32 String2V3           ( const char* pStr, vector3& V3   );    // return the number of values converted
s32 String2Color        ( const char* pStr, xcolor& Color );    // expects 4 integer values

} // namespace fx_core

#endif
