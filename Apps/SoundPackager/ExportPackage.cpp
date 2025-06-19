#include <stdlib.h>
#include "x_files.hpp"
#include "ExportPackage.hpp"
#include "PackageTypes.hpp"
#include "SoundPackager.hpp"
#include "Endian.hpp"
#include "Compress.hpp"
#include "windows.h"
#include "x_log.hpp"

s32 s_ParameterSizes[NUM_PARAMETERS] = 
{
    2, // PITCH           16-bit represents [0.0..4.0]
    2, // PITCH_VARIANCE  16-bit represents +/- [0.0..1.0]
    2, // VOLUME,         16-bit represents [0.0...1.0]
    2, // VOLUME_VARIANCE 16-bit represents +/- [0.0..1.0]
    2, // VOLUME_CENTER   16-bit represents [0.0...1.0]
    2, // VOLUME_LFE      16-bit represents [0.0...1.0]
    2, // VOLUME_DUCK     16-bit represents [0.0...1.0]
    2, // USER_FLAGS      16-bit represents whatever...
    2, // REPLAY_DELAY    16-bit represnets [0.0..6553.5]
    2, // LAST_PLAY       16-bit represnets [0.0..6553.5]
    1, // PAN              8-bit represents [-1.0..1.0]
    1, // PRIORITY         8-bit represents 256 priorities.
    1, // EFFECT_SEND      8-bit represents [0.0..1.0]
    1, // NEAR_FALLOFF     8-bit represents [0%..1000%]
    1, // FAR_FALLOFF      8-bit represents [0%..1000%]
    1, // ROLLOFF_METHOD   8-bit represnet 256 rolloff methods.
    1, // NEAR_DIFFUSE     8-bit represents [0%..1000%]
    1, // FAR_DIFFUSE      8-bit represents [0%..1000%]
    1, // PLAY_PERCENT     8-bit represents [0..100]
};    

s32 s_CompressionHeaderSizes[EXPORT_NUM_TARGETS][NUM_COMPRESSION_TYPES] ={
//                                                         ADPCM                      PCM                      MP3
{                                        sizeof( sample_header ), sizeof( sample_header ), sizeof( sample_header ) }, // PC
{                                        sizeof( sample_header ),                       0,                       0 }, // PS2
{ sizeof(sample_header) + sizeof( gcn_adpcm_compression_header ),                       0, sizeof( sample_header ) }, // GameCube
{                                        sizeof( sample_header ), sizeof( sample_header ),                       0 }}; // XBOX

package_info    s_Package;
//------------------------------------------------------------------------------

s32 GetBreakPointSize( xarray<audio_file::breakpoint>& BreakPoints, s32& nBreakPoints )
{
    s32 Result = 0;

    // Init.
    nBreakPoints = 0;

    // Currently very simple
    for( s32 i=0 ; i<BreakPoints.GetCount() ; i++ )
    {
        if( x_strcmp( (const char*)BreakPoints[i].Name, "BP" ) == 0 )
        {
            // Bump size.
            Result += sizeof(f32);

            // Count 'em
            nBreakPoints++;
        }
    }

    // Make room for count.
    if( Result )
        Result += sizeof( s32 );

    return Result;
}

//------------------------------------------------------------------------------

void WriteBreakPoints( xarray<audio_file::breakpoint>& BreakPoints, X_FILE*out, xbool bReverseEndian )
{
    s32 nBreakPoints;

    // Get number of break points
    GetBreakPointSize( BreakPoints, nBreakPoints );
    
    s32 pos = x_ftell( out );
    x_DebugMsg( "WriteBreakPoints ENTER - File Position: %d, nBreakPoints: %d\n", x_ftell( out ), nBreakPoints );

    // Have some?
    if( nBreakPoints )
    {
        // Write out number of break points.
        s32 xnBreakPoints = nBreakPoints;
        if( bReverseEndian )
            xnBreakPoints = reverse_endian_32( xnBreakPoints );
        x_fwrite( &xnBreakPoints, sizeof( s32 ), 1, out );

        // Write out each break point.
        for( s32 i=0 ; i<BreakPoints.GetCount() ; i++ )
        {
            if( x_strcmp( (const char*)BreakPoints[i].Name, "BP" ) == 0 )
            {
                f32 Data = BreakPoints[i].Position;
                x_DebugMsg( "%08.3f ", Data );
                if( bReverseEndian )
                    Data = reverse_endian_f32( Data );
                x_fwrite( &Data, sizeof(f32), 1, out );
            }
        }
    }

    x_DebugMsg( "\nWriteBreakPoints EXIT  - File Position: %d, size: %d\n", x_ftell( out ), x_ftell( out ) - pos );
}

//------------------------------------------------------------------------------

u32 GetLipSyncSize( s32 nSamples, s32 SampleRate )
{
    f32 Rate     = 30.0f;                           // Sampling rate for lip sync data
    f32 Seconds  = (f32)nSamples / (f32)SampleRate; // Length of sample in seconds.
    u32 nLipSync = (u32)((Seconds * Rate) + 0.9999999999999f);

    nLipSync++; // Make space for rate in data stream

    return nLipSync;
}

//------------------------------------------------------------------------------

void WriteLipSyncData( audio_file* Aiff, X_FILE* out )
{
    s32  i;
    s32  SampleRate    = Aiff->GetSampleRate();
    s32  nChannels     = Aiff->GetNumChannels();
    s32  nSamples      = Aiff->GetNumSamples();
    s16* pSampleBuffer = (s16*)x_malloc( sizeof(s16) * nSamples );

    // Currently only support mono and stereo.
    ASSERT( (nChannels == 1) || (nChannels == 2) );
    s32 pos = x_ftell( out );
    x_DebugMsg( "WriteLipSyncData ENTER - File Position: %d\n", x_ftell( out ) );

    // Compress each channel...
    for( i=0 ; i<1 ; i++ )
    {
        f32 Rate     = 30.0f;                           // Sampling rate for lip sync data
        f32 Seconds  = (f32)nSamples / (f32)SampleRate; // Length of sample in seconds.
        s32 nLipSync = (u32)((Seconds * Rate) + 0.9999999999999f);
        u8  cRate    = (u8)Rate;

        // Write out sample rate
        x_fwrite( &cRate, 1, 1, out );

        // Read the the uncompressed waveform data.
        Aiff->GetChannelData( pSampleBuffer, i );

        for( s32 j=0 ; j<nLipSync ; j++ )
        {
            u8 Data;

            // Last sample?
            if( j==(nLipSync-1) )
            {
                Data = 0;
            }
            // Find largest sample...
            else
            {
                s32 StartSample = (s32)((f32)(j+0) * (f32)nSamples / (f32)nLipSync);
                s32 EndSample   = (s32)((f32)(j+1) * (f32)nSamples / (f32)nLipSync);
                u16 Big         = 0;

                for( s32 k=StartSample ; k<EndSample ; k++ )
                {
                    u16 Test = x_abs( pSampleBuffer[k] );
                    if( Test > Big )
                        Big = Test;
                }

                Data = (u8)(Big >> 8);
            }

            // Write out the data.
            x_fwrite( &Data, 1, 1, out );
        }
    }

    x_DebugMsg( "WriteLipSyncData EXIT  - File Position: %d, size: %d\n", x_ftell( out ), x_ftell( out )-pos );

    // Free the buffer.
    x_free( pSampleBuffer );
}

//------------------------------------------------------------------------------

s32 ParameterSize( compressed_parameters* Params )
{
    s32 Result = 0;

    if( Params->Bits1 || Params->Bits2 )
    {
        s32 Bits = (u32)Params->Bits1 + (((u32)Params->Bits2) << 16);

        Result += 6;  // Make space for the size field and parameter bits.

        for( s32 i=0 ; i<NUM_PARAMETERS ; i++ )
        {
            if( Bits & (1<<i) )
                Result += s_ParameterSizes[i];
        }

        // 16-bit align the parameter size.
        if( Result & 1 )
            Result++;
    }

    // Tell the world
    return( Result );
}

//------------------------------------------------------------------------------

void ExportParameters( compressed_parameters* Params, s32 ParamSize, xbool ReverseEndian )
{
    s32 nTotalBytes = 0;
    s32 nBytes      = 0;

    // Export the flags.
    {
        s16 Bits = Params->Flags;
        nBytes   = 2;
        if( ReverseEndian )
            Bits = reverse_endian_16( Bits );
        s_Package.m_DescriptorStream.Append( (const u8*)&Bits, nBytes );
        nTotalBytes += nBytes;
    }

    // Export the size of the parameters.
    if( ParamSize )
    {
        s16 ParamSize16 = ParamSize;
        nBytes = 2;
        if( ReverseEndian )
            ParamSize16 = reverse_endian_16( ParamSize16 );
        s_Package.m_DescriptorStream.Append( (const u8*)&ParamSize16, nBytes );
        nTotalBytes += nBytes;
        
        // Make sure its 16-bit aligned.
        ASSERT( (ParamSize & 1) == 0 );
    }

    // Only if any are defined...
    if( Params->Bits1 || Params->Bits2 )
    {
        u16 Bits1 = Params->Bits1;
        u16 Bits2 = Params->Bits2;
        s32 Bits = ((u32)Bits1 | (((u32)Bits2) << 16));
        s16 Word;

        // Export the parameter bit field
        nBytes = 2;
        if( ReverseEndian )
            Bits1 = reverse_endian_16( Bits1 );
        s_Package.m_DescriptorStream.Append( (const u8*)&Bits1, nBytes );
        nTotalBytes += nBytes;

        if( ReverseEndian )
            Bits2 = reverse_endian_16( Bits2 );
        s_Package.m_DescriptorStream.Append( (const u8*)&Bits2, nBytes );
        nTotalBytes += nBytes;

        if( GET_PITCH_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[PITCH];
            ASSERT( nBytes == 2 );
            Word = Params->Pitch;
            if( ReverseEndian )
                Word = reverse_endian_16( Word );
            s_Package.m_DescriptorStream.Append( (const u8*)&Word, nBytes );
            nTotalBytes += nBytes;
        }

        if( GET_PITCH_VARIANCE_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[PITCH_VARIANCE];
            ASSERT( nBytes == 2 );
            Word = Params->PitchVariance;
            if( ReverseEndian )
                Word = reverse_endian_16( Word );
            s_Package.m_DescriptorStream.Append( (const u8*)&Word, nBytes );
            nTotalBytes += nBytes;
        }

        if( GET_VOLUME_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[VOLUME];
            ASSERT( nBytes == 2 );
            Word = Params->Volume;
            if( ReverseEndian )
                Word = reverse_endian_16( Word );
            s_Package.m_DescriptorStream.Append( (const u8*)&Word, nBytes );
            nTotalBytes += nBytes;
        }

        if( GET_VOLUME_VARIANCE_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[VOLUME_VARIANCE];
            ASSERT( nBytes == 2 );
            Word = Params->VolumeVariance;
            if( ReverseEndian )
                Word = reverse_endian_16( Word );
            s_Package.m_DescriptorStream.Append( (const u8*)&Word, nBytes );
            nTotalBytes += nBytes;
        }

        if( GET_VOLUME_CENTER_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[VOLUME_CENTER];
            ASSERT( nBytes == 2 );
            Word = Params->VolumeCenter;
            if( ReverseEndian )
                Word = reverse_endian_16( Word );
            s_Package.m_DescriptorStream.Append( (const u8*)&Word, nBytes );
            nTotalBytes += nBytes;
        }

        if( GET_VOLUME_LFE_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[VOLUME_LFE];
            ASSERT( nBytes == 2 );
            Word = Params->VolumeLFE;
            if( ReverseEndian )
                Word = reverse_endian_16( Word );
            s_Package.m_DescriptorStream.Append( (const u8*)&Word, nBytes );
            nTotalBytes += nBytes;
        }

        if( GET_VOLUME_DUCK_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[VOLUME_DUCK];
            ASSERT( nBytes == 2 );
            Word = Params->VolumeDuck;
            if( ReverseEndian )
                Word = reverse_endian_16( Word );
            s_Package.m_DescriptorStream.Append( (const u8*)&Word, nBytes );
            nTotalBytes += nBytes;
        }

        if( GET_USER_DATA_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[USER_DATA];
            ASSERT( nBytes == 2 );
            Word = Params->UserData;
            if( ReverseEndian )
                Word = reverse_endian_16( Word );
            s_Package.m_DescriptorStream.Append( (const u8*)&Word, nBytes );
            nTotalBytes += nBytes;
        }

        if( GET_REPLAY_DELAY_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[REPLAY_DELAY];
            ASSERT( nBytes == 2 );
            Word = Params->ReplayDelay;
            if( ReverseEndian )
                Word = reverse_endian_16( Word );
            s_Package.m_DescriptorStream.Append( (const u8*)&Word, nBytes );
            nTotalBytes += nBytes;
            ASSERT( GET_LAST_PLAY_BIT( Bits ) );
        }

        if( GET_LAST_PLAY_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[LAST_PLAY];
            ASSERT( nBytes == 2 );
            Word = Params->LastPlay;
            if( ReverseEndian )
                Word = reverse_endian_16( Word );
            s_Package.m_DescriptorStream.Append( (const u8*)&Word, nBytes );
            nTotalBytes += nBytes;
        }

        if( GET_PAN_2D_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[PAN_2D];
            ASSERT( nBytes == 1 );
            s_Package.m_DescriptorStream += Params->Pan2d;
            nTotalBytes += nBytes;
        }

        if( GET_PRIORITY_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[PRIORITY];
            ASSERT( nBytes == 1 );
            s_Package.m_DescriptorStream += Params->Priority;
            nTotalBytes += nBytes;
        }

        if( GET_EFFECT_SEND_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[EFFECT_SEND];
            ASSERT( nBytes == 1 );
            s_Package.m_DescriptorStream += Params->EffectSend;
            nTotalBytes += nBytes;
        }

        if( GET_NEAR_FALLOFF_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[NEAR_FALLOFF];
            ASSERT( nBytes == 1 );
            s_Package.m_DescriptorStream += Params->NearFalloff;
            nTotalBytes += nBytes;
        }

        if( GET_FAR_FALLOFF_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[FAR_FALLOFF];
            ASSERT( nBytes == 1 );
            s_Package.m_DescriptorStream += Params->FarFalloff;
            nTotalBytes += nBytes;
        }

        if( GET_ROLLOFF_METHOD_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[ROLLOFF_METHOD];
            ASSERT( nBytes == 1 );
            s_Package.m_DescriptorStream += Params->RolloffCurve;
            nTotalBytes += nBytes;
        }

        if( GET_NEAR_DIFFUSE_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[NEAR_DIFFUSE];
            ASSERT( nBytes == 1 );
            s_Package.m_DescriptorStream += Params->NearDiffuse;
            nTotalBytes += nBytes;
        }

        if( GET_FAR_DIFFUSE_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[FAR_DIFFUSE];
            ASSERT( nBytes == 1 );
            s_Package.m_DescriptorStream += Params->FarDiffuse;
            nTotalBytes += nBytes;
        }

        if( GET_PLAY_PERCENT_BIT( Bits ) )
        {
            nBytes = s_ParameterSizes[PLAY_PERCENT];
            ASSERT( nBytes == 1 );
            s_Package.m_DescriptorStream += Params->PlayPercent;
            nTotalBytes += nBytes;
        }

        // Need to 16 bit align the stream
        if( s_Package.m_DescriptorStream.GetLength() & 1 )
        {
            s_Package.m_DescriptorStream += (char)0;
            nTotalBytes++;
        }
    }

    // Make sure it matches (adjsut for the flags).
    ASSERT( nTotalBytes == ParamSize+2 );
}

//------------------------------------------------------------------------------

void ExportElement( s32 DescriptorIndex, s32 ElementIndex, xbool ReverseEndian )
{
    s32 ParamSize;
    u16 IndexId;

    // Set the elements type, parameter length and index.
    SET_INDEX_TYPE( IndexId, s_Package.m_Descriptors[DescriptorIndex].Elements[ElementIndex].Type );
    ParamSize = ParameterSize( &s_Package.m_Descriptors[DescriptorIndex].Elements[ElementIndex].Params );
    SET_INDEX_HAS_PARAMS( IndexId, ParamSize );
    ASSERT( s_Package.m_Descriptors[DescriptorIndex].Elements[ElementIndex].Index >= 0 );
    ASSERT( s_Package.m_Descriptors[DescriptorIndex].Elements[ElementIndex].Index < MAX_DESCRIPTOR_INDEX );
    SET_INDEX( IndexId, s_Package.m_Descriptors[DescriptorIndex].Elements[ElementIndex].Index );

    // Export the element index id
    if( ReverseEndian )
        IndexId = reverse_endian_16( IndexId );
    s_Package.m_DescriptorStream.Append( (const u8*)&IndexId, 2 );

    // Export the elements parameters.
    ExportParameters( &s_Package.m_Descriptors[DescriptorIndex].Elements[ElementIndex].Params, ParamSize, ReverseEndian );
}

//------------------------------------------------------------------------------

void ExportSimple( s32 DescriptorIndex, xbool ReverseEndian )
{
    // Export a single element.
    ExportElement( DescriptorIndex, 0, ReverseEndian );
}

//------------------------------------------------------------------------------

void ExportComplex( s32 DescriptorIndex, xbool ReverseEndian )
{
    s32 i;
    s32 nElements    = s_Package.m_Descriptors[ DescriptorIndex ].Elements.GetCount();
    u16 ElementCount = (u16)nElements;

    // Export the number of elements.
    if( ReverseEndian )
        ElementCount = reverse_endian_16( ElementCount );
    s_Package.m_DescriptorStream.Append( (const u8*)&ElementCount, 2 );

    // Clear the exported flag.
    for( i=0 ; i<nElements ; i++ )
    {
        s_Package.m_Descriptors[ DescriptorIndex ].Elements[i].IsExported = FALSE;
    }

    // Export them in ascending time deltas.
    for( i=0 ; i<nElements ; i++ )
    {
        s32 EarliestElement = -1;         // Not defined...
        f32 EarliestTime    = 1000000.0f; // Very large... 
        u16 TimeDelay; 

        // Damn n-squared search...
        for( s32 j=0 ; j<nElements ; j++ )
        {
            if( !s_Package.m_Descriptors[DescriptorIndex].Elements[j].IsExported && s_Package.m_Descriptors[DescriptorIndex].Elements[j].StartDelay < EarliestTime )
            {
                EarliestElement = j;
                EarliestTime    = s_Package.m_Descriptors[DescriptorIndex].Elements[j].StartDelay;
            }
        }

        // Mark it as exported.
        s_Package.m_Descriptors[DescriptorIndex].Elements[EarliestElement].IsExported = TRUE;

        // Export the time delta.
        TimeDelay = FLOAT100_TO_U16BIT( s_Package.m_Descriptors[DescriptorIndex].Elements[EarliestElement].StartDelay );
        if( ReverseEndian )
            TimeDelay = reverse_endian_16( TimeDelay );
        s_Package.m_DescriptorStream.Append( (const u8*)&TimeDelay, 2 );

        // Export the element.
        ExportElement( DescriptorIndex, EarliestElement, ReverseEndian );
    }
}

//------------------------------------------------------------------------------

void ExportRandomList( s32 DescriptorIndex, xbool ReverseEndian )
{
    s32 i;
    s32 nElements    = s_Package.m_Descriptors[ DescriptorIndex ].Elements.GetCount();
    u16 ElementCount = (u16)nElements;

    // Export the number of elements.
    if( ReverseEndian )
        ElementCount = reverse_endian_16( ElementCount );
    s_Package.m_DescriptorStream.Append( (const u8*)&ElementCount, 2 );

    // Now make space for the 64 bit flag field.
    for( i=0 ; i<8 ; i++ )
    {
        s_Package.m_DescriptorStream += '\0';
    }

    // Export each element.
    ASSERT( nElements <= 63 );
    for( i=0 ; i<nElements ; i++ )
    {
        // Export the element.
        ExportElement( DescriptorIndex, i, ReverseEndian );
    }
}

//------------------------------------------------------------------------------

void ExportWeightedList( s32 DescriptorIndex, xbool ReverseEndian )
{
    s32 i;
    f32 TotalWeight;
    f32 CurrentWeight;
    s32 nElements    = s_Package.m_Descriptors[ DescriptorIndex ].Elements.GetCount();
    u16 ElementCount = (u16)nElements;

    // Export the number of elements.
    if( ReverseEndian )
        ElementCount = reverse_endian_16( ElementCount );
    s_Package.m_DescriptorStream.Append( (const u8*)&ElementCount, 2 );

    // Calulate the total weight of all elements.
    for( i=0,TotalWeight=0.0f ; i<nElements ; i++ )
    {
        TotalWeight += (f32)s_Package.m_Descriptors[DescriptorIndex].Elements[i].Weight;
    }

    // Export the weights.
    for( i=0,CurrentWeight=0.0f ; i<nElements ; i++ )
    {
        u16 Weight;
        
        if( i == (nElements-1) )
        {
            Weight = 65535;
        }
        else
        {
            CurrentWeight += (f32)s_Package.m_Descriptors[DescriptorIndex].Elements[i].Weight / TotalWeight;
            if( CurrentWeight > 1.0f )
                CurrentWeight = 1.0f;
            Weight = (u16)(CurrentWeight * 65535.0f);
        }

        if( ReverseEndian )
            Weight = reverse_endian_16( Weight );
        s_Package.m_DescriptorStream.Append( (const u8*)&Weight, 2 );
    }

    // Export each element.
    for( i=0 ; i<nElements ; i++ )
    {
        // Export the element.
        ExportElement( DescriptorIndex, i, ReverseEndian );
    }
}

//------------------------------------------------------------------------------

void ExportDescriptorIdentifiers( xbool ReverseEndian )
{
    s32   StringSize;
    s32   i;
    s32   j;
    u8    Dummy[2] = { 0x7f, 0x00 };

    // Clear the identifier export streams and offsets.
    s_Package.m_StringTableStream.Clear();
    s_Package.m_DescriptorIdentifierStream.Clear();

    // Mark each identifier as not processed.
    for( i=0 ; i<s_Package.m_Descriptors.GetCount() ; i++ )
    {
        // Mark identifier as not processed.
        s_Package.m_Descriptors[i].IdentifierProcessed = FALSE;
    }

    // Process each descriptor...
    for( i=0 ; i<s_Package.m_Descriptors.GetCount() ; i++ )
    {
        const char* SmallString = (const char*)Dummy;
        s32         SmallIndex  = -1;
        u16         Word;
        u32         Offset;

        // N-squared sort...I know...
        for( j=0 ; j<s_Package.m_Descriptors.GetCount() ; j++ )
        {
            if( !s_Package.m_Descriptors[ j ].IdentifierProcessed )
            {
                if( x_strcmp( (const char*)s_Package.m_Descriptors[ j ].Identifier, SmallString ) < 0 )
                {
                    SmallString = (const char*)s_Package.m_Descriptors[ j ].Identifier;
                    SmallIndex  = j;
                }
            }
        }

        // Mark it as processed.
        s_Package.m_Descriptors[ SmallIndex ].IdentifierProcessed = TRUE;

        // Set the descriptor identifier offset
        StringSize = x_strlen( (const char *)s_Package.m_Descriptors[ SmallIndex ].Identifier );
        Offset     = s_Package.m_StringTableStream.GetLength(); 
        
        // Write null terminated string to the string table.
        s_Package.m_StringTableStream.Append( (const unsigned char*)((const char *)s_Package.m_Descriptors[ SmallIndex ].Identifier), StringSize );
        s_Package.m_StringTableStream += '\0';

        // Export the string table offset.
        ASSERT( Offset <= 65535 );
        Word = Offset;
        if( ReverseEndian )
            Word = reverse_endian_16( Word );
        s_Package.m_DescriptorIdentifierStream.Append( (const u8*)&Word, 2 );

        // Export the index.
        ASSERT( SmallIndex <= 65535 );
        Word = SmallIndex;
        if( ReverseEndian )
            Word = reverse_endian_16( Word );
        s_Package.m_DescriptorIdentifierStream.Append( (const u8*)&Word, 2 );

        // Make space for the pointer to the package.
        s_Package.m_DescriptorIdentifierStream += 0;
        s_Package.m_DescriptorIdentifierStream += 0;
        s_Package.m_DescriptorIdentifierStream += 0;
        s_Package.m_DescriptorIdentifierStream += 0;
    }

    // Pad string table to 32-bit alingment.
    while( s_Package.m_StringTableStream.GetLength() & 3 )
        s_Package.m_StringTableStream += 0;
}

//------------------------------------------------------------------------------

void ExportDescriptors( xbool ReverseEndian )
{
    // Clear the descriptor export stream and offsets.
    s_Package.m_DescriptorStream.Clear();
    s_Package.m_DescriptorOffsets.Clear();
    
    // Process each descriptor...
    for( s32 i=0 ; i<s_Package.m_Descriptors.GetCount() ; i++ )
    {
        u16 DescriptorId=0;
        s32 ParamSize;

        if( s_Debug )
        {
            x_printf( "    %s\n", s_Package.m_Descriptors[i].Identifier );
            x_DebugMsg( "    %s\n", s_Package.m_Descriptors[i].Identifier );
        }

        // Set the descriptor offset (must be 16 bit aligned).
        s_Package.m_DescriptorOffsets.Append() = s_Package.m_DescriptorStream.GetLength();
        ASSERT( (s_Package.m_DescriptorOffsets[i] & 1) == 0 );

        // Set type and parameter length of the descriptor.
        SET_DESCRIPTOR_TYPE( DescriptorId, s_Package.m_Descriptors[i].Type );
        ParamSize = ParameterSize( &s_Package.m_Descriptors[i].Params );
        SET_DESCRIPTOR_HAS_PARAMS( DescriptorId, ParamSize );

        // Export descriptor id.
        if( ReverseEndian )
            DescriptorId = reverse_endian_16( DescriptorId );
        s_Package.m_DescriptorStream.Append( (const u8*)&DescriptorId, 2 );

        // Export the parameters.
        ExportParameters( &s_Package.m_Descriptors[i].Params, ParamSize, ReverseEndian );

        switch( s_Package.m_Descriptors[i].Type )
        {
            case SIMPLE:
                ExportSimple( i, ReverseEndian );
                break;

            case COMPLEX:
                ExportComplex( i, ReverseEndian );
                break;

            case RANDOM_LIST:
                ExportRandomList( i, ReverseEndian );
                break;

            case WEIGHTED_LIST:
                ExportWeightedList( i, ReverseEndian );
                break;
        }

        // 16 bit align
        if( s_Package.m_DescriptorStream.GetLength() & 1 )
            s_Package.m_DescriptorStream += '\0';
    }
}

//------------------------------------------------------------------------------

void ExportLipSyncData( s32 Target )
{
    multi_channel_data Data;

    // Clear the descriptor export stream and offsets.
    s_Package.m_LipSyncTableStream.Clear();
 
    for( s32 i=0 ; i<s_Package.m_Files.GetCount() ; i++ )
    {
        file_info& File = s_Package.m_Files[ i ];

        if( File.UsesLipSync )
        {
            // Set the offset.
            File.LipSyncOffset = s_Package.m_LipSyncTableStream.GetLength();
            
            // Load the file.
            LoadTrueMultiChannelFile( File, &Data, TRUE, Target );

            // Mono files for now...
            ASSERT( Data.nChannels == 1 );

            // Append the lip sync data.
            s_Package.m_LipSyncTableStream.Append( Data.LipSyncData[0] );
        }
        else
        {
            File.LipSyncOffset = -1;
        }
    }
}

//------------------------------------------------------------------------------

void ExportBreakPointData( s32 Target )
{
    multi_channel_data Data;

    // Clear the export stream
    s_Package.m_BreakPointTableStream.Clear();
 
    for( s32 i=0 ; i<s_Package.m_Files.GetCount() ; i++ )
    {
        file_info& File = s_Package.m_Files[ i ];

        // Load the file.
        LoadTrueMultiChannelFile( File, &Data, TRUE, Target );

        // Break points?
        if( Data.BreakPointSize > 0 )
        {
            // Set the offset.
            File.BreakPointOffset = s_Package.m_BreakPointTableStream.GetLength();
            
            // Append the break point data.
            s_Package.m_BreakPointTableStream.Append( Data.BreakPointData );
        }
        else
        {
            File.BreakPointOffset = -1;
        }
    }
}

//------------------------------------------------------------------------------

void ExportMusicData( void )
{
    s32 nIntensity = s_Package.m_Intensity.GetCount();
    s_Package.m_MusicStream.Clear();

    if( nIntensity )
    {
        s_Package.m_MusicStream.Append( (const u8*)s_Package.m_MusicType, 32 );
        for( s32 i=0 ; i<nIntensity ; i++ )
        {
            s_Package.m_MusicStream.Append( (const u8)s_Package.m_Intensity[i].Level );
            s_Package.m_MusicStream.Append( (const u8*)s_Package.m_Intensity[i].Descriptor, 31 );
        }
    }

    s_Package.m_MusicFootprint = s_Package.m_MusicStream.GetLength();
}
//------------------------------------------------------------------------------

s32 CalculateAudioRam( s32 Target )
{
    s32 i;
    s32 Footprint;

    for( i=0, Footprint=0 ; i<s_Package.m_Files.GetCount() ; i++ )
    {
        if( s_Package.m_Files[i].Temperature == HOT )
        {
            Footprint += s_Package.m_Files[i].CompressedSize[Target];
        }

        if( s_Package.m_Files[i].Temperature == WARM )
        {
            // TODO: Put in warm sample contribution.
        }
    }

    return Footprint;
}

//------------------------------------------------------------------------------

s32 CalculateMainRam( s32 Target )
{
    s32 Result = 0;
    
    // Descriptor identifiers.
    Result += s_Package.m_DescriptorIdentifierStream.GetLength();

    // String table.
    Result += s_Package.m_StringTableStream.GetLength();

    // Descriptor offset table.
    Result += s_Package.m_Descriptors.GetCount() * sizeof(s32);

    // Descriptors
    Result += s_Package.m_DescriptorStream.GetLength();

    // Number of samples for each temperature + offset table for each temperature.
    for( s32 i=0 ; i<NUM_TEMPERATURES ; i++ )
    {
        Result += sizeof(s32);
        s32 Count = s_Package.m_TemperatureCount[ i ];
        Result += Count * sizeof(s32);
    }

    return Result;
}

//------------------------------------------------------------------------------

s32 CalculateNumberOfSampleHeaders( s32 Temperature, s32 Target )
{
    s32 Total = 0;

    for( s32 i=0 ; i<s_Package.m_Files.GetCount() ; i++ )
    {
        file_info& File = s_Package.m_Files[ i ];

        if( File.Temperature == (u32)Temperature )
            Total += File.NumChannels[ Target ];
    }

    return Total;
}

//------------------------------------------------------------------------------

void ExportPackageHeader( xbool ReverseEndian, s32 Target )
{
    package_header PH;

    // Clear the package header stream.
    s_Package.m_HeaderStream.Clear();

    PH.Mram         = s_Package.m_MainRamFootprint  = CalculateMainRam( Target );
    PH.Aram         = s_Package.m_AudioRamFootprint = CalculateAudioRam( Target );
    PH.Params.Bits  = 0xffffffff;
    PH.Params.Flags = s_Package.m_Params.Flags;
        
    // Is it compressed 1.0?
    if( s_Package.m_Params.Pitch == FLOAT4_TO_U16BIT( 1.0f ) )
    {
        PH.Params.Pitch = 1.0f;
    }
    else
    {
        PH.Params.Pitch = U16BIT_TO_FLOAT4( s_Package.m_Params.Pitch );
    }

    // Is it compressed 0.0?
    if( s_Package.m_Params.PitchVariance == FLOAT1_TO_U16BIT( 0.0f ) )
    {
        PH.Params.PitchVariance = 0.0f;
    }
    else
    {
        PH.Params.PitchVariance = U16BIT_TO_FLOAT1( s_Package.m_Params.PitchVariance );
    }

    // Is it compressed 1.0?
    if( s_Package.m_Params.Volume == FLOAT1_TO_U16BIT( 1.0f ) )
    {
        PH.Params.Volume = 1.0f;
    }
    else
    {
        PH.Params.Volume = U16BIT_TO_FLOAT1( s_Package.m_Params.Volume );
    }

    // Is it compressed 0.0f?
    if( s_Package.m_Params.VolumeVariance == FLOAT1_TO_U16BIT( 0.0f ) )
    {
        PH.Params.VolumeVariance = 0.0f;
    }
    else
    {
        PH.Params.VolumeVariance = U16BIT_TO_FLOAT1( s_Package.m_Params.VolumeVariance );
    }

    // Is it compressed 0.0f?
    if( s_Package.m_Params.VolumeCenter == FLOAT1_TO_U16BIT( 0.0f ) )
    {
        PH.Params.VolumeCenter = 0.0f;
    }
    else
    {
        PH.Params.VolumeCenter = U16BIT_TO_FLOAT1( s_Package.m_Params.VolumeCenter );
    }

    // Is it compressed 0.0f?
    if( s_Package.m_Params.VolumeLFE == FLOAT1_TO_U16BIT( 0.0f ) )
    {
        PH.Params.VolumeLFE = 0.0f;
    }
    else
    {
        PH.Params.VolumeLFE = U16BIT_TO_FLOAT1( s_Package.m_Params.VolumeLFE );
    }

    // Is it compressed 1.0f?
    if( s_Package.m_Params.VolumeDuck == FLOAT1_TO_U16BIT( 1.0f ) )
    {
        PH.Params.VolumeDuck = 1.0f;
    }
    else
    {
        PH.Params.VolumeDuck = U16BIT_TO_FLOAT1( s_Package.m_Params.VolumeDuck );
    }

    PH.Params.UserData     = s_Package.m_Params.UserData;
    PH.Params.ReplayDelay  = U16BIT_TO_FLOAT_TENTH( s_Package.m_Params.ReplayDelay );
    PH.Params.LastPlay     = 0.0f;
    PH.Params.Pan2d        = S8BIT_TO_FLOAT1 ( s_Package.m_Params.Pan2d );
    PH.Params.Priority     = (u32)s_Package.m_Params.Priority;
    PH.Params.EffectSend   = U8BIT_TO_FLOAT1 ( s_Package.m_Params.EffectSend );
    PH.Params.NearFalloff  = U8BIT_TO_FLOAT10( s_Package.m_Params.NearFalloff );
    PH.Params.FarFalloff   = U8BIT_TO_FLOAT10( s_Package.m_Params.FarFalloff );
    PH.Params.RolloffCurve = (u32)s_Package.m_Params.RolloffCurve;
    PH.Params.NearDiffuse  = U8BIT_TO_FLOAT10( s_Package.m_Params.NearDiffuse );
    PH.Params.FarDiffuse   = U8BIT_TO_FLOAT10( s_Package.m_Params.FarDiffuse );
    PH.Params.PlayPercent  = s_Package.m_Params.PlayPercent;

    // TODO: Not necessarily a one-to-one corespondence between nDescriptrs and nIdentifiers.
    PH.nDescriptors             = s_Package.m_Descriptors.GetCount();
    PH.nIdentifiers             = s_Package.m_Descriptors.GetCount(); 
    PH.DescriptorFootprint      = s_Package.m_DescriptorStream.GetLength();
    PH.StringTableFootprint     = s_Package.m_StringTableStream.GetLength();
    PH.LipSyncTableFootprint    = s_Package.m_LipSyncTableStream.GetLength();
    PH.BreakPointTableFootprint = s_Package.m_BreakPointTableStream.GetLength();
    PH.MusicDataFootprint       = s_Package.m_MusicStream.GetLength();

    for( s32 i=0 ; i<NUM_TEMPERATURES ; i++ )
    {
        PH.nSampleHeaders[ i ]  = CalculateNumberOfSampleHeaders( i, Target );
        PH.nSampleIndices[ i ]  = s_Package.m_TemperatureCount[ i ];
        PH.CompressionType[ i ] = s_CompressionTypes[ Target ][ i ];
        PH.HeaderSizes[ i ]     = s_CompressionHeaderSizes[ Target ][ PH.CompressionType[ i ] ];
    }

    if( ReverseEndian )
    {
        PH.Mram                     = reverse_endian_32 ( PH.Mram );
        PH.Aram                     = reverse_endian_32 ( PH.Aram );
        PH.Params.Bits              = reverse_endian_32 ( PH.Params.Bits );
        PH.Params.Flags             = reverse_endian_32 ( PH.Params.Flags );
        PH.Params.Pitch             = reverse_endian_f32( PH.Params.Pitch );
        PH.Params.PitchVariance     = reverse_endian_f32( PH.Params.PitchVariance );
        PH.Params.Volume            = reverse_endian_f32( PH.Params.Volume );
        PH.Params.VolumeVariance    = reverse_endian_f32( PH.Params.VolumeVariance );
        PH.Params.VolumeCenter      = reverse_endian_f32( PH.Params.VolumeCenter );
        PH.Params.VolumeLFE         = reverse_endian_f32( PH.Params.VolumeLFE );
        PH.Params.VolumeDuck        = reverse_endian_f32( PH.Params.VolumeDuck );
        PH.Params.UserData          = reverse_endian_32 ( PH.Params.UserData );
        PH.Params.ReplayDelay       = reverse_endian_f32( PH.Params.ReplayDelay );
        PH.Params.LastPlay          = reverse_endian_f32( PH.Params.LastPlay );
        PH.Params.Pan2d             = reverse_endian_f32( PH.Params.Pan2d );
        PH.Params.Priority          = reverse_endian_32 ( PH.Params.Priority );
        PH.Params.EffectSend        = reverse_endian_f32( PH.Params.EffectSend );
        PH.Params.NearFalloff       = reverse_endian_f32( PH.Params.NearFalloff );
        PH.Params.FarFalloff        = reverse_endian_f32( PH.Params.FarFalloff );
        PH.Params.RolloffCurve      = reverse_endian_32 ( PH.Params.RolloffCurve );
        PH.Params.NearDiffuse       = reverse_endian_f32( PH.Params.NearDiffuse );
        PH.Params.FarDiffuse        = reverse_endian_f32( PH.Params.FarDiffuse );
        PH.Params.PlayPercent       = reverse_endian_32 ( PH.Params.PlayPercent );

        PH.nDescriptors             = reverse_endian_32 ( PH.nDescriptors );
        PH.nIdentifiers             = reverse_endian_32 ( PH.nIdentifiers );
        PH.DescriptorFootprint      = reverse_endian_32 ( PH.DescriptorFootprint );
        PH.StringTableFootprint     = reverse_endian_32 ( PH.StringTableFootprint );
        PH.LipSyncTableFootprint    = reverse_endian_32 ( PH.LipSyncTableFootprint );
        PH.BreakPointTableFootprint = reverse_endian_32 ( PH.BreakPointTableFootprint );
        PH.MusicDataFootprint       = reverse_endian_32 ( PH.MusicDataFootprint );

        for( i=0 ; i<NUM_TEMPERATURES ; i++ )
        {
            PH.nSampleHeaders[ i ]  = reverse_endian_32( PH.nSampleHeaders[ i ] );
            PH.nSampleIndices[ i ]  = reverse_endian_32( PH.nSampleIndices[ i ] );
            PH.CompressionType[ i ] = reverse_endian_32( PH.CompressionType[ i ] );
            PH.HeaderSizes[ i ]     = reverse_endian_32( PH.HeaderSizes[ i ] );
        }
    }

    s_Package.m_HeaderStream.Append( (const u8*)&PH, sizeof(package_header) );
}

//------------------------------------------------------------------------------

void WritePackageToDisk( const char*Filename, xbool ReverseEndian, s32 Target )
{
    xbytestream Final;
    xbytestream SampleStream;
    s32         TotalSampleTableSize = 0;    
    s32         TotalSampleTableSize2 = 0;
    s32         CompressionType;
    s32         HeaderSize;
    s32         Fixup;
    X_FILE*     Out;
    char        VersionID[VERSION_ID_SIZE];
    char        TargetID[TARGET_ID_SIZE];
    char        UserID[USER_ID_SIZE];
    char        DateTimeString[DATE_TIME_SIZE];
    char*       TargetString;
    SYSTEMTIME  SysTime;
    xarray<s16> IndexTable; 
    multi_channel_data Data;

    Final.Clear();
    SampleStream.Clear();

    if( s_Verbose )
    {
        x_printf( "*** Exporting to file '%s ***'\n", Filename  );
        x_DebugMsg( "*** Exporting to file '%s ***'\n", Filename );
    }

    // Write out the version id.
    x_memset( VersionID, 0, VERSION_ID_SIZE );
    switch( Target )
    {
        case EXPORT_PC:   TargetString = PC_PACKAGE_VERSION; break;
        case EXPORT_PS2:  TargetString = PS2_PACKAGE_VERSION; break;
        case EXPORT_GCN:  TargetString = GCN_PACKAGE_VERSION; break;
        case EXPORT_XBOX: TargetString = XBOX_PACKAGE_VERSION; break;
    }
    x_strncpy( VersionID, TargetString, VERSION_ID_SIZE );
    Final.Append( (u8*)VersionID, VERSION_ID_SIZE );

    // Write out the target id.
    x_memset( TargetID, 0, TARGET_ID_SIZE );
    switch( Target )
    {
        case EXPORT_PC:   TargetString = PC_TARGET_ID; break;
        case EXPORT_PS2:  TargetString = PS2_TARGET_ID; break;
        case EXPORT_GCN:  TargetString = GCN_TARGET_ID; break;
        case EXPORT_XBOX: TargetString = XBOX_TARGET_ID; break;
    }
    x_strncpy( TargetID, TargetString, TARGET_ID_SIZE );
    Final.Append( (u8*)TargetID, TARGET_ID_SIZE );

    // Write out the user name.
    x_memset( UserID, 0, USER_ID_SIZE );
    x_strncpy( UserID, "Unknown", USER_ID_SIZE );
    if( getenv("USERNAME") )  
    {
        // Use the username.
        x_strncpy( UserID, getenv("USERNAME"), USER_ID_SIZE );
    }
    Final.Append( (u8*)UserID, USER_ID_SIZE );
    
    // Write out the date and time.
    x_memset( DateTimeString, 0, 16 );
    GetSystemTime( &SysTime );
    x_sprintf( DateTimeString, "%02d/%02d/%02d %02d:%02d", SysTime.wMonth, SysTime.wDay, SysTime.wYear-2000, SysTime.wHour, SysTime.wMinute );
    Final.Append( (u8*)DateTimeString, DATE_TIME_SIZE );
    
    // Write the package header out.
    Final.Append( s_Package.m_HeaderStream );

    // Write out the string table.
    Final.Append( s_Package.m_StringTableStream );

    // Write out the music data.
    if( s_Package.m_MusicStream.GetLength() )
        Final.Append( s_Package.m_MusicStream );

    // Write out the lipsync table
    if( s_Package.m_LipSyncTableStream.GetLength() )
        Final.Append( s_Package.m_LipSyncTableStream );

    // Write out the break points
    if( s_Package.m_BreakPointTableStream.GetLength() )
        Final.Append( s_Package.m_BreakPointTableStream );

    // Write out the descriptor identifiers.
    Final.Append( s_Package.m_DescriptorIdentifierStream );

    // Write out the descriptor offset table.
    for( s32 i=0 ; i<s_Package.m_Descriptors.GetCount() ; i++ )
    {
        s32 Offset;
        Offset = s_Package.m_DescriptorOffsets[i];
        if( ReverseEndian )
            Offset = reverse_endian_32( Offset );
        Final.Append( (const u8*)&Offset, sizeof(s32) );
    }

    // Write out the descriptors
    Final.Append( s_Package.m_DescriptorStream );

    // For each temperature.
    for( i=0 ; i<NUM_TEMPERATURES ; i++ )
    {
        s16 Count = 0;
/*
x_printf  ( "************************************************************\n" );
x_DebugMsg( "************************************************************\n" );
x_printf  ( "Temperature:%d\n", i );
x_DebugMsg( "Temperature:%d\n", i );
*/
        if( s_Package.m_TemperatureCount[ i ] )
        {
            IndexTable.Clear();
            IndexTable.SetCapacity( s_Package.m_TemperatureCount[ i ] + 1 );

            // Get compression type based on target and temperature.
            CompressionType = s_CompressionTypes[ Target ][ i ];

            // Get header size based on 'target' and compression type.
            HeaderSize = s_CompressionHeaderSizes[ Target ][ CompressionType ];
/*
x_printf  ( "CompressionType: %d, HeaderSize: %d\n", CompressionType, HeaderSize );
x_DebugMsg( "CompressionType: %d, HeaderSize: %d\n", CompressionType, HeaderSize );
*/        
            // For every file...
            for( s32 j=0 ; j<s_Package.m_Files.GetCount() ; j++ )
            {
                file_info& File = s_Package.m_Files[ j ];

                // Is this file the correct temperature?
                if( File.Temperature == (u32)i )
                {
                    // Set the count.
                    IndexTable.Append() = Count;
/*
x_printf  ( "nChannels: %d, File: %s\n", File.NumChannels[ Target ], File.Filename  );
x_DebugMsg( "nChannels: %d, File: %s\n", File.NumChannels[ Target ], File.Filename  );
*/
                    // Bump size by number of channels * the header size.
                    TotalSampleTableSize += (File.NumChannels[ Target ] * HeaderSize);

                    // Bump count
                    Count += File.NumChannels[ Target ];
                }
            }

            // Dummy one at end for size calculations.
            IndexTable.Append() = Count;

            for( j=0 ; j<IndexTable.GetCount() ; j++ )
            {
                s16 Word = IndexTable[j];
                if( ReverseEndian )
                    Word = reverse_endian_16( Word );

                // Write out the index table
                Final.Append( (const u8*)&Word, sizeof(s16) );
            }
        }
    }
/*
x_printf  ( "************************************************************\n" );
x_DebugMsg( "************************************************************\n" );
*/
    // Calculate fixup for the samples.
    Fixup  = Final.GetLength();
    Fixup += TotalSampleTableSize;

    // For each temperature...
    for( i=0 ; i<NUM_TEMPERATURES ; i++ )
    {
        s32 AudioRam = 0;
/*
x_printf  ( "============================================================\n" );
x_DebugMsg( "============================================================\n" );
x_printf  ( "Temperature:%d\n", i );
x_DebugMsg( "Temperature:%d\n", i );
*/
        // Get compression type based on file temperature.
        s32 xCompressionType = CompressionType = s_CompressionTypes[ Target ][ i ];
        if( ReverseEndian )
            xCompressionType = reverse_endian_32( xCompressionType );
/*            
x_printf  ( "CompressionType: %d\n", CompressionType );
x_DebugMsg( "CompressionType: %d\n", CompressionType );
*/
        if( i == COLD )
        {
            // Pad to sector boundary within the file.
            s32 Base = SampleStream.GetLength() + Fixup;
            s32 Padding = (((Base + 2047) / 2048) * 2048) - Base;
            while( Padding-- )
                SampleStream += '\0';
        }

        // For every file...
        for( s32 j=0 ; j<s_Package.m_Files.GetCount() ; j++ )
        {
            file_info& File = s_Package.m_Files[ j ];

            // Is this file the correct temperature?
            if( File.Temperature == (u32)i )
            {
                s32 WaveformOffset = SampleStream.GetLength() + Fixup;

                // Load the file.
                LoadTrueMultiChannelFile( File, &Data, TRUE, Target );

                // For each channel...
                for( s32 k=0 ; k<Data.nChannels ; k++ )
                {
                    s32 HeaderSize2 = TotalSampleTableSize2;

                    // Make space for audio ram reference
                    Final.Append( (const u8*)&AudioRam, sizeof(s32) );
                    TotalSampleTableSize2 += sizeof( s32 );

                    // Adjust for multiple waveforms.
                    if( Data.nWaveforms > 1 )
                        WaveformOffset = SampleStream.GetLength() + Fixup;

                    // Write out the offset.
                    s32 xWaveformOffset = WaveformOffset; 
                    if( ReverseEndian )
                        xWaveformOffset = reverse_endian_32( xWaveformOffset );
                    Final.Append( (const u8*)&xWaveformOffset, sizeof(s32) );
                    TotalSampleTableSize2 += sizeof( s32 );

                    // Write out length
                    s32 xWaveformLength = Data.CompressedSize;
                    if( ReverseEndian )
                        xWaveformLength = reverse_endian_32( xWaveformLength );
                    Final.Append( (const u8*)&xWaveformLength, sizeof(s32) );
                    TotalSampleTableSize2 += sizeof( s32 );

                    // Write out offset of lipsync data.
                    s32 xLipSyncOffset= File.LipSyncOffset;
                    if( ReverseEndian )
                        xLipSyncOffset = reverse_endian_32( xLipSyncOffset );
                    Final.Append( (const u8*)&xLipSyncOffset, sizeof(s32) );
                    TotalSampleTableSize2 += sizeof( s32 );

                    // write out the offset of the breakpoint data.
                    s32 xBreakPointOffset = File.BreakPointOffset;
                    if( ReverseEndian )
                        xBreakPointOffset = reverse_endian_32( xBreakPointOffset );
                    Final.Append( (const u8*)&xBreakPointOffset, sizeof(s32) );
                    TotalSampleTableSize2 += sizeof( s32 );

                    // Write out compression type.
                    Final.Append( (const u8*)&xCompressionType, sizeof(s32) );
                    TotalSampleTableSize2 += sizeof( s32 );

                    // Write out the header.
                    Final.Append( (const u8*)Data.Headers[ k ], Data.HeaderSize );
                    TotalSampleTableSize2 += Data.HeaderSize;
/*
HeaderSize2 = TotalSampleTableSize2 - HeaderSize2;
x_printf  ( "nChannels: %d, HeaderSize: %3d, HeaderSize2: %3d, File: %s\n", Data.nChannels, Data.HeaderSize, HeaderSize2, File.Filename  );
x_DebugMsg( "nChannels: %d, HeaderSize: %3d, HeaderSize2: %3d, File: %s\n", Data.nChannels, Data.HeaderSize, HeaderSize2, File.Filename  );
*/
                    // Write out the sample
                    if( k < Data.nWaveforms )
                        SampleStream.Append( Data.WaveformData[ k ] );
                }
            }
        }
    }

    // Append the samples to the package stream.
    ASSERT( TotalSampleTableSize == TotalSampleTableSize2 );
    Final.Append( SampleStream );
    
    // Open the output file.
    Out = x_fopen( Filename , "wb" );
    if( !Out )
    {
        x_printf( "Error opening output file!\n", Filename );
        x_DebugMsg( "Error opening output file!\n", Filename );
        return;
    }
    else
    {
        x_fwrite( Final.GetBuffer(), Final.GetLength(), 1, Out );
        x_fclose( Out );
    }
}

//------------------------------------------------------------------------------

void ExportPackage( void )
{
    static char FullFilename[256];

    if( s_Verbose )
    {
        x_printf( "Exporting package.\n"  );
        x_DebugMsg( "Exporting package.\n" );
    }

    ASSERT( s_Targets.GetCount() == 1 );

    // for each target
    for( s32 i=0 ; i<s_Targets.GetCount() ; i++ )
    {
        s32     Target        = s_Targets[i];
        xbool   ReverseEndian = (Target == EXPORT_GCN);
        
        // Construct the file name.
        x_sprintf( FullFilename, "%s.%s", (const char*)s_Package.m_Identifier, PACKAGE_EXTENSION );

        // Export the music data.
        ExportMusicData();

        // Export the lip sync data.
        ExportLipSyncData( Target );

        // Export the break point data.
        ExportBreakPointData( Target );

        // Export the descriptors.
        ExportDescriptors( ReverseEndian );

        // Export the descriptor identifiers.
        ExportDescriptorIdentifiers( ReverseEndian );

        // Export the package header.
        ExportPackageHeader( ReverseEndian, Target );

        // Now write it all to the file.
        WritePackageToDisk( (const char*)s_Package.m_OutputFilename, ReverseEndian, Target );
    }
}
