#ifndef BITMAP_VIEWER_HPP
#define BITMAP_VIEWER_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "x_files.hpp"

//=========================================================================
// CLASS
//=========================================================================

class bitmap_viewer
{
public:

    void    AppendPicture   ( const char* pFileName );
    void    Render          ( void );
    void    Clear           ( void );

protected:

    struct bitmap_info
    {
        xbitmap  Bitmap;
        char    FileName[256];
    };

    xarray<xbitmap>     m_Bitmap;
    s32                 m_iCurrent;     // Which bitmap we are looking at 
};

//=========================================================================
// END
//=========================================================================
#endif