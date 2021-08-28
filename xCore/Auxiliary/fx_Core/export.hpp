#ifndef __EXPORT_HPP
#define __EXPORT_HPP

namespace fx_core
{

class effect;

class export
{

public:
    //==============================================================================
    // Structures used for exporting (hacked from DMT's test stuff)
    //==============================================================================
    struct fx_datahdr
    {
        s32 TotalSize;      // In 32-bit values.  Does NOT include strings at end of file.
        u32 Flags;          
        s32 NSAValues;      // Staging area values.
        s32 NControllers;
        s32 NElements;
        s32 NBitmaps;
        s32 MasterCopy;     // unused
        s32 ReferenceCount; // Has no meaningful value in file.
        s32 pEffectName;    // Has no meaningful value in file.
        s32 pController;    // Has no meaningful value in file.
        s32 pElement;       // Has no meaningful value in file.
        s32 pBitmapD;       // Has no meaningful value in file.
        s32 pBitmapA;       // Has no meaningful value in file.

        // Tack on the following
        //  s32 ControllerAddr[ NControllers ];   // Has no meaningful value in file.
        //  s32 ElementAddr   [ NElements ];      // Has no meaningful value in file.
        /// s32 BitmapAddr    [ NBitmaps  ];      // Has no meaningful value in file.
    };

    struct fx_controllerhdr
    {
        s32         TotalSize;      // In 32-bit values.
        s32         TypeIndex;      // Has no meaningful value in file.
        s32         NOutputValues;  // How many output channels of data?
        s32         LeadIn;         // How to evaluate before DataBegin?
        s32         LeadOut;        // How to evaluate after  DataEnd?
        f32         DataBegin;      // within effect.
        f32         DataEnd;        // within effect.
        s32         OutputIndex;    // Where to write data in the staging area.
        // Custom arguments for the controller type.
        // s32         NKeys;
        // Tack on the following
        // f32         Key0Time;
        // f32         Key1Time;
        // f32         Key0Data;
        // f32         Key1Data;
        // ...
    }; 

    struct fx_elementhdr
    {
        s32         TotalSize;     // In 32-bit values.
        s32         TypeIndex;     // Has no meaningful value in file.
        xbool       ReadZ;
        s32         CombineMode;    // -1 = sub, 0 = multiplicative, 1 = additive
        s32         CtrlOffsets[13];// Where to get SRT/C values from Staging Area?
        f32         Scale[3];       // Constant values for S.
        f32         Rotate[3];      // Constant values for R.
        f32         Translate[3];   // Constant values for T.
        f32         Color[4];       // Constant values for C (as floats).
        f32         TimeStart;      // within effect.  Start.
        f32         TimeStop;       // within effect.  Stop.
        // Custom arguments for the element type.
        //s32         BitmapIndex;    
    };

    // bytestream
    xbytestream     m_ExportData;
    xbytestream     m_Additional;
    
public:

    void            ConstructData       ( effect*     pEffect,   s32 ExportTarget );
    xbool           SaveData            ( const char* pFilename, s32 ExportTarget );
};

const char* GetExportVersion( void );

} // namespace fx_core

#endif
