#include "x_types.hpp"
#include "decode.hpp"
#include "aiff.h"
#include "x_files.hpp"
#include "../../support/audiomgr/audiodefs.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <msacm.h>


#define SECTOR_LENGTH       (2048)
#define SAMPLES_PER_CHUNK   (28)
#define CHUNK_SIZE          (16)

#define SAMPLES_PER_SECTOR  (SECTOR_LENGTH / CHUNK_SIZE * SAMPLES_PER_CHUNK)

void InterleaveData(s16 *pSrc,s32 Length, s32 nChannels,t_DecodeHeader *pHeader)
{
        s16 *pIn;
        s16 *pLeft,*pRight;

        printf("Warning: Sample is stereo. Interleaving....");
        if (nChannels != 2)
        {
            printf("Error: Sample has way too many channels to deal with\n");
            exit(-1);
        }
        Length/=4;
        pHeader->pLeft = pSrc;
        pHeader->pRight = (s16 *)x_malloc( Length* sizeof(s16) + 4096);
        ASSERT(pHeader->pRight);

        pIn = pSrc;
        pLeft = pIn;
        pRight = pHeader->pRight;
        while (Length)
        {
            *pLeft++=*pIn++;
            *pRight++=*pIn++;
            Length--;
        }

        printf("Done.\n");
}
//-----------------------------------------------------------------------------
void *ConvertToRAW( WAVEFORMATEX* pSrcFormat, void* pSrcData, LONG SrcDataSize, t_DecodeHeader *pHeader )
{
    WAVEFORMATEX dstFormat;

    dstFormat.wFormatTag      = WAVE_FORMAT_PCM;
    dstFormat.nChannels       = pSrcFormat->nChannels;
    dstFormat.nSamplesPerSec  = pSrcFormat->nSamplesPerSec;
    dstFormat.nAvgBytesPerSec = (16 * dstFormat.nChannels) / 8 * dstFormat.nSamplesPerSec ;
    dstFormat.nBlockAlign     = (16 * dstFormat.nChannels) / 8;
    dstFormat.wBitsPerSample  = 16;
    dstFormat.cbSize          = 0;

    HACMSTREAM  hs;
    DWORD       DstDataSize;
    LPBYTE      pDstData;

    // Open Conversion Stream
    MMRESULT res = acmStreamOpen( &hs, NULL, pSrcFormat, &dstFormat, NULL, 0, 0, 0 );
    if( res != 0 )
    {
        printf( "acmStreamOpen error\n" );
        return NULL;
    }

    // Get Size of Destination Buffer
    res = acmStreamSize( hs, SrcDataSize, &DstDataSize, ACM_STREAMSIZEF_SOURCE );
    if( res != 0 )
    {
        printf( "acmStreamSize error\n" );
        return NULL;
    }

    // Allocate Buffer
    pDstData = (LPBYTE)x_malloc( DstDataSize*sizeof(s16)+4096);

    // Prepare the Header
    ACMSTREAMHEADER ash;
    ash.cbStruct    = sizeof(ACMSTREAMHEADER);
    ash.fdwStatus   = 0;
    ash.dwUser      = 0;
    ash.pbSrc       = (LPBYTE)pSrcData;
    ash.cbSrcLength = SrcDataSize;
    ash.dwSrcUser   = 0;
    ash.pbDst       = pDstData;
    ash.cbDstLength = DstDataSize;
    ash.dwDstUser   = 0;
    res = acmStreamPrepareHeader( hs, &ash, 0 );
    if( res != 0 )
    {
        printf( "acmStreamPrepareHeader error\n" );
        x_free( pDstData );
        return NULL;
    }

    // Convert
    res = acmStreamConvert( hs, &ash, ACM_STREAMCONVERTF_START|ACM_STREAMCONVERTF_BLOCKALIGN );
    if( res != 0 )
    {
        printf( "acmStreamConvert error\n" );
        x_free( pDstData );
        return NULL;
    }

    // Unprepare Header
    res = acmStreamUnprepareHeader( hs, &ash, 0 );
    if( res != 0 )
    {
        printf( "acmStreamUnprepareHeader error\n" );
        x_free( pDstData );
        return NULL;
    }

    // Close Conversion Stream
    res = acmStreamClose( hs, 0 );
    if( res != 0 )
    {
        printf( "acmStreamClose error\n" );
        x_free( pDstData );
        return NULL;
    }

    dstFormat.nChannels       = pSrcFormat->nChannels;
    if (pSrcFormat->nChannels != 1)
    {
        pHeader->Flags |= AUDFLAG_STEREO;
        InterleaveData((s16*)pDstData,DstDataSize,pSrcFormat->nChannels,pHeader);
    }
    else
    {
        pHeader->pLeft = (s16 *)pDstData;
    }
    // Return ptr to dest data and data size
    pHeader->Length = DstDataSize;
    pHeader->SampleRate = pSrcFormat->nSamplesPerSec;
    return pDstData;
}

//-----------------------------------------------------------------------------
void *DecodeAiffToPcm(char *pFilename,t_DecodeHeader *pHeader)
{
    CAIFF   *aiff;
    s16     *pData;
    AIFFInstrumentChunk *pInstr;

    aiff = new CAIFF;

    if (!aiff->Create(pFilename))
    {
        delete aiff;
        return NULL;
    }
    pHeader->SampleRate = (s32)aiff->GetSampleRate();
    pHeader->Length = aiff->GetNumFrames() * sizeof(s16);

    pData = (s16 *)x_malloc(pHeader->Length+4096);
    if (!pData)
    {
        delete aiff;
        return NULL;
    }
    memcpy(pData,aiff->GetSoundData(),pHeader->Length);
    pInstr = aiff->GetInstrumentChunk();
    if (pInstr)
    {
        AIFFMarker *pStartMarker,*pEndMarker;

        pStartMarker = aiff->GetMarker(pInstr->sustainLoop.beginLoop);
        pEndMarker = aiff->GetMarker(pInstr->sustainLoop.endLoop);
        if ( !pStartMarker || !pEndMarker)
        {
            pHeader->LoopStart = 0;
            pHeader->LoopEnd = 0;
        }
        else
        {
            pHeader->LoopStart = (pStartMarker->positionH * 65536 + pStartMarker->positionL)*2;
            pHeader->LoopEnd   = (pEndMarker->positionH * 65536 + pEndMarker->positionL)*2;
        }
    }
    else
    {
        pHeader->LoopStart = 0;
        pHeader->LoopEnd = 0;
    }

    delete aiff;
    return pData;
}

void DecodeToPcm( char *pFilename,t_DecodeHeader *pHeader)
{
    HMMIO           hmmio;              // file handle for open file 
    MMCKINFO        mmckinfoParent;     // parent chunk information 
    MMCKINFO        mmckinfoSubchunk;   // subchunk information structure 
    LONG            dwFmtSize;          // size of "FMT" chunk 
    LONG            dwDataSize;         // size of "DATA" chunk 
    WAVEFORMATEX*   pFormat;           // address of "FMT" chunk 
    HPSTR           lpData;             // address of "DATA" chunk 
    FILE            *fp;
    char            buffer[4];
    
    fp = fopen(pFilename,"rb");
    pHeader->pLeft = NULL;
    pHeader->pRight = NULL;
    if (!fp)
    {
        printf( "ERROR: File not found - %s\n",pFilename);
        return;
    }
    fread(buffer,1,4,fp);
    fclose(fp);
    if ( (buffer[0]=='F') && (buffer[1]=='O') && 
         (buffer[2]=='R') && (buffer[3]=='M') )
    {
        pHeader->pLeft = (s16 *)DecodeAiffToPcm(pFilename,pHeader);
        return;
    }

    // Open Wave File
    hmmio = mmioOpen( pFilename, NULL, MMIO_READ | MMIO_ALLOCBUF );
    if( hmmio == NULL )
    {
        printf( "Error opening file\n" );
        return;
    }

    // Locate a "RIFF" chunk with a "WAVE" form type to make 
    // sure the file is a waveform-audio file. 
    mmckinfoParent.fccType = mmioFOURCC( 'W', 'A', 'V', 'E' ); 
    if( mmioDescend( hmmio, (LPMMCKINFO) &mmckinfoParent, NULL, MMIO_FINDRIFF ) ) 
    { 
        printf( "This is not a waveform-audio file." ); 
        mmioClose( hmmio, 0 ); 
        return; 
    } 

    // Find the "FMT" chunk (form type "FMT"); it must be 
    // a subchunk of the "RIFF" chunk. 
    mmckinfoSubchunk.ckid = mmioFOURCC( 'f', 'm', 't', ' ' ); 
    if( mmioDescend( hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK ) ) 
    { 
        printf( "Waveform-audio file has no FMT chunk." ); 
        mmioClose( hmmio, 0 ); 
        return; 
    } 
 
    // Get the size of the "FMT" chunk. Allocate 
    // and lock memory for it. 
    dwFmtSize = mmckinfoSubchunk.cksize; 
    pFormat = (WAVEFORMATEX*)x_malloc( dwFmtSize + 4096 );

    // Read the "FMT" chunk. 
    if( mmioRead( hmmio, (HPSTR)pFormat, dwFmtSize ) != dwFmtSize )
    { 
        printf( "Failed to read format chunk." ); 
        x_free( pFormat );
        mmioClose( hmmio, 0 ); 
        return; 
    } 
 
    // Ascend out of the "FMT" subchunk. 
    mmioAscend( hmmio, &mmckinfoSubchunk, 0 ); 
 
    // Find the data subchunk. The current file position should be at 
    // the beginning of the data chunk; however, you should not make 
    // this assumption. Use mmioDescend to locate the data chunk. 
    mmckinfoSubchunk.ckid = mmioFOURCC( 'd', 'a', 't', 'a' );
    if( mmioDescend( hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK ) )
    {
        printf("Waveform-audio file has no data chunk.");
        x_free( pFormat );
        mmioClose( hmmio, 0 );
        return; 
    } 
 
    // Get the size of the data subchunk. 
    dwDataSize = mmckinfoSubchunk.cksize;
    if( dwDataSize == 0L )
    {
        printf("The data chunk contains no data.");
        x_free( pFormat );
        mmioClose( hmmio, 0 );
        return; 
    } 
 
    // Allocate and lock memory for the waveform-audio data. 
    // Allocate some extra just in case it's a stereo sample
    // since they require some extra at the end.
    lpData = (char*)x_malloc( dwDataSize + 4096);
 
    // Read the waveform-audio data subchunk. 
    if( mmioRead( hmmio, (HPSTR) lpData, dwDataSize ) != dwDataSize )
    { 
        printf("Failed to read data chunk.");
        x_free( lpData );
        x_free( pFormat );
        mmioClose( hmmio, 0 );
        return; 
    }

    // Convert Data to RAW Wave
    pHeader->SampleRate = (s16)pFormat->nSamplesPerSec;
    ConvertToRAW( pFormat, lpData, dwDataSize, pHeader );

    // free Data Buffer
    x_free( lpData );

    // free Format Buffer
    x_free( pFormat );

    // Close Wave File
    mmioClose( hmmio, 0 );

}
