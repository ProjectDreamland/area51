// This is the main project file for VC++ application project 
// generated using an Application Wizard.

#include "stdafx.h"

//==============================================================================
//  Defines
//==============================================================================

#define MAX_STRING_LENGTH   256
#define WORK_START_STRING   "Work_Start"
#define WORK_END_STRING     "Work_End"
#define TRACKS_STRING       "Tracks"
#define TRACK_STRING        "Track"
#define VIDEO_STRING        "Video"
#define TRACK_RECORD_STRING "Track_Record"
#define CLIP_ID_STRING      "ClipID"
#define START_STRING        "Start"
#define END_STRING          "End"
#define FX_STRING           "FX"
#define FX_OPTIONS_STRING   "FX_Options"
#define FX_TYPE_STRING      "FX_Type"
#define CORNERS_STRING      "Corners"
#define DIRECTION_STRING    "Direction"
#define DISSOLVE_STRING     "DISS"
#define AUDIO_STRING        "Audio"
#define MAPPING_STRING      "Mapping"
#define BAND_POINT_STRING   "Band_Point"
#define H_STRING            "h"
#define V_STRING            "v"
#define RUBBER_BAND_STRING  "RubberBand"
#define MAX_STRING          "Max"
#define PAN_BAND_STRING     "PanBand"
#define CLIPS_STRING        "Clips"
#define FILE_ID_STRING      "FileID"
#define IN_STRING           "In"
#define OUT_STRING          "Out"
#define FILES_STRING        "Files"
#define DOS_FILE_STRING     "DOS_File"
#define NUM_FRAMES_STRING   "Num_Frames"
#define WIDTH_STRING        "Width"
#define HEIGHT_STRING       "Height"
#define DEPTH_STRING        "Depth"
#define COLOR_STRING        "Color"
#define RED_STRING          "Red"
#define GREEN_STRING        "Green"
#define BLUE_STRING         "Blue"
#define RATE_STRING         "Rate"

//==============================================================================
//  Class types
//==============================================================================

class premiere_data
{
public:
             premiere_data( void );
    virtual ~premiere_data( void );

    bool    Load( const char* pFilename );

    enum
    {
        TOKEN_UNKNOWN = 0,
        TOKEN_LBRACKET,
        TOKEN_RBRACKET,
        TOKEN_STRING,
        TOKEN_INT,
        TOKEN_COMMA,
        TOKEN_EQUALS,
        TOKEN_POUND,
        TOKEN_WORK_START,
        TOKEN_WORK_END,
        TOKEN_TRACKS,
        TOKEN_TRACK,
        TOKEN_VIDEO,
        TOKEN_FX,
        TOKEN_TRACK_RECORD,
        TOKEN_CLIP_ID,
        TOKEN_START,
        TOKEN_END,
        TOKEN_FX_OPTIONS,
        TOKEN_FX_TYPE,
        TOKEN_CORNERS,
        TOKEN_DIRECTION,
        TOKEN_DISSOLVE,
        TOKEN_AUDIO,
        TOKEN_MAPPING,
        TOKEN_BAND_POINT,
        TOKEN_H,
        TOKEN_V,
        TOKEN_RUBBER_BAND,
        TOKEN_MAX,
        TOKEN_PAN_BAND,
        TOKEN_CLIPS,
        TOKEN_FILE_ID,
        TOKEN_IN,
        TOKEN_OUT,
        TOKEN_FILES,
        TOKEN_DOS_FILE,
        TOKEN_NUM_FRAMES,
        TOKEN_WIDTH,
        TOKEN_HEIGHT,
        TOKEN_DEPTH,
        TOKEN_COLOR,
        TOKEN_RED,
        TOKEN_GREEN,
        TOKEN_BLUE,
        TOKEN_RATE,
        TOKEN_EOF
    };

    enum
    {
        TRACK_UNKNOWN = 0,
        TRACK_VIDEO,
        TRACK_FX,
        TRACK_AUDIO
    };

    enum
    {
        FX_TYPE_UNKNOWN = 0,
        FX_TYPE_DISSOLVE,
    };

    struct p_token
    {
        int         GetType     ( void ) const { return Type; }
        int         GetInt      ( void ) const { return IntValue; }
        const char* GetString   ( void ) const { return StringValue; }

        int     Type;
        int     IntValue;
        char    StringValue[MAX_STRING_LENGTH+1];
    };

    struct p_record
    {
        int     ClipID;
        int     Start;
        int     End;

        int     FXCorners;
        int     FXDirection;
        int     FXStart;
        int     FXEnd;
        int     FXType;
    };

    struct p_track
    {
        int         Type;
        int         nRecords;
        p_record*   pRecords;

        int         AudioMapping;
    };

    struct p_clip
    {
        int         FileID;
        int         InValue;
        int         OutValue;
    };

    struct p_file
    {
        char        FileName[MAX_STRING_LENGTH+1];
        int         nFrames;
        int         Width;
        int         Height;
        int         Depth;
        int         Red;
        int         Green;
        int         Blue;
        int         Rate;
    };

    void    Kill                ( void );
    void    ReadToken           ( void );
    bool    ParseVideoTrack     ( int TrackID );
    bool    ParseFXOptions      ( int TrackID, int RecordID );
    bool    ParseFXTrack        ( int TrackID );
    bool    ParseBandPoints     ( int TrackID, int RecordID );
    bool    ParseAudioOptions   ( int TrackID, int RecordID );
    bool    ParseAudioTrack     ( int TrackID );
    bool    ParseTrack          ( int TrackID );
    bool    ParseTracks         ( void );
    bool    ParseClips          ( void );
    bool    ParseFileOptions    ( int FileID );
    bool    ParseFiles          ( void );
    bool    ParseData           ( void );

    void    ReallocRecords      ( int TrackID, int nRecords );
    void    ReallocTracks       ( int nTracks );
    void    ReallocClips        ( int nClips );
    void    ReallocFiles        ( int nFiles );

    char        m_Version[MAX_STRING_LENGTH+1];
    int         m_WorkStart;
    int         m_WorkEnd;

    int         m_FileSize;
    char*       m_FileBuffer;
    char*       m_CurrPtr;
    p_token     m_Token;

    int         m_nTracks;
    p_track*    m_pTracks;

    int         m_nClips;
    p_clip*     m_pClips;

    int         m_nFiles;
    p_file*     m_pFiles;
};

//==============================================================================

struct slide_data
{
    char    FileName[MAX_STRING_LENGTH+1];
    int     StartFadeIn;
    int     EndFadeIn;
    int     StartFadeOut;
    int     EndFadeOut;
    float   Red;
    float   Green;
    float   Blue;
};

//==============================================================================
//  Implementation
//==============================================================================

premiere_data::premiere_data( void ) :
    m_WorkStart     ( 0 ),
    m_WorkEnd       ( 0 ),
    m_FileSize      ( 0 ),
    m_FileBuffer    ( NULL ),
    m_CurrPtr       ( 0 ),
    m_nTracks       ( 0 ),
    m_pTracks       ( NULL ),
    m_nClips        ( 0 ),
    m_pClips        ( NULL ),
    m_nFiles        ( 0 ),
    m_pFiles        ( NULL )
{
    m_Version[0] = '\0';
}

//==============================================================================

premiere_data::~premiere_data( void )
{
    Kill();
}

//==============================================================================

void premiere_data::Kill( void )
{
    int i;

    // release allocated file buffer data
    if( m_FileBuffer )
        free( m_FileBuffer );

    // release allocated records
    for( i = 0; i < m_nTracks; i++ )
    {
        if( m_pTracks[i].pRecords )
            free( m_pTracks[i].pRecords );
    }

    // release allocated tracks
    if( m_pTracks )
        free( m_pTracks );

    // release allocated clips
    if( m_pClips )
        free( m_pClips );

    // release allocated files
    if( m_pFiles )
        free( m_pFiles );

    // re-initialize everything
    m_WorkStart  = 0;
    m_WorkEnd    = 0;
    m_FileSize   = 0;
    m_FileBuffer = NULL;
    m_CurrPtr    = NULL;
    m_Version[0] = '\0';
    m_nTracks    = 0;
    m_pTracks    = NULL;
    m_nClips     = 0;
    m_pClips     = NULL;
    m_nFiles     = 0;
    m_pFiles     = NULL;
}

//==============================================================================

bool premiere_data::Load( const char* pFilename )
{
    // kill any old data
    Kill();

    // open the file for reading
    FILE* fh;
    fh = fopen( pFilename, "rb" );
    if( !fh )
    {
        return false;
    }

    // read the file into a memory, buffer, then close it
    fseek( fh, 0, SEEK_END );
    m_FileSize = ftell( fh );
    fseek( fh, 0, SEEK_SET );
    m_FileBuffer = new char[m_FileSize];
    fread( m_FileBuffer, 1, m_FileSize, fh );
    fclose( fh );

    // now parse the data
    bool Result = ParseData();
    if( !Result )
    {
        printf("ERROR parsing EDL file");
    }

    return Result;
}

//==============================================================================

void premiere_data::ReadToken( void )
{
    // skip over any spaces, or end of lines
    while( m_CurrPtr < (m_FileBuffer+m_FileSize) )
    {
        if( *m_CurrPtr == ' ' ||
            *m_CurrPtr == '\n' ||
            *m_CurrPtr == '\r' ||
            *m_CurrPtr == '\t' ||
            *m_CurrPtr == '\0' )
        {
            m_CurrPtr++;
        }
        else
        {
            break;
        }
    }
    m_Token.Type = TOKEN_UNKNOWN;

    // have we reached the end of the file?
    if( m_CurrPtr >= (m_FileBuffer + m_FileSize) )
    {
        m_Token.Type = TOKEN_EOF;
    }
    // do the quick ones first
    else if( *m_CurrPtr == '[' )
    {
        m_Token.Type = TOKEN_LBRACKET;
        m_CurrPtr++;
    }
    else if( *m_CurrPtr == ']' )
    {
        m_Token.Type = TOKEN_RBRACKET;
        m_CurrPtr++;
    }
    else if( *m_CurrPtr == '=' )
    {
        m_Token.Type = TOKEN_EQUALS;
        m_CurrPtr++;
    }
    else if( *m_CurrPtr == '#' )
    {
        m_Token.Type = TOKEN_POUND;
        m_CurrPtr++;
    }
    else if( *m_CurrPtr == ',' )
    {
        m_Token.Type = TOKEN_COMMA;
        m_CurrPtr++;
    }
    else if( !strncmp( m_CurrPtr, WORK_START_STRING, strlen(WORK_START_STRING) ) )
    {
        m_Token.Type = TOKEN_WORK_START;
        m_CurrPtr += strlen(WORK_START_STRING);
    }
    else if( !strncmp( m_CurrPtr, WORK_END_STRING, strlen(WORK_END_STRING) ) )
    {
        m_Token.Type = TOKEN_WORK_END;
        m_CurrPtr += strlen(WORK_END_STRING);
    }
    else if( !strncmp( m_CurrPtr, TRACK_RECORD_STRING, strlen(TRACK_RECORD_STRING) ) )
    {
        m_Token.Type = TOKEN_TRACK_RECORD;
        m_CurrPtr += strlen(TRACK_RECORD_STRING);
    }
    else if( !strncmp( m_CurrPtr, TRACKS_STRING, strlen(TRACKS_STRING) ) )
    {
        m_Token.Type = TOKEN_TRACKS;
        m_CurrPtr += strlen(TRACKS_STRING);
    }
    else if( !strncmp( m_CurrPtr, TRACK_STRING, strlen(TRACK_STRING) ) )
    {
        m_Token.Type = TOKEN_TRACK;
        m_CurrPtr += strlen(TRACK_STRING);
    }
    else if( !strncmp( m_CurrPtr, VIDEO_STRING, strlen(VIDEO_STRING) ) )
    {
        m_Token.Type = TOKEN_VIDEO;
        m_CurrPtr += strlen(VIDEO_STRING);
    }
    else if( !strncmp( m_CurrPtr, CLIP_ID_STRING, strlen(CLIP_ID_STRING) ) )
    {
        m_Token.Type = TOKEN_CLIP_ID;
        m_CurrPtr += strlen(CLIP_ID_STRING);
    }
    else if( !strncmp( m_CurrPtr, START_STRING, strlen(START_STRING) ) )
    {
        m_Token.Type = TOKEN_START;
        m_CurrPtr += strlen(START_STRING);
    }
    else if( !strncmp( m_CurrPtr, END_STRING, strlen(END_STRING) ) )
    {
        m_Token.Type = TOKEN_END;
        m_CurrPtr += strlen(END_STRING);
    }
    else if( !strncmp( m_CurrPtr, FX_OPTIONS_STRING, strlen(FX_OPTIONS_STRING) ) )
    {
        m_Token.Type = TOKEN_FX_OPTIONS;
        m_CurrPtr += strlen(FX_OPTIONS_STRING);
    }
    else if( !strncmp( m_CurrPtr, FX_TYPE_STRING, strlen(FX_TYPE_STRING) ) )
    {
        m_Token.Type = TOKEN_FX_TYPE;
        m_CurrPtr += strlen(FX_TYPE_STRING);
    }
    else if( !strncmp( m_CurrPtr, FX_STRING, strlen(FX_STRING) ) )
    {
        m_Token.Type = TOKEN_FX;
        m_CurrPtr += strlen(FX_STRING);
    }
    else if( !strncmp( m_CurrPtr, CORNERS_STRING, strlen(CORNERS_STRING) ) )
    {
        m_Token.Type = TOKEN_CORNERS;
        m_CurrPtr += strlen(CORNERS_STRING);
    }
    else if( !strncmp( m_CurrPtr, DIRECTION_STRING, strlen(DIRECTION_STRING) ) )
    {
        m_Token.Type = TOKEN_DIRECTION;
        m_CurrPtr += strlen(DIRECTION_STRING);
    }
    else if( !strncmp( m_CurrPtr, DISSOLVE_STRING, strlen(DISSOLVE_STRING) ) )
    {
        m_Token.Type = TOKEN_DISSOLVE;
        m_CurrPtr += strlen(DISSOLVE_STRING);
    }
    else if( !strncmp( m_CurrPtr, AUDIO_STRING, strlen(AUDIO_STRING) ) )
    {
        m_Token.Type = TOKEN_AUDIO;
        m_CurrPtr += strlen(AUDIO_STRING);
    }
    else if( !strncmp( m_CurrPtr, MAPPING_STRING, strlen(MAPPING_STRING) ) )
    {
        m_Token.Type = TOKEN_MAPPING;
        m_CurrPtr += strlen(MAPPING_STRING);
    }
    else if( !strncmp( m_CurrPtr, BAND_POINT_STRING, strlen(BAND_POINT_STRING) ) )
    {
        m_Token.Type = TOKEN_BAND_POINT;
        m_CurrPtr += strlen(BAND_POINT_STRING);
    }
    else if( !strncmp( m_CurrPtr, RUBBER_BAND_STRING, strlen(RUBBER_BAND_STRING) ) )
    {
        m_Token.Type = TOKEN_RUBBER_BAND;
        m_CurrPtr += strlen(RUBBER_BAND_STRING);
    }
    else if( !strncmp( m_CurrPtr, MAX_STRING, strlen(MAX_STRING) ) )
    {
        m_Token.Type = TOKEN_MAX;
        m_CurrPtr += strlen(MAX_STRING);
    }
    else if( !strncmp( m_CurrPtr, PAN_BAND_STRING, strlen(PAN_BAND_STRING) ) )
    {
        m_Token.Type = TOKEN_PAN_BAND;
        m_CurrPtr += strlen(PAN_BAND_STRING);
    }
    else if( !strncmp( m_CurrPtr, CLIPS_STRING, strlen(CLIPS_STRING) ) )
    {
        m_Token.Type = TOKEN_CLIPS;
        m_CurrPtr += strlen(CLIPS_STRING);
    }
    else if( !strncmp( m_CurrPtr, FILE_ID_STRING, strlen(FILE_ID_STRING) ) )
    {
        m_Token.Type = TOKEN_FILE_ID;
        m_CurrPtr += strlen(FILE_ID_STRING);
    }
    else if( !strncmp( m_CurrPtr, FILES_STRING, strlen(FILES_STRING) ) )
    {
        m_Token.Type = TOKEN_FILES;
        m_CurrPtr += strlen(FILES_STRING);
    }
    else if( !strncmp( m_CurrPtr, DOS_FILE_STRING, strlen(DOS_FILE_STRING) ) )
    {
        m_Token.Type = TOKEN_DOS_FILE;
        m_CurrPtr += strlen(DOS_FILE_STRING);
    }
    else if( !strncmp( m_CurrPtr, NUM_FRAMES_STRING, strlen(NUM_FRAMES_STRING) ) )
    {
        m_Token.Type = TOKEN_NUM_FRAMES;
        m_CurrPtr += strlen(NUM_FRAMES_STRING);
    }
    else if( !strncmp( m_CurrPtr, WIDTH_STRING, strlen(WIDTH_STRING) ) )
    {
        m_Token.Type = TOKEN_WIDTH;
        m_CurrPtr += strlen(WIDTH_STRING);
    }
    else if( !strncmp( m_CurrPtr, HEIGHT_STRING, strlen(HEIGHT_STRING) ) )
    {
        m_Token.Type = TOKEN_HEIGHT;
        m_CurrPtr += strlen(HEIGHT_STRING);
    }
    else if( !strncmp( m_CurrPtr, DEPTH_STRING, strlen(DEPTH_STRING) ) )
    {
        m_Token.Type = TOKEN_DEPTH;
        m_CurrPtr += strlen(DEPTH_STRING);
    }
    else if( !strncmp( m_CurrPtr, COLOR_STRING, strlen(COLOR_STRING) ) )
    {
        m_Token.Type = TOKEN_COLOR;
        m_CurrPtr += strlen(COLOR_STRING);
    }
    else if( !strncmp( m_CurrPtr, RED_STRING, strlen(RED_STRING) ) )
    {
        m_Token.Type = TOKEN_RED;
        m_CurrPtr += strlen(RED_STRING);
    }
    else if( !strncmp( m_CurrPtr, GREEN_STRING, strlen(GREEN_STRING) ) )
    {
        m_Token.Type = TOKEN_GREEN;
        m_CurrPtr += strlen(GREEN_STRING);
    }
    else if( !strncmp( m_CurrPtr, BLUE_STRING, strlen(BLUE_STRING) ) )
    {
        m_Token.Type = TOKEN_BLUE;
        m_CurrPtr += strlen(BLUE_STRING);
    }
    else if( !strncmp( m_CurrPtr, RATE_STRING, strlen(RATE_STRING ) ) )
    {
        m_Token.Type = TOKEN_RATE;
        m_CurrPtr += strlen(RATE_STRING);
    }
    else if( !strncmp( m_CurrPtr, IN_STRING, strlen(IN_STRING) ) )
    {
        m_Token.Type = TOKEN_IN;
        m_CurrPtr += strlen(IN_STRING);
    }
    else if( !strncmp( m_CurrPtr, OUT_STRING, strlen(OUT_STRING) ) )
    {
        m_Token.Type = TOKEN_OUT;
        m_CurrPtr += strlen(OUT_STRING);
    }
    else if( !strncmp( m_CurrPtr, H_STRING, strlen(H_STRING) ) )
    {
        m_Token.Type = TOKEN_H;
        m_CurrPtr += strlen(H_STRING);
    }
    else if( !strncmp( m_CurrPtr, V_STRING, strlen(V_STRING) ) )
    {
        m_Token.Type = TOKEN_V;
        m_CurrPtr += strlen(V_STRING);
    }
    // handle ints
    else if( *m_CurrPtr >= '0' && *m_CurrPtr <= '9' )
    {
        char* pStringStart = m_CurrPtr;
        while( (*m_CurrPtr >= '0') &&
               (*m_CurrPtr <= '9') &&
               (m_CurrPtr < (m_FileBuffer+m_FileSize)) )
        {
            m_CurrPtr++;
        }

        m_Token.Type = TOKEN_INT;
        int StringLength = (int)(m_CurrPtr-pStringStart);
        if( StringLength > MAX_STRING_LENGTH )
            StringLength = MAX_STRING_LENGTH;
        strncpy( m_Token.StringValue, pStringStart, StringLength );
        m_Token.StringValue[StringLength] = '\0';

        m_Token.IntValue = atoi( m_Token.StringValue );
    }
    // handle strings
    else if( *m_CurrPtr == '\'' )
    {
        m_CurrPtr++;
        char* pStringStart = m_CurrPtr;
        while( (*m_CurrPtr != '\'') &&
               (m_CurrPtr < (m_FileBuffer+m_FileSize)) )
        {
            m_CurrPtr++;
        }

        // reached end of file?
        if( m_CurrPtr >= (m_FileBuffer+m_FileSize) )
        {
            m_Token.Type = TOKEN_EOF;
        }
        else
        {
            m_Token.Type = TOKEN_STRING;
            int StringLength = (int)(m_CurrPtr-pStringStart);
            if( StringLength > MAX_STRING_LENGTH )
                StringLength = MAX_STRING_LENGTH;
            strncpy( m_Token.StringValue, pStringStart, StringLength );
            m_Token.StringValue[StringLength] = '\0';
        }

        // skip over the single quote
        m_CurrPtr++;
    }
}

//==============================================================================

bool premiere_data::ParseVideoTrack( int TrackID )
{
    // we now know this track is a video track
    m_pTracks[TrackID].Type = TRACK_VIDEO;

    // skip over the right bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_RBRACKET )
        return false;

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // now read in all the records
    bool Finished = false;
    int  RecordID = -1;
    while( !Finished )
    {
        ReadToken();
        switch( m_Token.GetType() )
        {
        default:
            return false;
        
        case TOKEN_COMMA:
            // do nothing
            break;

        case TOKEN_LBRACKET:
            {
                // read in the track record token
                ReadToken();
                if( m_Token.GetType() != TOKEN_TRACK_RECORD )
                    return false;

                // read past the pound sign
                ReadToken();
                if( m_Token.GetType() != TOKEN_POUND )
                    return false;

                // read the record number
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                RecordID = m_Token.GetInt();

                // allocate a new record if necessary
                ReallocRecords( TrackID, RecordID + 1 );

                // skip over the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // read the clip id token
                ReadToken();
                if( m_Token.GetType() != TOKEN_CLIP_ID )
                    return false;
    
                // read the equals token
                ReadToken();
                if( m_Token.GetType() != TOKEN_EQUALS )
                    return false;

                // read the clip id
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                m_pTracks[TrackID].pRecords[RecordID].ClipID = m_Token.GetInt();

                // skip over the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;
                
                // read the start token
                ReadToken();
                if( m_Token.GetType() != TOKEN_START )
                    return false;

                // read the equals token
                ReadToken();
                if( m_Token.GetType() != TOKEN_EQUALS )
                    return false;

                // read the start
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                m_pTracks[TrackID].pRecords[RecordID].Start = m_Token.GetInt();

                // skip over the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // read the end token
                ReadToken();
                if( m_Token.GetType() != TOKEN_END )
                    return false;

                // read the equals token
                ReadToken();
                if( m_Token.GetType() != TOKEN_EQUALS )
                    return false;

                // read the end
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                m_pTracks[TrackID].pRecords[RecordID].End = m_Token.GetInt();

                // read the right bracket
                ReadToken();
                if( m_Token.GetType() != TOKEN_RBRACKET )
                    return false;
            }
            break;

        case TOKEN_RBRACKET:
            Finished = true;
            break;
        }
    }
    
    return true;
}

//==============================================================================

bool premiere_data::ParseFXOptions( int TrackID, int RecordID )
{
    // skip over the left bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_LBRACKET )
        return false;

    // skip over the fx options bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_FX_OPTIONS )
        return false;

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // skip over the corners
    ReadToken();
    if( m_Token.GetType() != TOKEN_CORNERS )
        return false;

    // skip over the equals
    ReadToken();
    if( m_Token.GetType() != TOKEN_EQUALS )
        return false;

    // read the corners value
    ReadToken();
    if( m_Token.GetType() != TOKEN_INT )
        return false;
    m_pTracks[TrackID].pRecords[RecordID].FXCorners = m_Token.GetInt();

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // skip over the direction
    ReadToken();
    if( m_Token.GetType() != TOKEN_DIRECTION )
        return false;

    // skip over the equals
    ReadToken();
    if( m_Token.GetType() != TOKEN_EQUALS )
        return false;

    // read the direction value
    ReadToken();
    if( m_Token.GetType() != TOKEN_INT )
        return false;
    m_pTracks[TrackID].pRecords[RecordID].FXDirection = m_Token.GetInt();

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // skip over the start
    ReadToken();
    if( m_Token.GetType() != TOKEN_START )
        return false;

    // skip over the equals
    ReadToken();
    if( m_Token.GetType() != TOKEN_EQUALS )
        return false;

    // read the start value
    ReadToken();
    if( m_Token.GetType() != TOKEN_INT )
        return false;
    m_pTracks[TrackID].pRecords[RecordID].FXStart = m_Token.GetInt();

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // skip over the end
    ReadToken();
    if( m_Token.GetType() != TOKEN_END )
        return false;

    // skip over the equals
    ReadToken();
    if( m_Token.GetType() != TOKEN_EQUALS )
        return false;

    // read the end value
    ReadToken();
    if( m_Token.GetType() != TOKEN_INT )
        return false;
    m_pTracks[TrackID].pRecords[RecordID].FXEnd = m_Token.GetInt();

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // skip over the left bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_LBRACKET )
        return false;

    // skip over the fx type
    ReadToken();
    if( m_Token.GetType() != TOKEN_FX_TYPE )
        return false;

    // skip over the equals
    ReadToken();
    if( m_Token.GetType() != TOKEN_EQUALS )
        return false;

    // read the fx type value
    ReadToken();
    if( m_Token.GetType() != TOKEN_DISSOLVE )
        return false;
    m_pTracks[TrackID].pRecords[RecordID].FXType = FX_TYPE_DISSOLVE;

    // skip over the right bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_RBRACKET )
        return false;

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // skip over the right bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_RBRACKET )
        return false;

    return true;
}

//==============================================================================

bool premiere_data::ParseFXTrack( int TrackID )
{
    // we now know this track is an fx track
    m_pTracks[TrackID].Type = TRACK_FX;

    // skip over the right bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_RBRACKET )
        return false;

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // now read in all the records
    bool Success;
    bool Finished = false;
    int  RecordID = -1;
    while( !Finished )
    {
        ReadToken();
        switch( m_Token.GetType() )
        {
        default:
            return false;

        case TOKEN_COMMA:
            // do nothing
            break;

        case TOKEN_LBRACKET:
            {
                // read in the track record token
                ReadToken();
                if( m_Token.GetType() != TOKEN_TRACK_RECORD )
                    return false;

                // read past the pound sign
                ReadToken();
                if( m_Token.GetType() != TOKEN_POUND )
                    return false;

                // read the record number
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                RecordID = m_Token.GetInt();

                // allocate a new record if necessary
                ReallocRecords( TrackID, RecordID + 1 );

                // skip over the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // read the clip id token
                ReadToken();
                if( m_Token.GetType() != TOKEN_CLIP_ID )
                    return false;

                // read the equals token
                ReadToken();
                if( m_Token.GetType() != TOKEN_EQUALS )
                    return false;

                // read the clip id
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                m_pTracks[TrackID].pRecords[RecordID].ClipID = m_Token.GetInt();

                // skip over the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // read the start token
                ReadToken();
                if( m_Token.GetType() != TOKEN_START )
                    return false;

                // read the equals token
                ReadToken();
                if( m_Token.GetType() != TOKEN_EQUALS )
                    return false;

                // read the start
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                m_pTracks[TrackID].pRecords[RecordID].Start = m_Token.GetInt();

                // skip over the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // read the end token
                ReadToken();
                if( m_Token.GetType() != TOKEN_END )
                    return false;

                // read the equals token
                ReadToken();
                if( m_Token.GetType() != TOKEN_EQUALS )
                    return false;

                // read the end
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                m_pTracks[TrackID].pRecords[RecordID].End = m_Token.GetInt();

                // skip over the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // parse the effects options
                Success = ParseFXOptions( TrackID, RecordID );
                if( !Success )
                    return false;

                // skip over the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // skip over the right bracket
                ReadToken();
                if( m_Token.GetType() != TOKEN_RBRACKET )
                    return false;
            }
            break;

        case TOKEN_RBRACKET:
            Finished = true;
            break;
        }
    }

    return true;
}

//==============================================================================

bool premiere_data::ParseBandPoints( int TrackID, int RecordID )
{
    (void)TrackID;
    (void)RecordID;

    int i;
    for( i = 0; i < 2; i++ )
    {
        // skip the left bracket
        ReadToken();
        if( m_Token.GetType() != TOKEN_LBRACKET )
            return false;

        // skip the band point
        ReadToken();
        if( m_Token.GetType() != TOKEN_BAND_POINT )
            return false;

        // skip the pound
        ReadToken();
        if( m_Token.GetType() != TOKEN_POUND )
            return false;

        // skip the band point #
        ReadToken();
        if( m_Token.GetType() != TOKEN_INT )
            return false;

        // skip the comma
        ReadToken();
        if( m_Token.GetType() != TOKEN_COMMA )
            return false;

        // skip the 'h'
        ReadToken();
        if( m_Token.GetType() != TOKEN_H )
            return false;

        // skip the equals
        ReadToken();
        if( m_Token.GetType() != TOKEN_EQUALS )
            return false;

        // skip the h value
        ReadToken();
        if( m_Token.GetType() != TOKEN_INT )
            return false;

        // skip the comma
        ReadToken();
        if( m_Token.GetType() != TOKEN_COMMA )
            return false;

        // skip the v value
        ReadToken();
        if( m_Token.GetType() != TOKEN_V )
            return false;

        // skip the equals
        ReadToken();
        if( m_Token.GetType() != TOKEN_EQUALS )
            return false;

        // skip the v value
        ReadToken();
        if( m_Token.GetType() != TOKEN_INT )
            return false;

        // skip the right bracket
        ReadToken();
        if( m_Token.GetType() != TOKEN_RBRACKET )
            return false;

        // skip the comma
        ReadToken();
        if( m_Token.GetType() != TOKEN_COMMA )
            return false;
    }

    return true;
}

//==============================================================================

bool premiere_data::ParseAudioOptions( int TrackID, int RecordID )
{
    bool Success;

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // skip over the left bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_LBRACKET )
        return false;

    // skip over the rubberband token
    ReadToken();
    if( m_Token.GetType() != TOKEN_RUBBER_BAND )
        return false;

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // skip over the max token
    ReadToken();
    if( m_Token.GetType() != TOKEN_MAX )
        return false;

    // skip over the equals
    ReadToken();
    if( m_Token.GetType() != TOKEN_EQUALS )
        return false;

    // skip the max value
    ReadToken();
    if( m_Token.GetType() != TOKEN_INT )
        return false;

    // skip the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // skip the band points
    Success = ParseBandPoints( TrackID, RecordID );
    if( !Success )
        return false;

    // skip over the right bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_RBRACKET )
        return false;

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // skip over the left bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_LBRACKET )
        return false;

    // skip over the pan band token
    ReadToken();
    if( m_Token.GetType() != TOKEN_PAN_BAND )
        return false;

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // skip over the max token
    ReadToken();
    if( m_Token.GetType() != TOKEN_MAX )
        return false;

    // skip over the equals
    ReadToken();
    if( m_Token.GetType() != TOKEN_EQUALS )
        return false;

    // skip the max value
    ReadToken();
    if( m_Token.GetType() != TOKEN_INT )
        return false;

    // skip the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // skip the band points
    Success = ParseBandPoints( TrackID, RecordID );
    if( !Success )
        return false;

    // skip over the right bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_RBRACKET )
        return false;

    return true;
}

//==============================================================================

bool premiere_data::ParseAudioTrack( int TrackID )
{
    // we now know this track is an audio track
    m_pTracks[TrackID].Type = TRACK_AUDIO;

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // skip over the left bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_LBRACKET )
        return false;

    // skip over the mapping
    ReadToken();
    if( m_Token.GetType() != TOKEN_MAPPING )
        return false;

    // skip over the equals sign
    ReadToken();
    if( m_Token.GetType() != TOKEN_EQUALS )
        return false;

    // read the mapping value
    ReadToken();
    if( m_Token.GetType() != TOKEN_INT )
        return false;
    m_pTracks[TrackID].AudioMapping = m_Token.GetInt();

    // skip over the right bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_RBRACKET )
        return false;

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // skip over the right bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_RBRACKET )
        return false;

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // now read in all the records
    bool Success;
    bool Finished = false;
    int  RecordID = -1;
    while( !Finished )
    {
        ReadToken();
        switch( m_Token.GetType() )
        {
        default:
            return false;

        case TOKEN_COMMA:
            // do nothing
            break;

        case TOKEN_LBRACKET:
            {
                // read in the track record token
                ReadToken();
                if( m_Token.GetType() != TOKEN_TRACK_RECORD )
                    return false;

                // read past the pound sign
                ReadToken();
                if( m_Token.GetType() != TOKEN_POUND )
                    return false;

                // read the record number
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                RecordID = m_Token.GetInt();

                // allocate a new record if necessary
                ReallocRecords( TrackID, RecordID + 1 );

                // skip over the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // read the clip id token
                ReadToken();
                if( m_Token.GetType() != TOKEN_CLIP_ID )
                    return false;

                // read the equals token
                ReadToken();
                if( m_Token.GetType() != TOKEN_EQUALS )
                    return false;

                // read the clip id
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                m_pTracks[TrackID].pRecords[RecordID].ClipID = m_Token.GetInt();

                // skip over the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // read the start token
                ReadToken();
                if( m_Token.GetType() != TOKEN_START )
                    return false;

                // read the equals token
                ReadToken();
                if( m_Token.GetType() != TOKEN_EQUALS )
                    return false;

                // read the start
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                m_pTracks[TrackID].pRecords[RecordID].Start = m_Token.GetInt();

                // skip over the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // read the end token
                ReadToken();
                if( m_Token.GetType() != TOKEN_END )
                    return false;

                // read the equals token
                ReadToken();
                if( m_Token.GetType() != TOKEN_EQUALS )
                    return false;

                // read the end
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                m_pTracks[TrackID].pRecords[RecordID].End = m_Token.GetInt();

                // parse the audio options
                Success = ParseAudioOptions( TrackID, RecordID );
                if( !Success )
                    return false;

                // skip over the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // skip over the right bracket
                ReadToken();
                if( m_Token.GetType() != TOKEN_RBRACKET )
                    return false;
            }
            break;

        case TOKEN_RBRACKET:
            Finished = true;
            break;
        }
    }

    return true;
}

//==============================================================================

bool premiere_data::ParseTrack( int TrackID )
{
    // skip over the left bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_LBRACKET )
        return false;

    // now based on the next token, parse the different types of tracks
    bool Success;
    ReadToken();
    switch( m_Token.GetType() )
    {
    default:
        return false;

    case TOKEN_VIDEO:
        Success = ParseVideoTrack( TrackID );
        if( !Success )
            return false;
        break;

    case TOKEN_FX:
        Success = ParseFXTrack( TrackID );
        if( !Success )
            return false;
        break;

    case TOKEN_AUDIO:
        Success = ParseAudioTrack( TrackID );
        if( !Success )
            return false;
        break;
    }

    return true;
}

//==============================================================================

bool premiere_data::ParseTracks( void )
{
    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // now loop over all the tracks until we reach the final bracket
    bool Success;
    bool Finished = false;
    int  TrackID  = -1;
    while( !Finished )
    {
        ReadToken();
        switch( m_Token.GetType() )
        {
        default:
            return false;

        case TOKEN_COMMA:
            // do nothing
            break;

        case TOKEN_LBRACKET:
            {
                // read in the track token
                ReadToken();
                if( m_Token.GetType() != TOKEN_TRACK )
                    return false;

                // read past the pound sign
                ReadToken();
                if( m_Token.GetType() != TOKEN_POUND )
                    return false;

                // read in which track this is
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                TrackID = m_Token.GetInt();

                // make sure we have enough space for this track
                ReallocTracks( TrackID+1 );

                // read past the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // now read the track
                Success = ParseTrack( TrackID );
                if( !Success )
                    return false;
            }
            break;

        case TOKEN_RBRACKET:
            // we are finished
            Finished = true;
            break;
        }
    }

    return true;
}

//==============================================================================

bool premiere_data::ParseClips( void )
{
    int  ClipID = -1;
    bool Finished = false;
    while( !Finished )
    {
        ReadToken();
        switch( m_Token.GetType() )
        {
        default:
            return false;

        case TOKEN_COMMA:
            // nothing to do
            break;

        case TOKEN_LBRACKET:
            {
                // skip over the clip ID
                ReadToken();
                if( m_Token.GetType() != TOKEN_CLIP_ID )
                    return false;

                // skip over the pound sign
                ReadToken();
                if( m_Token.GetType() != TOKEN_POUND )
                    return false;

                // read in the clip id
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                ClipID = m_Token.GetInt();

                // allocate space for the clip if necessary
                ReallocClips( ClipID + 1 );

                // skip past the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // skip past the file id token
                ReadToken();
                if( m_Token.GetType() != TOKEN_FILE_ID )
                    return false;

                // skip past the equals
                ReadToken();
                if( m_Token.GetType() != TOKEN_EQUALS )
                    return false;

                // read the file id value
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                m_pClips[ClipID].FileID = m_Token.GetInt();

                // skip past the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // skip past the in token
                ReadToken();
                if( m_Token.GetType() != TOKEN_IN )
                    return false;

                // skip past the equals
                ReadToken();
                if( m_Token.GetType() != TOKEN_EQUALS )
                    return false;

                // read the in value
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                m_pClips[ClipID].InValue = m_Token.GetInt();

                // skip past the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;
                
                // skip past the out token
                ReadToken();
                if( m_Token.GetType() != TOKEN_OUT )
                    return false;

                // skip past the equals
                ReadToken();
                if( m_Token.GetType() != TOKEN_EQUALS )
                    return false;

                // read the out value
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                m_pClips[ClipID].OutValue = m_Token.GetInt();

                // skip past the right bracket
                ReadToken();
                if( m_Token.GetType() != TOKEN_RBRACKET )
                    return false;
            }
            break;

        case TOKEN_RBRACKET:
            // finished?
            Finished = true;
            break;
        }
    }

    return true;
}

//==============================================================================

bool premiere_data::ParseFileOptions( int FileID )
{
    bool Finished = false;
    while( !Finished )
    {
        ReadToken();
        switch( m_Token.GetType() )
        {
        default:
            return false;

        case TOKEN_COMMA:
            // nothing to do
            break;

        case TOKEN_LBRACKET:
            {
                ReadToken();
                switch( m_Token.GetType() )
                {
                default:
                    return false;

                case TOKEN_DOS_FILE:
                    {
                        // skip the equals sign
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_EQUALS )
                            return false;

                        // read the file name
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_STRING )
                            return false;
                        strcpy( m_pFiles[FileID].FileName, m_Token.GetString() );

                        // skip past the right bracket
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_RBRACKET )
                            return false;
                    }
                    break;

                case TOKEN_NUM_FRAMES:
                    {
                        // skip past the equals sign
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_EQUALS )
                            return false;

                        // read the number of frames
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_INT )
                            return false;
                        m_pFiles[FileID].nFrames = m_Token.GetInt();

                        // skip past the right bracket
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_RBRACKET )
                            return false;
                    }
                    break;

                case TOKEN_VIDEO:
                    {
                        // skip past the comma
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_COMMA )
                            return false;

                        // skip past the width
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_WIDTH )
                            return false;

                        // skip past the equals sign
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_EQUALS )
                            return false;

                        // read in the width
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_INT )
                            return false;
                        m_pFiles[FileID].Width = m_Token.GetInt();

                        // skip past the comma
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_COMMA )
                            return false;

                        // skip past the height
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_HEIGHT )
                            return false;

                        // skip past the equals sign
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_EQUALS )
                            return false;

                        // read in the height
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_INT )
                            return false;
                        m_pFiles[FileID].Height = m_Token.GetInt();

                        // skip past the comma
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_COMMA )
                            return false;

                        // skip past the depth
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_DEPTH )
                            return false;

                        // skip past the equals sign
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_EQUALS )
                            return false;

                        // read in the depth
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_INT )
                            return false;
                        m_pFiles[FileID].Depth = m_Token.GetInt();

                        // skip the right bracket
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_RBRACKET )
                            return false;
                    }
                    break;

                case TOKEN_COLOR:
                    {
                        // skip past the comma
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_COMMA )
                            return false;

                        // skip past the red
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_RED )
                            return false;

                        // skip past the equals
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_EQUALS )
                            return false;

                        // read in the red
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_INT )
                            return false;
                        m_pFiles[FileID].Red = m_Token.GetInt();

                        // skip past the comma
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_COMMA )
                            return false;

                        // skip past the green
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_GREEN )
                            return false;

                        // skip past the equals
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_EQUALS )
                            return false;

                        // read in the green
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_INT )
                            return false;
                        m_pFiles[FileID].Green = m_Token.GetInt();

                        // skip past the comma
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_COMMA )
                            return false;

                        // skip past the blue
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_BLUE )
                            return false;

                        // skip past the equals
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_EQUALS )
                            return false;

                        // read in the blue
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_INT )
                            return false;
                        m_pFiles[FileID].Blue = m_Token.GetInt();

                        // skip past the right bracket
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_RBRACKET )
                            return false;
                    }
                    break;
                    
                case TOKEN_AUDIO:
                    {
                        // skip past the comma
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_COMMA )
                            return false;

                        // skip the rate
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_RATE )
                            return false;

                        // skip past the equals
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_EQUALS )
                            return false;

                        // read in the audio rate
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_INT )
                            return false;
                        m_pFiles[FileID].Rate = m_Token.GetInt();

                        // skip past the right bracket
                        ReadToken();
                        if( m_Token.GetType() != TOKEN_RBRACKET )
                            return false;
                    }
                    break;
                }
            }
            break;

        case TOKEN_RBRACKET:
            // finished
            Finished = true;
            break;
        }
    }

    return true;
}

//==============================================================================

bool premiere_data::ParseFiles( void )
{
    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // loop through all the files
    bool Success;
    int  FileID = -1;
    bool Finished = false;
    while( !Finished )
    {
        ReadToken();
        switch( m_Token.GetType() )
        {
        default:
            return false;

        case TOKEN_COMMA:
            // nothing to do
            break;
            
        case TOKEN_LBRACKET:
            {
                // skip past the file id
                ReadToken();
                if( m_Token.GetType() != TOKEN_FILE_ID )
                    return false;

                // skip past the pound sign
                ReadToken();
                if( m_Token.GetType() != TOKEN_POUND )
                    return false;

                // read the file id
                ReadToken();
                if( m_Token.GetType() != TOKEN_INT )
                    return false;
                FileID = m_Token.GetInt();

                // allocate space for the file
                ReallocFiles( FileID + 1 );

                // skip past the comma
                ReadToken();
                if( m_Token.GetType() != TOKEN_COMMA )
                    return false;

                // read the file options
                Success = ParseFileOptions( FileID );
                if( !Success )
                    return false;
            }
            break;

        case TOKEN_RBRACKET:
            // finished
            Finished = true;
            break;
        }
    }

    return true;
}

//==============================================================================

bool premiere_data::ParseData( void )
{
    m_CurrPtr = m_FileBuffer;

    // skip past the first left bracket
    ReadToken();
    if( m_Token.GetType() != TOKEN_LBRACKET )
        return false;

    // check the string to make sure it is a version we can handle
    ReadToken();
    if( m_Token.GetType() != TOKEN_STRING )
        return false;

    // verify the version is one we can handle
    strcpy( m_Version, m_Token.GetString() );
    if( strcmp( m_Version, "Adobe Premiereª 6.0 Generic Edit Decision List" ) )
        return false;

    // skip over the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // read in the work start token
    ReadToken();
    if( m_Token.GetType() != TOKEN_WORK_START )
        return false;

    // skip past the equals sign
    ReadToken();
    if( m_Token.GetType() != TOKEN_EQUALS )
        return false;

    // read in the work start
    ReadToken();
    if( m_Token.GetType() != TOKEN_INT )
        return false;
    m_WorkStart = m_Token.GetInt();

    // skip past the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;
    
    // read in the work end token
    ReadToken();
    if( m_Token.GetType() != TOKEN_WORK_END )
        return false;

    // skip past the equals sign
    ReadToken();
    if( m_Token.GetType() != TOKEN_EQUALS )
        return false;

    // read in the work end
    ReadToken();
    if( m_Token.GetType() != TOKEN_INT )
        return false;
    m_WorkEnd = m_Token.GetInt();

    // skip past the comma
    ReadToken();
    if( m_Token.GetType() != TOKEN_COMMA )
        return false;

    // now loop over the next set of tokens until we are finished
    bool Success;
    bool Finished = false;
    while( !Finished )
    {
        ReadToken();
        switch( m_Token.GetType() )
        {
        case TOKEN_COMMA:
            // do nothing
            break;

        case TOKEN_LBRACKET:
            {
                // read the next token and call other functions based on that
                ReadToken();
                switch( m_Token.GetType() )
                {
                case TOKEN_TRACKS:
                    Success = ParseTracks();
                    if( !Success )
                        return false;
                    break;
                case TOKEN_CLIPS:
                    Success = ParseClips();
                    if( !Success )
                        return false;
                    break;
                case TOKEN_FILES:
                    Success = ParseFiles();
                    if( !Success )
                        return false;
                    break;
                default:
                    return false;
                }
            }
            break;

        case TOKEN_RBRACKET:
            // if we've reached the final bracket, then we are done
            Finished = true;
            break;
        }
    }

    return true;
}

//==============================================================================

void premiere_data::ReallocRecords( int TrackID, int nRecords )
{
    p_track& Track = m_pTracks[TrackID];

    if( nRecords > m_pTracks[TrackID].nRecords )
    {
        // Allocate space for records if this is the first time we've been in
        // here. Otherwise, realloc the space we already have to make it bigger.
        if( Track.nRecords == 0 )
            Track.pRecords = (p_record*)malloc( sizeof(p_record) * nRecords );
        else
            Track.pRecords = (p_record*)realloc( Track.pRecords, sizeof(p_record) * nRecords );
        int PrevNRecords = Track.nRecords;
        Track.nRecords   = nRecords;

        // zero out all of those records that we just allocated
        int ZeroCount = Track.nRecords - PrevNRecords;
        memset( &Track.pRecords[PrevNRecords], 0, ZeroCount * sizeof(p_record) );
    }
}

//==============================================================================

void premiere_data::ReallocTracks( int nTracks )
{
    if( nTracks > m_nTracks )
    {
        // Allocate space for tracks if this is the first time we've been in
        // here. Otherwise, realloc the space we already have to make it bigger.
        if( m_nTracks == 0 )
            m_pTracks = (p_track*)malloc( sizeof(p_track) * nTracks );
        else
            m_pTracks = (p_track*)realloc( m_pTracks, sizeof(p_track) * nTracks );
        int PrevNTracks = m_nTracks;
        m_nTracks       = nTracks;

        // zero out all of those tracks that we just allocated
        int ZeroCount = m_nTracks - PrevNTracks;
        memset( &m_pTracks[PrevNTracks], 0, ZeroCount * sizeof(p_track) );
    }
}

//==============================================================================

void premiere_data::ReallocClips( int nClips )
{
    if( nClips > m_nClips )
    {
        // Allocate space for clips if this is the first time we've been in
        // here. Otherwise, realloc the space we already have to make it bigger.
        if( m_nClips == 0 )
            m_pClips = (p_clip*)malloc( sizeof(p_clip) * nClips );
        else
            m_pClips = (p_clip*)realloc( m_pClips, sizeof(p_clip) * nClips );
        int PrevNClips = m_nClips;
        m_nClips = nClips;

        // zero out all of those clips that we just allocated
        int ZeroCount = m_nClips - PrevNClips;
        memset( &m_pClips[PrevNClips], 0, ZeroCount * sizeof(p_clip) );
    }
}

//==============================================================================

void premiere_data::ReallocFiles( int nFiles )
{
    if( nFiles > m_nFiles )
    {
        // Allocate space for files if this is the first time we've been in
        // here. Otherwise, realloc the space we already have to make it bigger.
        if( m_nFiles == 0 )
            m_pFiles = (p_file*)malloc( sizeof(p_file) * nFiles );
        else
            m_pFiles = (p_file*)realloc( m_pFiles, sizeof(p_file) * nFiles );
        int PrevNFiles = m_nFiles;
        m_nFiles = nFiles;

        // zero out all of those files that we just allocated
        int ZeroCount = m_nFiles - PrevNFiles;
        memset( &m_pFiles[PrevNFiles], 0, ZeroCount * sizeof(p_file) );

        // initialize all the colors to white
        int i;
        for( i = PrevNFiles; i < m_nFiles; i++ )
        {
            m_pFiles[i].Red   = 255;
            m_pFiles[i].Green = 255;
            m_pFiles[i].Blue  = 255;
        }
    }
}

//==============================================================================

void CleanFileName( char* pFileName )
{
    char CleanedName[MAX_STRING_LENGTH+1];

    // Find where the '.' is for the extension. This will be the end of
    // our cleaned up file name
    char* pEnd = &pFileName[strlen(pFileName) - 1];
    while( (pEnd > pFileName) && (*pEnd != '.') )
    {
        pEnd--;
    }
    if( pEnd == pFileName )
        return;

    // Find where the start of our filename is. This will either be right after
    // the first directory backslash, or if there is not a full path, just the
    // first letter.
    char* pStart = pEnd;
    while( (pStart > pFileName) && (*pStart != '\\') )
    {
        pStart--;
    }
    if( *pStart == '\\' )
        pStart++;

    // copy off the filename into a temp buffer
    int Length = (int)(pEnd - pStart);
    strncpy( CleanedName, pStart, Length );
    CleanedName[Length] = '\0';

    // make it upper case
    char* pCurr = CleanedName;
    while( *pCurr != '\0' )
    {
        if( (*pCurr>='a') && (*pCurr<='z') )
            *pCurr += 'A' - 'a';
        pCurr++;
    }

    // copy it back over the original
    strcpy( pFileName, CleanedName );
}

//==============================================================================

int _tmain( int argc, char** argv )
{
    // start with cleared slide data
    slide_data  SlideData[8];
    memset( SlideData, 0, sizeof(SlideData) );
    char AudioName[MAX_STRING_LENGTH+1];
    memset( AudioName, 0, sizeof(AudioName) );

    // make sure we have a valid filename
    argc--;
    argv++;
    if( (argc==0) ||
        (*argv==NULL) ||
        (*argv[0]=='\0') )
    {
        printf("Usage: EDLParser <edl filename>");
        return -1;
    }

    // load the premiere file containing the slideshow
    premiere_data SlideShow;
    bool Success = SlideShow.Load( *argv );
    if( !Success )
    {
        printf( "ERROR: Failed to load %s\n", *argv );
        return -1;
    }

    // find the first video track--this should be our slides
    int TrackID;
    for( TrackID = 0; TrackID < SlideShow.m_nTracks; TrackID++ )
    {
        if( SlideShow.m_pTracks[TrackID].Type == premiere_data::TRACK_VIDEO )
            break;
    }
    if( TrackID == SlideShow.m_nTracks )
    {
        printf( "ERROR: Couldn't find slide video track!" );
        return -1;
    }

    // make sure that the slide video track has 5 records
    if( SlideShow.m_pTracks[TrackID].nRecords != 5 )
    {
        printf( "ERROR: Slide video track didn't have 5 records!" );
        return -1;
    }

    // fill in the file names and colors for the first five slides
    int i;
    for( i = 0; i < 5; i++ )
    {
        int ClipID = SlideShow.m_pTracks[TrackID].pRecords[i].ClipID;
        int FileID = SlideShow.m_pClips[ClipID].FileID;
        strcpy( SlideData[i].FileName, SlideShow.m_pFiles[FileID].FileName );
        SlideData[i].Red   = (float)SlideShow.m_pFiles[FileID].Red   / 255.0f;
        SlideData[i].Green = (float)SlideShow.m_pFiles[FileID].Green / 255.0f;
        SlideData[i].Blue  = (float)SlideShow.m_pFiles[FileID].Blue  / 255.0f;
    }

    // find the next video track--this should be our white screens
    TrackID++;
    for( ; TrackID < SlideShow.m_nTracks; TrackID++ )
    {
        if( SlideShow.m_pTracks[TrackID].Type == premiere_data::TRACK_VIDEO )
            break;
    }
    if( TrackID == SlideShow.m_nTracks )
    {
        printf( "ERROR: Couldn't find white screen video track!" );
        return -1;
    }

    // make sure that the white screen video track has 3 records
    if( SlideShow.m_pTracks[TrackID].nRecords != 3 )
    {
        printf( "ERROR: White screen video track didn't have 3 records!" );
        return -1;
    }

    // fill in the start frame, end frame, and color for the white slides
    for( i = 0; i < 3; i++ )
    {
        SlideData[5+i].StartFadeIn  = SlideShow.m_pTracks[TrackID].pRecords[i].Start;
        SlideData[5+i].EndFadeIn    = SlideShow.m_pTracks[TrackID].pRecords[i].Start;
        SlideData[5+i].StartFadeOut = SlideShow.m_pTracks[TrackID].pRecords[i].Start;
        SlideData[5+i].EndFadeOut   = SlideShow.m_pTracks[TrackID].pRecords[i].End;

        int ClipID = SlideShow.m_pTracks[TrackID].pRecords[i].ClipID;
        int FileID = SlideShow.m_pClips[ClipID].FileID;
        SlideData[5+i].Red   = (float)SlideShow.m_pFiles[FileID].Red   / 255.0f;
        SlideData[5+i].Green = (float)SlideShow.m_pFiles[FileID].Green / 255.0f;
        SlideData[5+i].Blue  = (float)SlideShow.m_pFiles[FileID].Blue  / 255.0f;
    }

    // find the FX track
    for( TrackID = 0; TrackID < SlideShow.m_nTracks; TrackID++ )
    {
        if( SlideShow.m_pTracks[TrackID].Type == premiere_data::TRACK_FX )
            break;
    }
    if( TrackID == SlideShow.m_nTracks )
    {
        printf( "ERROR: Couldn't find fx track!" );
        return -1;
    }

    // make sure the fx track has 9 records
    if( SlideShow.m_pTracks[TrackID].nRecords != 9 )
    {
        printf( "ERROR: FX track didn't have 9 records!" );
        return -1;
    }

    // fill in the fade-in and fade-out times for the first five slides
    // The first slide only has a fade-out, but the rest have a fade in
    // AND fade-out.
    // So, record 0 is the fadeout for slide 0
    // Record 1-2 is the fadein and fadeout for slide 1
    // Record 3-4 is the fadein and fadeout for slide 2
    // Record 5-6 is the fadein and fadeout for slide 3
    // Record 7-8 is the fadein and fadeout for slide 4
    SlideData[0].StartFadeIn  = 0;
    SlideData[0].EndFadeIn    = 0;
    SlideData[0].StartFadeOut = SlideShow.m_pTracks[TrackID].pRecords[0].Start;
    SlideData[0].EndFadeOut   = SlideShow.m_pTracks[TrackID].pRecords[0].End;
    for( i = 1; i < 5; i++ )
    {
        SlideData[i].StartFadeIn  = SlideShow.m_pTracks[TrackID].pRecords[i*2-1].Start;
        SlideData[i].EndFadeIn    = SlideShow.m_pTracks[TrackID].pRecords[i*2-1].End;
        SlideData[i].StartFadeOut = SlideShow.m_pTracks[TrackID].pRecords[i*2+0].Start;
        SlideData[i].EndFadeOut   = SlideShow.m_pTracks[TrackID].pRecords[i*2+0].End;
    }

    // find the audio track
    for( TrackID = 0; TrackID < SlideShow.m_nTracks; TrackID++ )
    {
        if( SlideShow.m_pTracks[TrackID].Type == premiere_data::TRACK_AUDIO )
            break;
    }
    if( TrackID == SlideShow.m_nTracks )
    {
        printf( "ERROR: Couldn't find audio track!" );
        return -1;
    }

    // make sure the audio track only has one record
    if( SlideShow.m_pTracks[TrackID].nRecords != 1 )
    {
        printf( "ERROR: Audio track should only have 1 record!" );
        return -1;
    }

    // copy out the audio track name
    int ClipID = SlideShow.m_pTracks[TrackID].pRecords[0].ClipID;
    int FileID = SlideShow.m_pClips[ClipID].FileID;
    strcpy( AudioName, SlideShow.m_pFiles[FileID].FileName );

    // now strip the extensions off the audio and slide names
    CleanFileName( AudioName );
    for( i = 0; i < 5; i++ )
    {
        CleanFileName( SlideData[i].FileName );
    }

    // output some text that would do the level settings
    printf( "   \"LevelSettings\\SlideShowAudio\"          \"STRING\"     \"%s\"\n", AudioName );
    printf( "   \"LevelSettings\\StartTextAnim\"           \"INT\"        \"%d\"\n", SlideData[4].StartFadeIn );
    printf( "   \"LevelSettings\\NumSlides\"               \"INT\"        \"8\"\n" );
    for( i = 0; i < 8; i++ )
    {
        printf( "   \"LevelSettings\\Slide[%d]\\ImageName\"      \"STRING\"     \"%s\"\n", i, SlideData[i].FileName );
        printf( "   \"LevelSettings\\Slide[%d]\\SlideColor\"     \"COLOR\"      \"%8.6f %8.6f %8.6f %8.6f\"\n",
                i, SlideData[i].Red, SlideData[i].Green, SlideData[i].Blue, 1.0f );
        printf( "   \"LevelSettings\\Slide[%d]\\StartFadeIn\"    \"INT\"        \"%d\"\n", i, SlideData[i].StartFadeIn );
        printf( "   \"LevelSettings\\Slide[%d]\\EndFadeIn\"      \"INT\"        \"%d\"\n", i, SlideData[i].EndFadeIn );
        printf( "   \"LevelSettings\\Slide[%d]\\StartFadeOut\"   \"INT\"        \"%d\"\n", i, SlideData[i].StartFadeOut );
        printf( "   \"LevelSettings\\Slide[%d]\\EndFadeOut\"     \"INT\"        \"%d\"\n", i, SlideData[i].EndFadeOut );
    }

	return 0;
}
