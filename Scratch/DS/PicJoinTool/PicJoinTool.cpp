//==============================================================================
//  PicJoinTool.cpp
//
//  A tool to join tga pictures (named eg. ps2shot000_4x4_000.tga) together
//  and write out a big tga (eg. ps2shot000.tga)
//
//  By Stephen Broumley July 9th 2002 (C) Inevitable Entertainment
//
//==============================================================================



//==============================================================================
// INCLUDES
//==============================================================================

#include "x_files.hpp"
#include "aux_Bitmap.hpp"
#include <stdio.h>
#include <io.h>
#include <process.h>




//==============================================================================
// STRUCTURES
//==============================================================================

// Sub pic structure
struct sub_pic
{
    char            Name[X_MAX_PATH] ;  // Filename
    s32             X,Y ;               // Position within big pic
    xbitmap         Bitmap ;            // Pixels
} ;

// Big picture structure
struct big_pic
{
    char            Name[X_MAX_PATH] ;  // Filename
    s32             XCount, YCount ;    // # of individual sub pics
    xarray<sub_pic> SubPics ;           // List of individual sub pics
    xbitmap         Bitmap ;            // Pixels
} ;


//==============================================================================
// FUNCTIONS
//==============================================================================

//==============================================================================
//
// Extract big pic + sub-pic info from full path
//
//==============================================================================
// IN:   A path in the form:  ps2shot000_4x4_000.tga
//==============================================================================
// OUT:  char*  - Big pic name. eg. p2shot000.tga
//       XCount - # of x sub pics. eg. 4
//       YCount - # of y sub pics. eg. 4
//       X      - X position of sub pic within big pic
//       Y      - Y position of sub pic within big pic
//==============================================================================
const char* GetPicInfo( const char* Path, s32& XCount, s32& YCount, s32& X, s32& Y )
{
    // Data (that can be returned)
    static char    Drive        [X_MAX_DRIVE];
    static char    Dir          [X_MAX_DIR];
    static char    FName        [X_MAX_FNAME];
    static char    Ext          [X_MAX_EXT];
    static char    BigPicName   [X_MAX_PATH];

    // Incase of fail
    XCount = YCount = X = Y = 0 ;

    // Get components of filename
    x_splitpath(Path, Drive, Dir, FName, Ext) ;
    
    // Find first underscore
    char* pInfo = x_strstr(FName, "_") ;
    if (!pInfo)
        return NULL ;

    // Search for info
    s32 Number = 0 ;
    if (sscanf(pInfo, "_%dx%d_%d", &XCount, &YCount, &Number) != 3)
        return NULL ;

    // Calculate position of pic within big pic
    X = Number % YCount ;
    Y = Number / XCount ;

    // Terminate filename at info so we can name the big pic
    *pInfo = 0 ;

    // Create big pic name
    x_makepath(BigPicName, Drive, Dir, FName, Ext) ;
    return BigPicName ;
}

//==============================================================================
// MAIN APP
//==============================================================================

void main( void )
{
    s32             i, j ;
    xarray<big_pic> BigPicList ;

#if 0
    // Swaps red and blue components in a broken 24bit pic
    xbitmap BMP ;
    if (auxbmp_Load(BMP, "C:/Projects/Delta/BIG_SCREEN_SHOTS/xboxshot_space1.tga"))
    {
        byte* pData = (byte*)BMP.GetPixelData() ;
        for (s32 i = 0 ; i < (BMP.GetWidth() * BMP.GetHeight()) ; i++)
        {
            byte R = pData[0] ;
            byte G = pData[1] ;
            byte B = pData[2] ;
            pData[0] = B ;
            pData[1] = G ;
            pData[2] = R ;
            pData += 3 ;
        }

        BMP.SaveTGA("C:/Projects/Delta/BIG_SCREEN_SHOTS/xboxshot_space1_fixed.tga") ;
    }
    exit(0) ;
#endif

    // Tool info
    x_printf("PicJoinTool.exe                                                               ") ;
    x_printf("                                                                              ") ;
    x_printf("A tool to join tga pictures (named eg. ps2shot000_4x4_000.tga) together       ") ;
    x_printf("and write out a big tga (eg. ps2shot000.tga)                                  ") ;
    x_printf("                                                                              ") ;
    x_printf("By Stephen Broumley July 9th 2002 (C) Inevitable Entertainment                ") ;
    x_printf("                                                                              ") ;
    x_printf("Usage:  PicJoinTool.exe   (no params are needed - all tga will be auto found) ") ;
    x_printf("                                                                              ") ;

    //==========================================================================
    // Build pic list
    //==========================================================================

    // Pre-allocate
    BigPicList.SetCapacity(100) ;

	_finddata_t FileData;
	long        FindData ;

	// Find all tgas
	FindData = _findfirst("*.tga", &FileData) ;
	if(FindData != -1)
    {
		do
        {
			// Add the filename to the list.
			if (FileData.name)
            {
                // Get info about the big pic this could be from
                s32 XCount, YCount, X,Y ;
                const char* BigPicName = GetPicInfo(FileData.name, XCount, YCount, X, Y) ;

                // Found a pic that's part of a big pic?
                if (BigPicName)
                {
                    // See if big pic already exists
                    big_pic* pBigPic = NULL ;
                    for (i = 0 ; i < BigPicList.GetCount() ; i++)
                    {
                        // Matching names?
                        if (x_strcmp(BigPicList[i].Name, BigPicName) == 0)
                        {
                            pBigPic = &BigPicList[i] ;
                            break ;
                        }
                    }

                    // Create a new big pic, if it's not been created yet
                    if (pBigPic == NULL)
                    {
                        pBigPic = &BigPicList.Append() ;
                        x_strcpy(pBigPic->Name, BigPicName) ;
                        pBigPic->SubPics.SetCapacity(XCount * YCount) ;
                        pBigPic->XCount = XCount ;
                        pBigPic->YCount = YCount ;
                    }

                    // Create a new pic within the big pic
                    sub_pic& SubPic = pBigPic->SubPics.Append() ;
                    x_strcpy(SubPic.Name, FileData.name) ;
                    SubPic.X = X ;
                    SubPic.Y = Y ;
                }
            }

		} while(_findnext(FindData, &FileData) == 0) ;
	}

    //==========================================================================
    // Build and save out big pics
    //==========================================================================

    // Loop over all big pics
    for (i = 0 ; i < BigPicList.GetCount() ; i++)
    {
        // Get big pic
        big_pic& BigPic = BigPicList[i] ;
        x_printf("Creating BigPic: %s\n", BigPic.Name) ;

        // Reset format
        xbitmap::format     Format = xbitmap::FMT_NULL ;
        s32                 XRes, YRes, BBP ;
        xbool               Fail = FALSE ;

        // Loop over all sub pics
	    for (j = 0 ; j < BigPic.SubPics.GetCount() ; j++)
        {
            sub_pic& SubPic = BigPic.SubPics[j] ;
            x_printf("    Loading SubPic: %s\n", SubPic.Name) ;

            // Load bitmap
            Fail |= !auxbmp_Load(SubPic.Bitmap, SubPic.Name) ;

            // Convert to 24bit here so that SaveTGA doesn't have to convert the huge mother!
            SubPic.Bitmap.ConvertFormat(xbitmap::FMT_24_BGR_888) ;

            // Keep format ready for big bitmap
            Format = SubPic.Bitmap.GetFormat() ;
            XRes   = SubPic.Bitmap.GetWidth() ;
            YRes   = SubPic.Bitmap.GetHeight() ;
            BBP    = SubPic.Bitmap.GetBPP() ;
        }

        // Now create the big pic bitmap
        if ((!Fail) && (Format != xbitmap::FMT_NULL))
        {
            // Calculate size of big pic
            s32     W     = BigPic.XCount * XRes ;
            s32     H     = BigPic.YCount * YRes ;

            // Allocate pixel data and setup bitmap
            byte* pData = (byte*)x_malloc(W * H * 3) ;
            if (!pData)
            {
                x_printf("ERROR! Ran out of memory allocating %s\n", BigPic.Name) ;
                break ;
            }
            BigPic.Bitmap.Setup(Format, W, H, TRUE, pData) ;
                                
    	    // Now paste all sub pics into big pic
            for (j = 0 ; j < BigPic.SubPics.GetCount() ; j++)
            {
                sub_pic& SubPic = BigPic.SubPics[j] ;

                // Splatter sub pic into the big pic
                BigPic.Bitmap.Blit(SubPic.X*XRes, SubPic.Y*YRes, 
                                   0,0, XRes, YRes, 
                                   SubPic.Bitmap) ;

                // Sub pic is no longer needed
                SubPic.Bitmap.Kill() ;
            }

            // Now save out the big pic
            x_printf("Writing: %s\n", BigPic.Name) ;
            if (BigPic.Bitmap.SaveTGA(BigPic.Name))
            {
                // Success so delete all the little pic files
                for (j = 0 ; j < BigPic.SubPics.GetCount() ; j++)
                {
                    sub_pic& SubPic = BigPic.SubPics[j] ;

                    // Delete the file
                    system(xfs("del %s", SubPic.Name)) ;
                }
            }

            // Big pic is no longer needed
            BigPic.Bitmap.Kill() ;
        }

        // Next big pic
        x_printf("\n\n") ;
    }

    // Cleanup
    BigPicList.Clear() ;

    // Shutdown
    x_Kill();
}

//==============================================================================
