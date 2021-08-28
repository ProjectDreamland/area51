#ifndef __PACKAGETYPES_HPP
#define __PACKAGETYPES_HPP

#include "x_files.hpp"
#include "x_bytestream.hpp"
#include "aiff_file.hpp"
#include "audio_private_pkg.hpp"

enum export_targets
{
    EXPORT_PC,
    EXPORT_PS2,
    EXPORT_GCN,
    EXPORT_XBOX,
    EXPORT_NUM_TARGETS
};

enum {
    NONE=0,
    PACKAGE,
    FILES,
    DESCRIPTORS,
    MUSIC,
    OUTPUT,
    DONE,
};

enum {
    MUSIC_TYPE = 0,
    MUSIC_INTENSITY,
    NUM_MUSIC_TYPES,
};

//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

struct multi_channel_data
{
    s32                     nChannels;
    s32                     nWaveforms;
    s32                     CompressionMethod;
    s32                     CompressedSize;
    s32                     LipSyncSize;
    s32                     BreakPointSize;
    s32                     HeaderSize;
    xarray<xbytestream>     WaveformData;
    xarray<xbytestream>     LipSyncData;
    xbytestream             BreakPointData;
    xarray<void*>           Headers;
    xbytestream             FileHeader;
};                          

struct file_info
{
    xstring                 Identifier;
    u32                     Temperature;
    s32                     LipSyncOffset;
    s32                     BreakPointOffset;
    xbool                   UsesLipSync;
    u32                     Index;
    s32                     NumChannels[EXPORT_NUM_TARGETS];
    xstring                 Filename;
    s32                     CompressedSize[EXPORT_NUM_TARGETS];
    xstring                 CompressedFilename[EXPORT_NUM_TARGETS];
    xstring                 CompressedStereoFilenames[EXPORT_NUM_TARGETS][2];
    xstring                 CompressedInterleavedFilename[EXPORT_NUM_TARGETS];
    xarray<aiff_file::breakpoint> BreakPoints;
};

struct element_info
{
    xstring                 Identifier;
    u32                     Type;
    u32                     Index;
    compressed_parameters   Params;
    xbool                   IsExported;
    union 
    {
        u32                 Weight;
        f32                 StartDelay;
    };
};

struct descriptor_info
{
    xstring                 Identifier;
    u32                     Type;
    u32                     Index;
    xbool                   IdentifierProcessed;
    compressed_parameters   Params;
    xarray<element_info>    Elements;
};

struct music_intensity
{
    u8                      Level;
    char                    Descriptor[31];
};

struct package_info
{
    xstring                 m_Identifier;
    xstring                 m_OutputFilename;
    s32                     m_ParseSection;
    xbool                   m_ParseError;
    compressed_parameters   m_Params;
    s32                     m_AudioRamFootprint;
    s32                     m_MainRamFootprint;
    xbytestream             m_HeaderStream;
    xarray<u32>             m_DescriptorOffsets;
    xbytestream             m_DescriptorStream;
    s32                     m_TemperatureCount[NUM_TEMPERATURES];
    xarray<u32>             m_SampleOffsets[NUM_TEMPERATURES];    
    xbytestream             m_SampleStream[NUM_TEMPERATURES];
    xbytestream             m_StringTableStream;
    xbytestream             m_LipSyncTableStream;
    xbytestream             m_BreakPointTableStream;
    xbytestream             m_DescriptorIdentifierStream;
    xbytestream             m_MusicStream;
    xarray<descriptor_info> m_Descriptors;
    xarray<file_info>       m_Files;
    char                    m_MusicType[32];
    s32                     m_MusicFootprint; 
    xarray<music_intensity> m_Intensity;
};

#endif
