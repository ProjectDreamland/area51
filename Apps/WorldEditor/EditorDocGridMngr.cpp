// EditorDoc.cpp : implementation of the CEditorDoc class
//

#include "StdAfx.h"

#include "EditorDocGridMngr.h"

#include "..\WinControls\FileSearch.h"

#include "aux_Bitmap.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=========================================================================
// CGridSettings
//=========================================================================

void CGridSettings::Reset() 
{ 
    m_nGridSnap = 100; 
    m_nSpeed = 25;
    m_nRotateSnap = 90; 
    m_nImageScale = 100;
    m_nImageAlpha = 96;
    m_fGridLocation = vector3(0.0f, 0.0f, 0.0f);
    m_fFarZLimit = 50000.0f;
    m_bImageDraw = FALSE;
    m_bShowStats = FALSE;
    m_xcBackground = xcolor(0,0,0);
    m_strSchematic.Empty();
    m_fTimeDilation = 1.0f;

    m_GridPresets[0] = 1;
    m_GridPresets[1] = 5;
    m_GridPresets[2] = 25;
    m_GridPresets[3] = 50;
    m_GridPresets[4] = 100;
    m_RotatePresets[0] = 1;
    m_RotatePresets[1] = 15;
    m_RotatePresets[2] = 30;
    m_RotatePresets[3] = 45;
    m_RotatePresets[4] = 90;

    m_nGridIncrement = 25;
    m_nRotateIncrement = 15;

    m_bShowBaselineGrid = FALSE;
}

//=========================================================================

void CGridSettings::LoadImage()
{
    if (m_bImageLoaded)
    {
        UnloadImage();
    }

    if (CFileSearch::DoesFileExist(m_strSchematic))
    {
        if( auxbmp_LoadNative( m_xbmpImage, m_strSchematic ) )
        {
            BOOL bBadImage = FALSE;
            x_try;
            s32 nWidth = m_xbmpImage.GetWidth();
            while( true )
            {
                if (((nWidth%2) != 0) && (nWidth != 1) )
                {
                    bBadImage = TRUE;
                    x_throw( "Image width and height must be a power of 2" );
                }
                else if( nWidth == 1 )
                    break;

                nWidth /= 2;
            }

            s32 nHeight = m_xbmpImage.GetHeight();
            while( true )
            {
                if (((nHeight%2) != 0) && (nHeight != 1) )
                {
                    bBadImage = TRUE;
                    x_throw( "Image width and height must be a power of 2" );
                }
                else if( nHeight == 1 )
                    break;

                nHeight /= 2;
            }

            m_bImageLoaded = TRUE;
            vram_Register( m_xbmpImage  );

            x_catch_display;

            if (bBadImage)
            {
                 m_xbmpImage.Kill();
            }
        }
    }
}

//=========================================================================

void CGridSettings::UnloadImage()
{
    if (m_bImageLoaded)
    {
        m_bImageLoaded = FALSE;
        vram_Unregister( m_xbmpImage    );
        m_xbmpImage.Kill();
    }
}

//=========================================================================

void CGridSettings::OnEnumProp ( prop_enum&    List )
{
    List.PropEnumString  (  "Grid", "Header which contains information about the current Grid settings", PROP_TYPE_HEADER );
    List.PropEnumInt     (  "Grid\\Speed", "Speed in cm. Applies to W,A,S,D,R,F movement.", 0 );
    List.PropEnumInt     (  "Grid\\Grid Snap", "Grid size, used for snapping to grid and raising and lowering grid.", 0 );
    List.PropEnumInt     (  "Grid\\Rotate Snap", "Snap for keyboard rotation of objects.", 0 );
    List.PropEnumVector3 (  "Grid\\Position", "Position of the grid.", 0 );
    List.PropEnumColor   (  "Grid\\Background", "Background color for the world editor.", 0 );
    List.PropEnumBool    (  "Grid\\ShowBaseline", "Show Overlay of baseline 100 cm grid", 0);
    List.PropEnumString  (  "Grid\\Schematic", "Schematic image to aid in laying out the levels.", PROP_TYPE_HEADER );
    List.PropEnumBool    (  "Grid\\Schematic\\Draw", "Draw the overlay schematic.", 0 );
    List.PropEnumFileName(  "Grid\\Schematic\\File", "Bitmaps (*.bmp)|*.bmp|Targas (*.tga)|*.tga|PNG (*.png)|*.png||", "File name for the schematic bitmap. Image Size must be a power of 2.", 0 );
    List.PropEnumInt     (  "Grid\\Schematic\\Scale", "Scale of the schematic image.", 0 );
    List.PropEnumInt     (  "Grid\\Schematic\\Alpha", "Transparency value for the schematic image.", 0 );
    List.PropEnumString  (  "Engine", "Header which contains information about the current engine settings", PROP_TYPE_HEADER );
    List.PropEnumBool    (  "Engine\\Show Stats", "Show engine stats in Debug window, when game is running.", 0 );
    List.PropEnumFloat   (  "Engine\\Far Z Clip", "Limit for the viewable distance.  Make this number smaller to improve framerate on large levels.", 0 );
    List.PropEnumFloat   (  "Engine\\Time Dilation", "Slow or Speed up time, 1.0 is normal, higher is faster time, lower is slower", 0 );
    List.PropEnumString  (  "Grid\\Presets", "Grid and Rotate Snap presets, used through number keys", PROP_TYPE_HEADER );
    List.PropEnumInt     (  "Grid\\Presets\\GridSnap1", "Grid Snap Preset for Numkey 1", 0 );
    List.PropEnumInt     (  "Grid\\Presets\\GridSnap2", "Grid Snap Preset for Numkey 2", 0 );
    List.PropEnumInt     (  "Grid\\Presets\\GridSnap3", "Grid Snap Preset for Numkey 3", 0 );
    List.PropEnumInt     (  "Grid\\Presets\\GridSnap4", "Grid Snap Preset for Numkey 4", 0 );
    List.PropEnumInt     (  "Grid\\Presets\\GridSnap5", "Grid Snap Preset for Numkey 5", 0 );
    List.PropEnumInt     (  "Grid\\Presets\\RotateSnap6", "Rotate Snap Preset for Numkey 6", 0 );
    List.PropEnumInt     (  "Grid\\Presets\\RotateSnap7", "Rotate Snap Preset for Numkey 7", 0 );
    List.PropEnumInt     (  "Grid\\Presets\\RotateSnap8", "Rotate Snap Preset for Numkey 8", 0 );
    List.PropEnumInt     (  "Grid\\Presets\\RotateSnap9", "Rotate Snap Preset for Numkey 9", 0 );
    List.PropEnumInt     (  "Grid\\Presets\\RotateSnap0", "Rotate Snap Preset for Numkey 0", 0 );
    List.PropEnumString  (  "Grid\\Increments", "Grid and Rotate Snap increments, used through number -,+ for rotate and /,* for grid", PROP_TYPE_HEADER );
    List.PropEnumInt     (  "Grid\\Increments\\GridAmount", "Amount to raise or lower grid snap when /,* are pressed (capped values)", 0 );
    List.PropEnumInt     (  "Grid\\Increments\\RotateAmount", "Amount to raise or lower rotate snap when -,+ are pressed (capped values)", 0 );
}

//=========================================================================

xbool CGridSettings::OnProperty ( prop_query&   I    )
{
    if( I.IsVar( "Grid\\Grid Snap" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nGridSnap );
        }
        else
        {
            m_nGridSnap = max( 1, I.GetVarInt() );
        }
    }
    else if( I.IsVar( "Grid\\Speed" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nSpeed );
        }
        else
        {
            m_nSpeed = max( 1, I.GetVarInt() );
        }
    }
    else if( I.IsVar( "Grid\\Rotate Snap" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nRotateSnap );
        }
        else
        {
            m_nRotateSnap = max( 1, I.GetVarInt() );
        }
    }
    else if( I.IsVar( "Grid\\Position" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarVector3( m_fGridLocation );
        }
        else
        {
            m_fGridLocation = I.GetVarVector3();
        }
    }
    else if( I.IsVar( "Grid\\Background" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarColor( m_xcBackground );
        }
        else
        {
            m_xcBackground = I.GetVarColor();
        }
    }
    else if( I.IsVar( "Grid\\Schematic\\File" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFileName( m_strSchematic, 256 );
        }
        else
        {
            m_strSchematic = CString(I.GetVarFileName());
            LoadImage();
        }
    }    
    else if( I.IsVar( "Grid\\Schematic\\Scale" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nImageScale );
        }
        else
        {
            m_nImageScale = I.GetVarInt();
        }
    }
    else if( I.IsVar( "Grid\\Schematic\\Alpha" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarInt( m_nImageAlpha );
        }
        else
        {
            m_nImageAlpha = I.GetVarInt();
        }
    }
    else if( I.VarBool( "Grid\\Schematic\\Draw", m_bImageDraw ) )
    {
        //do nothing
    }
    else if( I.VarBool( "Engine\\Show Stats", m_bShowStats ) )
    {
        //do nothing
    }
    else if( I.IsVar( "Engine\\Far Z Clip" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_fFarZLimit );
        }
        else
        {
            m_fFarZLimit = max( 1.0f, I.GetVarFloat() );
        }
    }
    else if ( I.IsVar( "Engine\\Time Dilation" ) ) 
    {
        if( I.IsRead() )
        {
            I.SetVarFloat( m_fTimeDilation );
        }
        else
        {
            m_fTimeDilation = I.GetVarFloat();
            if (m_fTimeDilation < 0) //keep it positive
            {
                m_fTimeDilation = 0.0f;
            }
        }
    }
    else if ( I.VarInt("Grid\\Presets\\GridSnap1", m_GridPresets[0], 1, 1000))
    {
        //do nothing
    }
    else if ( I.VarInt("Grid\\Presets\\GridSnap2", m_GridPresets[1], 1, 1000))
    {
        //do nothing
    }
    else if ( I.VarInt("Grid\\Presets\\GridSnap3", m_GridPresets[2], 1, 1000))
    {
        //do nothing
    }
    else if ( I.VarInt("Grid\\Presets\\GridSnap4", m_GridPresets[3], 1, 1000))
    {
        //do nothing
    }
    else if ( I.VarInt("Grid\\Presets\\GridSnap5", m_GridPresets[4], 1, 1000))
    {
        //do nothing
    }
    else if ( I.VarInt("Grid\\Presets\\RotateSnap6", m_RotatePresets[0], 1, 360))
    {
        //do nothing
    }
    else if ( I.VarInt("Grid\\Presets\\RotateSnap7", m_RotatePresets[1], 1, 360))
    {
        //do nothing
    }
    else if ( I.VarInt("Grid\\Presets\\RotateSnap8", m_RotatePresets[2], 1, 360))
    {
        //do nothing
    }
    else if ( I.VarInt("Grid\\Presets\\RotateSnap9", m_RotatePresets[3], 1, 360))
    {
        //do nothing
    }
    else if ( I.VarInt("Grid\\Presets\\RotateSnap0", m_RotatePresets[4], 1, 360))
    {
        //do nothing
    }
    else if ( I.VarInt("Grid\\Increments\\GridAmount", m_nGridIncrement, 1, 100))
    {
        //do nothing
    }
    else if ( I.VarInt("Grid\\Increments\\RotateAmount", m_nRotateIncrement, 1, 360))
    {
        //do nothing
    }
    else if ( I.VarBool("Grid\\ShowBaseline", m_bShowBaselineGrid))
    {
        //do nothing
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

