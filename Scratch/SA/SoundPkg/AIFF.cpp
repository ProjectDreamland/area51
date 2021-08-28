// AIFF.cpp: implementation of the CAIFF class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include <windows.h>
#include <math.h>
#include <mmsystem.h>
#include <assert.h>
#include "AIFF.h"
#include "x_files.hpp"

#if 0
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAIFF::CAIFF()
{
	m_Valid = false ;
	m_ValidCOMM = false ;
	m_ValidSSND = false ;
	m_ValidINST = false ;
	m_ValidMARK = false ;
	m_pMarkers = NULL ;
	m_pSoundData = NULL ;

	m_cForm = 0 ;
	m_cCompression = 0 ;
	m_nChannels = 0 ;
	m_nSampleBits = 0 ;
	m_nFrames = 0 ;
	m_SampleRate = 1000.0 ;
}

CAIFF::~CAIFF()
{
	//---	Free any Data
	if( m_pMarkers ) x_free( m_pMarkers ) ;
	if( m_pSoundData ) x_free( m_pSoundData ) ;
}


///////////////////////////////////////////////////////////////////////
//	GetPlayTime

double CAIFF::GetPlayTime( )
{
	double	Time = 0.0 ;

	if( m_SampleRate > 0 )
		Time = m_nFrames / m_SampleRate ;

	return Time ;
}


///////////////////////////////////////////////////////////////////////
//	GetSoundData

short *CAIFF::GetSoundData( )
{
	return m_pSoundData ;
}

///////////////////////////////////////////////////////////////////////
//	SetSampleRate

void CAIFF::SetSampleRate( double SampleRate )
{
	if( SampleRate < 1000.0 ) SampleRate = 1000.0 ;
	m_SampleRate = SampleRate ;
}

///////////////////////////////////////////////////////////////////////
//	GetSampleRate

double CAIFF::GetSampleRate( )
{
	return m_SampleRate ;
}

///////////////////////////////////////////////////////////////////////
//	GetSample

short CAIFF::GetSample( double Frame )
{
	//---	Get Interpolated Sample otherwise
	unsigned long	i1 = (unsigned long)Frame ;
	unsigned long	i2 = i1 + 1 ;
	double			t = fmod( Frame, 1.0 ) ;
	short			v ;

	//---	Return 0 if Frame outside Range
	if( (i1 < 0.0) || (i1 > GetNumFrames()) )
		return 0 ;

	if( i1 >= GetNumFrames() ) i1 = GetNumFrames() - 1 ;
	if( i2 >= GetNumFrames() ) i2 = GetNumFrames() - 1 ;

	v = short(m_pSoundData[i1] * (1.0-t) + m_pSoundData[i2] * t) ;

	return v ;
}

///////////////////////////////////////////////////////////////////////
//	SetNumFrames

void CAIFF::SetNumFrames( unsigned long NumFrames )
{
	m_pSoundData = (short*)realloc( m_pSoundData, NumFrames * sizeof(short) ) ;
	if( NumFrames > m_nFrames )
	{
		//---	Zero new Data
		for( unsigned long i = m_nFrames ; i < NumFrames ; i++ )
			m_pSoundData[i] = 0 ;
	}
	m_nFrames = NumFrames ;
}

///////////////////////////////////////////////////////////////////////
//	GetNumFrames

unsigned long CAIFF::GetNumFrames( )
{
	return m_nFrames ;
}

///////////////////////////////////////////////////////////////////////
//	Mix

void CAIFF::Mix( CAIFF *pAIFF, double StartTime, double Volume )
{
	unsigned long	StartDstFrame ;
	unsigned long	EndDstFrame ;

	if( StartTime < 0 ) StartTime = 0 ;
	StartDstFrame = (unsigned long)(GetSampleRate() * StartTime) ;
	EndDstFrame = StartDstFrame + (unsigned long)(pAIFF->GetNumFrames( ) * GetSampleRate() / pAIFF->GetSampleRate()) ;

	//---	Ensure there is enough data to merge into
	if( EndDstFrame > GetNumFrames() )
		SetNumFrames( EndDstFrame ) ;

	//---	Merge in the Sample
	short		*pSrc = pAIFF->GetSoundData() ;
	short		*pDst = GetSoundData() + StartDstFrame ;

	//---	Merge in the new sample
	if( pSrc && pDst )
	{
		double			Dst2Src = pAIFF->GetSampleRate() / GetSampleRate() ;
		unsigned long	iDst ;

		for( iDst = StartDstFrame ; iDst < EndDstFrame ; iDst++ )
		{
			long			SrcSample = 0 ;
			long			DstSample = 0 ;
			unsigned long	iSrc = (unsigned long)((iDst - StartDstFrame) * Dst2Src) ;

			//---	Get Src and Dst Samples
			SrcSample = (long)(pAIFF->GetSample( (iDst - StartDstFrame) * Dst2Src ) * Volume) ;
			DstSample = *pDst ;

			//---	Mix them
			DstSample += SrcSample ;
			if( DstSample < -32878 ) DstSample = -32768 ;
			if( DstSample > 32767 ) DstSample = 32767 ;

			//---	Write out Sample
			*pDst++ = (short)DstSample ;
		}
	}
}



///////////////////////////////////////////////////////////////////////
//	Create

bool CAIFF::Create( double SampleRate, unsigned long NumFrames )
{
	SetSampleRate( SampleRate ) ;
	SetNumFrames( NumFrames ) ;
	m_Valid = true ;

	//---	Return Success Code
	return m_Valid ;
}


///////////////////////////////////////////////////////////////////////
//	Create
//
//	Create AIF-C from disk file

bool CAIFF::Create( LPCTSTR Filename )
{
	bool				done = false ;					// done processing

	long				ChunkOffset ;					// Offset of Chunk in File
	AIFFChunkHeader		Header ;						// Chunk Header
	unsigned char		strnLen ;						// Length of String
	int					i ;								// Loop Counter

	//---	Open the file
	m_Filename = Filename ;
	m_pFile = fopen( Filename, "rb" ) ;
	if( m_pFile )
	{
		//---	Read Form Chunk and Chunk for AIFF / AIFC
		ReadChunk( &m_FormChunk, sizeof(AIFFChunk), 1, m_pFile ) ;
		m_cForm = m_FormChunk.formType ;
		if( (m_FormChunk.ckID != IFF_ID_FORM) ||
			((m_cForm != IFF_ID_AIFC) && (m_cForm != IFF_ID_AIFF)) )
		{
			done = true ;
        }

		//---	Scan AIFC file for chunks. Read in the COMMON chunk.
		//		Set-up a pointer to the sound data chunk. Read loop points
		//		from the INST chunk.
		while( !done )
		{
			//---	Read Header for Current Chunk
			if( ReadChunkHeader( &Header, sizeof(AIFFChunkHeader), 1, m_pFile ) < 1 )
			{
				done = true ;
				break ;
			}

			//---	Add the pad byte if necessary to the count
			Header.ckSize  = ++Header.ckSize & ~1 ;

			//---	Record Chunk Position
			ChunkOffset = ftell( m_pFile ) ;

			//---	Read Chunk depending on Header
			switch( Header.ckID )
			{
			//---	Read Common Chunk
			case (IFF_ID_COMM):
				//---	Read Chunk
				if( ReadCommonChunk( &m_CommChunk, sizeof(AIFFCommonChunk), 1, m_pFile ) < 1 )
				{
					done = true ;
					break ;
				}
            
				//---	Get info from Common Chunk
				m_cCompression = (m_CommChunk.compressionTypeH<<16) + m_CommChunk.compressionTypeL ;
				m_nChannels = m_CommChunk.numChannels ;
				m_nSampleBits = m_CommChunk.sampleSize ;
				m_nFrames = ((long)m_CommChunk.numFramesH << 16) | ((long)m_CommChunk.numFramesL & 0xffff) ;
				m_SampleRate = ConvertFromIeeeExtended( m_CommChunk.sampleRate ) ;

				//---	Check for a valid file format
				if( ((m_cForm == IFF_ID_AIFC) && (m_cCompression != 'NONE')) ||
					(m_nChannels != 1) ||
					(m_nSampleBits != 16) )
				{
                    printf("\nCAIFF::Create(): Not a valid file format. Got nChannels=%d, nSampleBits=%d\n",m_nChannels,m_nSampleBits);
					done = true ;
					break ;
				}

				m_ValidCOMM = true ;

				break ;

			//---	Read File Sound Data Chunk
			case (IFF_ID_SSND):
				//---	Read Chunk
				ReadSoundDataChunk( &m_SndDChunk, sizeof (AIFFSoundDataChunk), 1, m_pFile ) ;
				if( (m_SndDChunk.offset != 0) || (m_SndDChunk.blockSize != 0) )
				{
					done = true ;
					break ;
				}

				//---	Load Sound Data
				m_pSoundData = (short*)x_malloc( 2 * m_nFrames) ;
				if( m_pSoundData )
				{
					ReadShort( m_pSoundData, 2, m_nFrames, m_pFile ) ;
				}
				else
				{
					done = true ;
					break ;
				}

				m_ValidSSND = true ;

				break ;

			//---	Markers
			case (IFF_ID_MARK):
				//---	Read Chunk
				ReadShort( &m_nMarkers, sizeof(short), 1, m_pFile ) ;
				m_pMarkers = (AIFFMarker*)x_malloc( sizeof(AIFFMarker) * m_nMarkers ) ;
                assert( m_pMarkers ) ;
				for( i = 0 ; i < m_nMarkers ; i++ )
				{
					//---	Read Marker
					ReadMarker( &m_pMarkers[i], sizeof(AIFFMarker), 1, m_pFile ) ;
					fread( &strnLen, 1, 1, m_pFile ) ;
					fseek( m_pFile, strnLen + 1-(strnLen & 1), SEEK_CUR ) ;
				}

				m_ValidMARK = true ;

				break ;

			//---	Instrument Definition
			case (IFF_ID_INST):
				//---	Read Chunk
				ReadInstrumentChunk( &m_InstChunk, sizeof(AIFFInstrumentChunk), 1, m_pFile ) ;

				m_ValidINST = true ;

				break ;

			//---	Skip Chunks we don't care about
			default:
				break ;
			}

			//---	Seek to next Chunk
			fseek( m_pFile, ChunkOffset + Header.ckSize, SEEK_SET ) ;
		}

		//---	Close File
		fclose( m_pFile ) ;
		m_pFile = NULL ;
	}

	//---	Set Valid Flag
	m_Valid = m_ValidCOMM || m_ValidSSND ;

	//---	Return Success Code
	return m_Valid ;
}
		


bool CAIFF::Play() 
{
	bool		Success = false ;

	//---	Exit if not a valid AIFF
	if( !m_Valid )
		goto End ;

	//---	Setup a WAVEFORMATEX
	WAVEFORMATEX	WaveFormat ;
	WaveFormat.wFormatTag = WAVE_FORMAT_PCM ;
	WaveFormat.nChannels = 1 ;
	WaveFormat.nSamplesPerSec = (unsigned long)m_SampleRate ;
	WaveFormat.wBitsPerSample = 16 ;
	WaveFormat.nBlockAlign = WaveFormat.wBitsPerSample/8 ;
	WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign ;
	WaveFormat.cbSize = 0 ;
	
	//---	Open the Wave Out Device
	HWAVEOUT	HWaveOut ;
	if( waveOutOpen( &HWaveOut, WAVE_MAPPER, &WaveFormat, NULL, NULL, CALLBACK_NULL ) != MMSYSERR_NOERROR )
	{
		//---	Open Failed
		MessageBox( NULL, "Error Opening WaveOutDevice", NULL, MB_ICONSTOP ) ;
	}
	else
	{
		//---	Open Succes, setup WAVEHDR
		WAVEHDR		WaveHeader ;
		WaveHeader.lpData			= (char*)m_pSoundData ;
		WaveHeader.dwBufferLength	= m_nFrames * 2 ;
		WaveHeader.dwFlags			= 0 ;
		if( waveOutPrepareHeader( HWaveOut, &WaveHeader, sizeof(WAVEHDR) ) == MMSYSERR_NOERROR )
		{
			//---	Play Wave
			if( waveOutWrite( HWaveOut, &WaveHeader, sizeof(WAVEHDR) ) == MMSYSERR_NOERROR )
				while( !(WaveHeader.dwFlags & WHDR_DONE) ) ;
		}

		//---	Close Wave Out Device
		waveOutClose( HWaveOut ) ;
	}

	//---	Set Success Code
	Success = true ;

End:
	//---	Return Success Code
	return Success ;
}











		
/****************************************************************
 * Extended precision IEEE floating-point conversion routines
 ****************************************************************/

#define FloatToUnsigned(f)	((unsigned long)(((long)((f) - 2147483648.0)) + 2147483647L + 1))
#define UnsignedToFloat(u)	(((double)((long)((u) - 2147483647L - 1))) + 2147483648.0)

 double CAIFF::ConvertFromIeeeExtended( char *bytes )
{
	double	f;
	long	expon;
	unsigned long hiMant, loMant;

	expon = ((bytes[0] & 0x7F) << 8) | (bytes[1] & 0xFF);
	hiMant	=	((unsigned long)(bytes[2] & 0xFF) << 24)
			|	((unsigned long)(bytes[3] & 0xFF) << 16)
			|	((unsigned long)(bytes[4] & 0xFF) << 8)
			|	((unsigned long)(bytes[5] & 0xFF));
	loMant	=	((unsigned long)(bytes[6] & 0xFF) << 24)
			|	((unsigned long)(bytes[7] & 0xFF) << 16)
			|	((unsigned long)(bytes[8] & 0xFF) << 8)
			|	((unsigned long)(bytes[9] & 0xFF));

	if (expon == 0 && hiMant == 0 && loMant == 0) {
		f = 0;
	}
	else {
		if (expon == 0x7FFF) {	/* Infinity or NaN */
			f = HUGE_VAL;
		}
		else {
			expon -= 16383;
			f  = ldexp(UnsignedToFloat(hiMant), expon-=31);
			f += ldexp(UnsignedToFloat(loMant), expon-=32);
		}
	}

	if (bytes[0] & 0x80)
		return -f;
	else
		return f;
}

void CAIFF::ConvertToIeeeExtended( double num, char *bytes )
{
	int				sign;
	int				expon;
	double			fMant, fsMant;
	unsigned long	hiMant, loMant;

	if (num < 0) {
		sign = 0x8000;
		num *= -1;
	} else {
		sign = 0;
	}

	if (num == 0) {
		expon = 0; hiMant = 0; loMant = 0;
	}
	else {
		fMant = frexp(num, &expon);
		if ((expon > 16384) || !(fMant < 1)) {	/* Infinity or NaN */
			expon = sign|0x7FFF; hiMant = 0; loMant = 0; /* infinity */
		}
		else {	/* Finite */
			expon += 16382;
			if (expon < 0) {	/* denormalized */
				fMant = ldexp(fMant, expon);
				expon = 0;
			}
			expon |= sign;
			fMant = ldexp(fMant, 32);          fsMant = floor(fMant); hiMant = FloatToUnsigned(fsMant);
			fMant = ldexp(fMant - fsMant, 32); fsMant = floor(fMant); loMant = FloatToUnsigned(fsMant);
		}
	}
	
	bytes[0] = (char)(expon >> 8);
	bytes[1] = (char)expon;
	bytes[2] = (char)(hiMant >> 24);
	bytes[3] = (char)(hiMant >> 16);
	bytes[4] = (char)(hiMant >> 8);
	bytes[5] = (char)hiMant;
	bytes[6] = (char)(loMant >> 24);
	bytes[7] = (char)(loMant >> 16);
	bytes[8] = (char)(loMant >> 8);
	bytes[9] = (char)loMant;
}


///////////////////////////////////////////////////////////////////////
//	Custom freads

void CAIFF::swap16( unsigned short &v )
{
	v = ((v << 8) & 0xff00) | ((v >> 8) & 0x00ff) ;
}

void CAIFF::swap32( unsigned long &v )
{
	v = ((v << 24) & 0xff000000) |
		((v << 8 ) & 0x00ff0000) |
		((v >> 8 ) & 0x0000ff00) |
		((v >> 24) & 0x000000ff) ;
}

void CAIFF::swap16( short &v )
{
	swap16( (unsigned short &)v ) ;
}

void CAIFF::swap32( long &v )
{
	swap32( (unsigned long &)v ) ;
}

size_t CAIFF::ReadShort( void *buffer, size_t size, size_t count, FILE *stream )
{
	unsigned short	*pSwap ;
	size_t			NumRead ;
	size_t			i ;

	NumRead = fread( buffer, size, count, stream ) ;

	pSwap = (unsigned short*)buffer ;
	for( i = 0 ; i < count ; i++ )
	{
		swap16( *pSwap ) ;
		pSwap++ ;
	}

	return NumRead ;
}

size_t CAIFF::ReadLong( void *buffer, size_t size, size_t count, FILE *stream )
{
	unsigned long	*pSwap ;
	size_t			NumRead ;
	size_t			i ;

	NumRead = fread( buffer, size, count, stream ) ;

	pSwap = (unsigned long*)buffer ;
	for( i = 0 ; i < count ; i++ )
	{
		swap32( *pSwap ) ;
		pSwap++ ;
	}

	return NumRead ;
}

size_t CAIFF::ReadChunk( void *buffer, size_t size, size_t count, FILE *stream )
{
	AIFFChunk	*pChunk ;
	size_t		NumRead ;
	size_t		i ;

	NumRead = fread( buffer, size, count, stream ) ;

	pChunk = (AIFFChunk*)buffer ;
	for( i = 0 ; i < count ; i++ )
	{
		swap32( pChunk->ckID ) ;
		swap32( pChunk->ckSize ) ;
		swap32( pChunk->formType ) ;
		pChunk++ ;
	}

	return NumRead ;
}

size_t CAIFF::ReadChunkHeader( void *buffer, size_t size, size_t count, FILE *stream )
{
	AIFFChunkHeader	*pChunkHeader ;
	size_t			NumRead ;
	size_t			i ;

	NumRead = fread( buffer, size, count, stream ) ;

	pChunkHeader = (AIFFChunkHeader*)buffer ;
	for( i = 0 ; i < count ; i++ )
	{
		swap32( pChunkHeader->ckID ) ;
		swap32( pChunkHeader->ckSize ) ;
		pChunkHeader++ ;
	}

	return NumRead ;
}

size_t CAIFF::ReadCommonChunk( void *buffer, size_t size, size_t count, FILE *stream )
{
	AIFFCommonChunk	*pCommonChunk ;
	size_t			NumRead ;
	size_t			i ;

	NumRead = fread( buffer, size, count, stream ) ;

	pCommonChunk = (AIFFCommonChunk*)buffer ;
	for( i = 0 ; i < count ; i++ )
	{
		swap16( pCommonChunk->numChannels ) ;
		swap16( pCommonChunk->numFramesH ) ;
		swap16( pCommonChunk->numFramesL ) ;
		swap16( pCommonChunk->sampleSize ) ;
		swap16( pCommonChunk->compressionTypeH ) ;
		swap16( pCommonChunk->compressionTypeL ) ;

		pCommonChunk++ ;
	}

	return NumRead ;
}

size_t CAIFF::ReadInstrumentChunk( void *buffer, size_t size, size_t count, FILE *stream )
{
	AIFFInstrumentChunk	*pInstrumentChunk ;
	size_t				NumRead ;
	size_t				i ;

	NumRead = fread( buffer, size, count, stream ) ;

	pInstrumentChunk = (AIFFInstrumentChunk*)buffer ;
	for( i = 0 ; i < count ; i++ )
	{
		swap16( pInstrumentChunk->gain ) ;
		swap16( pInstrumentChunk->sustainLoop.playMode ) ;
		swap16( pInstrumentChunk->sustainLoop.beginLoop ) ;
		swap16( pInstrumentChunk->sustainLoop.endLoop ) ;
		swap16( pInstrumentChunk->releaseLoop.playMode ) ;
		swap16( pInstrumentChunk->releaseLoop.beginLoop ) ;
		swap16( pInstrumentChunk->releaseLoop.endLoop ) ;
		pInstrumentChunk++ ;
	}

	return NumRead ;
}

size_t CAIFF::ReadSoundDataChunk( void *buffer, size_t size, size_t count, FILE *stream )
{
	AIFFSoundDataChunk	*pSoundDataChunk ;
	size_t				NumRead ;
	size_t				i ;

	NumRead = fread( buffer, size, count, stream ) ;

	pSoundDataChunk = (AIFFSoundDataChunk*)buffer ;
	for( i = 0 ; i < count ; i++ )
	{
		swap32( pSoundDataChunk->offset ) ;
		swap32( pSoundDataChunk->blockSize ) ;
		pSoundDataChunk++ ;
	}

	return NumRead ;
}

size_t CAIFF::ReadMarker( void *buffer, size_t size, size_t count, FILE *stream )
{
	AIFFMarker	*pMarker ;
	size_t		NumRead ;
	size_t		i ;

	NumRead = fread( buffer, size, count, stream ) ;

	pMarker = (AIFFMarker*)buffer ;
	for( i = 0 ; i < count ; i++ )
	{
		swap16( pMarker->id ) ;
		swap16( pMarker->positionH ) ;
		swap16( pMarker->positionL ) ;
		pMarker++ ;
	}

	return NumRead ;
}
		
AIFFInstrumentChunk *CAIFF::GetInstrumentChunk(void)
{
    if (m_ValidINST)
        return &m_InstChunk;
    else
        return NULL;
}

AIFFMarker *CAIFF::GetMarker( long MarkerId)
{
    if (MarkerId < m_nMarkers)
        return &m_pMarkers[MarkerId];
    else
        return NULL;
}