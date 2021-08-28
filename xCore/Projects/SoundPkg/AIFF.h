// AIFF.h: interface for the CAIFF class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AIFF_H__4FB4ACB7_8AA6_11D1_9507_00207811EE70__INCLUDED_)
#define AFX_AIFF_H__4FB4ACB7_8AA6_11D1_9507_00207811EE70__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
#include <stdio.h>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// AIFF Structures

#define IFF_ID_FORM             'FORM'
#define IFF_ID_AIFC             'AIFC'
#define IFF_ID_AIFF             'AIFF'
#define IFF_ID_COMM             'COMM'
#define IFF_ID_SSND             'SSND'
#define IFF_ID_INST             'INST'
#define IFF_ID_APPL             'APPL'
#define IFF_ID_MARK             'MARK'

#define CODE_NAME               "VADPCMCODES"
#define LOOP_NAME               "VADPCMLOOPS"

#define NoLooping               0
#define ForwardLooping          1
#define ForwardBackwardLooping  2

typedef struct
{
    unsigned long       ckID ;
    long                ckSize ;
} AIFFChunkHeader ;

typedef struct
{
    unsigned long       ckID ;
    long                ckSize ;
    unsigned long       formType ;
} AIFFChunk ;

typedef struct
{
    short               numChannels ;
    unsigned short      numFramesH ;				// To Prevent 4 byte alignment
    unsigned short      numFramesL ;
    short               sampleSize ;
    char                sampleRate[10] ;
    unsigned short      compressionTypeH ;
    unsigned short      compressionTypeL ;			// NOTE: String Follows
} AIFFCommonChunk ;

typedef short AIFFMarkerID ;

typedef struct
{
    AIFFMarkerID        id ;
    unsigned short      positionH ;
    unsigned short      positionL ;					// NOTE: String Follows
} AIFFMarker ;

typedef struct
{
    short               playMode ;
    AIFFMarkerID        beginLoop ;
    AIFFMarkerID        endLoop ;
} AIFFLoop ;

typedef struct
{
    char                baseNote ;
    char                detune ;
    char                lowNote ;
    char                highNote ;
    char                lowVelocity ;
    char                highVelocity ;
    short               gain ;
    AIFFLoop            sustainLoop ;
    AIFFLoop            releaseLoop ;
} AIFFInstrumentChunk ;

typedef struct
{
    unsigned long       offset ;
    unsigned long       blockSize ;
} AIFFSoundDataChunk ;

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//---	CAIFF Class
class CAIFF// : public CObject  
{
//---	Interface
public:
	CAIFF();
	virtual ~CAIFF();

	bool Create( double SampleRate, unsigned long NumFrames = 0 ) ;			// Create Blank
	bool Create( const char *Filename ) ;										// Create from Disk File
	bool Play() ;															// Play AIFF

	double GetPlayTime() ;													// Get play Time in Seconds
	short *GetSoundData() ;													// Get Pointer to Sound Data
	void SetSampleRate( double SampleRate ) ;								// Set the Sample Rate
	double GetSampleRate( ) ;												// Get the Sample Rate
	void SetNumFrames( unsigned long NumFrames ) ;							// Set the number of Frames
	unsigned long GetNumFrames( ) ;											// Get the number of Frames
	short GetSample( double Frame ) ;										// Get Interpolated Sample
    AIFFInstrumentChunk *GetInstrumentChunk(void);                       // Get instrument chunk
    AIFFMarker *GetMarker( long MarkerId);

	void Mix( CAIFF *pAIFF, double StartTime = 0.0, double Volume = 1.0 ) ;	// Mix in another Sample

//---	Private Functions
private:
	double ConvertFromIeeeExtended( char *bytes ) ;
	void  ConvertToIeeeExtended( double num, char *bytes ) ;
	void  swap16( unsigned short &v ) ;
	void  swap32( unsigned long &v ) ;
	void  swap16( short &v ) ;
	void  swap32( long &v ) ;
	size_t ReadShort( void *buffer, size_t size, size_t count, FILE *stream ) ;
	size_t ReadLong( void *buffer, size_t size, size_t count, FILE *stream ) ;
	size_t ReadChunk( void *buffer, size_t size, size_t count, FILE *stream ) ;
	size_t ReadChunkHeader( void *buffer, size_t size, size_t count, FILE *stream ) ;
	size_t ReadCommonChunk( void *buffer, size_t size, size_t count, FILE *stream ) ;
	size_t ReadInstrumentChunk( void *buffer, size_t size, size_t count, FILE *stream ) ;
	size_t ReadSoundDataChunk( void *buffer, size_t size, size_t count, FILE *stream ) ;
	size_t ReadMarker( void *buffer, size_t size, size_t count, FILE *stream ) ;


//---	Data
private:
	bool				m_Valid ;						// Valid AIFF
	bool				m_ValidCOMM ;					// COMM chunk valid
	bool				m_ValidSSND ;					// SSND chunk valid
	bool				m_ValidINST ;					// INST chunk valid
	bool				m_ValidMARK ;					// MARK chunk valid

	const char          *m_Filename ;					// Filename of associated file
	FILE				*m_pFile ;						// File Handle

    AIFFChunk			m_FormChunk ;					// Form Chunk (1st in file)
    AIFFCommonChunk		m_CommChunk ;					// Common Chunk
    AIFFSoundDataChunk	m_SndDChunk ;					// Sound Data Chunk
    AIFFInstrumentChunk	m_InstChunk ;					// Instrument Chunk
	short				m_nMarkers ;					// Number of Markers
    AIFFMarker			*m_pMarkers ;					// Markers

	unsigned long		m_cForm ;						// Form String ;
	unsigned long		m_cCompression ;				// Compression String ;
	short				m_nChannels ;					// Number of Channels
	short				m_nSampleBits ;					// Number of Bits in Samples
	unsigned long		m_nFrames ;						// Number of Frames of Data
	short				*m_pSoundData ;					// Actual Sound Data Pointer
	double				m_SampleRate ;					// Sample Rate (Hz)
} ;

#endif // !defined(AFX_AIFF_H__4FB4ACB7_8AA6_11D1_9507_00207811EE70__INCLUDED_)
