
#ifndef TEXT_IN_HPP
#define TEXT_IN_HPP

//=========================================================================
// This file reads text file descrive as falls:
// 
// [ HeaderName : HeaderCount ]
// { FieldName:ddd  FieldName:fff FieldName:sss FieldName:fds }
//      1 2 3       1.0 2.0 3.0   "1" "2" "3"    1.0 2 "3"
// ...
// 
// or also can look like this
//
// [ HeaderName ]
// { FieldName:ddd  FieldName:fff FieldName:sss FieldName:fds }
//      1 2 3       1.0 2.0 3.0   "1" "2" "3"    1.0 2 "3"
// ...
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "x_math.hpp"
#include "x_color.hpp"
#include "Tokenizer.hpp"

//=========================================================================
// CLASS
//=========================================================================
class text_in
{
public:
                            text_in        ( void );
                           ~text_in        ( void ); // do not virtualise

           xbool            OpenFile        ( const char* pFileName );
           void             CloseFile       ( void );

           xbool            ReadHeader      ( void );
           xbool            SkipToNextHeader( void );

           xbool            ReadFields      ( void );
           xbool            GetField        ( const char* FieldName, ... );
           xbool            GetVector3      ( const char* pName, vector3& V );
           xbool            GetColor        ( const char* pName, xcolor& C );
           xbool            GetF32          ( const char* pName, f32& F );
           xbool            GetS32          ( const char* pName, s32& I );
           xbool            GetString       ( const char* pName, char* pStr );
           xbool            GetBBox         ( const char* pName, bbox& BBox );
           xbool            GetRadian3      ( const char* pName, radian3& Orient );
           xbool            GetQuaternion   ( const char* pName, quaternion& Q );
           xbool            GetBool         ( const char* pName, xbool& Bool );
           xbool            GetGuid         ( const char* pName, guid& Guid );

    inline const char*      GetHeaderName   ( void ) const { return m_Record.Name;              }
    inline s32              GetHeaderCount  ( void ) const { return m_Record.Count;             }
    inline const char*      GetError        ( void ) const { return s_Error;                    }
    inline xbool            IsEOF           ( void ) const { return m_Tokenizer.IsEOF();        }
    inline const char*      GetFileName     ( void )       { return m_Tokenizer.GetFilename();  }

	inline s32				GetFieldCount   ( void ) const { return m_Record.nFields;    }
    inline const char*      GetFieldName    ( s32 nIndex ) const { return m_Record.Field[nIndex].Name; }
    inline s32              GetFieldTypeCount ( s32 nIndex ) const { return m_Record.Field[nIndex].nTypes; }
           void             GetFieldTypeStr ( s32 nField, char* pString );
           xbool            GetFieldAsString( const char* FieldName, char **pOutStr);
//=========================================================================
// END PUBLIC
//=========================================================================
protected:

    enum type
    {
        TYPE_NULL,
        TYPE_FLOAT,
        TYPE_INTEGER,
        TYPE_STRING,
        TYPE_GUID
    };

    #define TEXTFILE_MAX_FIELDS      32         // a:f b:f c:f d:f e:f f:f g:f h:f ...
    #define TEXTFILE_MAX_TYPES       16         // MaxTypes:ffffffffffffffff
    #define TEXTFILE_MAX_STRLENGTH   256        // "dssdfsdfsdsdfdsfsdfsfd ..." 
	#define TEXTFILE_MAX_FLOATS		 16
	#define TEXTFILE_MAX_INTS		 16
	#define TEXTFILE_MAX_STRINGS	 4
	#define TEXTFILE_MAX_GUIDS		 1

    struct data
    {
        s32     FieldID;

        s32     nFloats;
        s32     nIntegers;
        s32     nStrings;
        s32     nGuids;

        f32     Float  [TEXTFILE_MAX_FLOATS];
        s32     Integer[TEXTFILE_MAX_INTS];
        char    String [TEXTFILE_MAX_STRINGS][TEXTFILE_MAX_STRLENGTH];
        guid    Guid   [TEXTFILE_MAX_GUIDS];
    };

    struct field
    {
        char    Name[TEXTFILE_MAX_STRLENGTH];
        s32     ID;
        s32     nTypes;
        type    Type[TEXTFILE_MAX_TYPES];
    };

    struct record
    {
        char    Name[TEXTFILE_MAX_STRLENGTH];
        s32     Count;

        s32     nFields;
        s32     nFieldsAllocated;
        field*  Field;
    };

    struct field_match
    {
        const char* UserString;
        s32         Index;
    };

protected:
    
    xbool   ReadAllFields  ( void );
    xbool   ReportError ( char* pStr, ... );
    s32     Stricmp     ( const char* pStr1,  const char* pStr2, s32 Count );

protected:

    record          m_Record;
    token_stream    m_Tokenizer;
    s32             m_nValidFields;

    s32             m_nFieldMatches;
    s32             m_nFieldMatchesAllocated;
    field_match*    m_FieldMatch;

    s32             m_RecordLineNumber;
    s32             m_RecordTypeNumber;
    
    data*           m_pData;
    s32             m_nDataEntriesAllocated;

    static char     s_Error[512]; 

protected:
};

//=========================================================================
// END
//=========================================================================
#endif
