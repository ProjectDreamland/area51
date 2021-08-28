// EditorDoc.h : interface of the CEditorDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(_EDTIOR_GRID_MNGR_)
#define _EDTIOR_GRID_MNGR_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "WorldEditor.hpp"
#include "..\Editor\BaseDocument.h"
#include "..\MiscUtils\SimpleUtils.hpp"

//=========================================================================
//=========================================================================

class CGridSettings : public prop_interface
{
public:
    CGridSettings() { m_bImageLoaded = FALSE; Reset(); }
    ~CGridSettings() { UnloadImage(); }
    virtual void        OnEnumProp                      ( prop_enum&    List );
    virtual xbool       OnProperty                      ( prop_query&   I    );

    void Reset();

    s32 m_nGridSnap;
    s32 m_nRotateSnap;
    s32 m_nImageScale;
    s32 m_nImageAlpha;
    s32 m_nSpeed;
    vector3 m_fGridLocation;
    f32 m_fFarZLimit;
    f32 m_fTimeDilation;
    xcolor m_xcBackground;
    BOOL m_bImageDraw;
    BOOL m_bShowStats;
    CString m_strSchematic;

    s32 m_GridPresets[5];
    s32 m_RotatePresets[5];
    s32 m_nGridIncrement;
    s32 m_nRotateIncrement;
    xbool m_bShowBaselineGrid;

    //image stuff
    void LoadImage();
    void UnloadImage();

    xbool m_bImageLoaded;
    xbitmap m_xbmpImage;
};

#endif
