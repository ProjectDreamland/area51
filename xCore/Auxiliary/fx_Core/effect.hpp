#ifndef __EFFECT_H
#define __EFFECT_H

//============================================================================
//  INCLUDES
//============================================================================

#include "base.hpp"
#include "controller.hpp"
#include "element.hpp"

//#include "properties.h"

namespace fx_core
{

//============================================================================
// effect
//============================================================================

class effect : public base
{

protected:
    xarray<controller*>             m_Controllers;          // the controllers
    xarray<element*>                m_Elements;             // the elements
    xarray<element*>                m_SelSet;               // the elements

public:
    
    xstring                         m_BackgroundName;       // Name of background
    xstring                         m_ExportPC;             // Path to export for PC
    xstring                         m_ExportGCN;            // Path to export for GCN
    xstring                         m_ExportPS2;            // Path to export for PS2
    xstring                         m_ExportXBOX;           // Path to export for XBOX
    xbool                           m_Instanceable;         // Effect can be re-rendered mutliple times in the game world

    xbool                           m_RenderBBoxesEnabled;
    xbool                           m_AnimMode;
    xbool                           m_IgnoreMeshes;

public:
                        effect                  ( void );
                        effect                  ( effect&           Effect );
                       ~effect                  ( void );                 
    
    void                Destroy                 ( void );

    void                SelClear                ( void )                { m_SelSet.SetCount(0);     }
    void                Select                  ( element* pElem )      { m_SelSet.Append( pElem ); }

    s32                 GetNumElements          ( void );
    element*            GetElement              ( s32      iElement );
    void                AddElement              ( element* pElement );
    void                RemoveElement           ( element* pElem );
    void                MoveElement             ( s32 iElement, s32 iElementNew );
    element*            FindElementByName       ( const char* pElemName );
    void                SetAllElementsSelected  ( xbool State );

    s32                 GetNumControllers       ( void );
    s32                 GetOptimalNumControllers( void );
    s32                 GetTotalNumControllers  ( void );
    controller*         GetController           ( s32               iController );
    void                AddController           ( controller*       pController );

    void                RenderBackground        ( f32 T );
    void                Render                  ( f32 T );

    virtual void        Save                    ( igfmgr&   Igf );
    virtual void        Load                    ( igfmgr&   Igf );
                                                
    void                EnableRenderBBoxes      ( xbool IsEnabled )     { m_RenderBBoxesEnabled = IsEnabled; }
    xbool               RenderBBoxesEnabled     ( void )                { return m_RenderBBoxesEnabled; }
    void                ActivateAllTextures     ( void );
    void                FlagAllTextures         ( void );

    void                SetAnimMode             ( xbool IsAnimMode )    { m_AnimMode = IsAnimMode; }
    xbool               IsAnimateModeOn         ( void )                { return m_AnimMode;       }

    void                GetBookends             ( s32& FirstKey, s32& LastKey );
    s32                 GetLifeSpan             ( void );
    
    //========================================================================
    // Function for enumerating effect properties
    //========================================================================
    xbool               GetProperty             ( s32 Idx, s32 T, xstring& Name, xstring& Value, xbool& IsDisabled, element::prop_type& Type );
    xbool               OnPropertyChanged       ( s32 T, xstring& Field, xstring& Value );

};

//============================================================================

} // namespace fx_core

#endif
