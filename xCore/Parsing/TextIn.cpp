
#include "TextIn.hpp"
#include "x_plus.hpp"
#include "x_stdio.hpp"
#include "x_string.hpp"

//=========================================================================
//=========================================================================
// VARIABLES
//=========================================================================
//=========================================================================

char                text_in::s_Error[512];    

//=========================================================================
//=========================================================================
// FUNCTIONS
//=========================================================================
//=========================================================================

text_in::text_in( void )
{
    m_pData = NULL;
    m_nDataEntriesAllocated = 0;

    m_Record.nFieldsAllocated = 0;
    m_Record.Field = NULL;

    m_nFieldMatchesAllocated = 0;
    m_FieldMatch = NULL;
}

//=========================================================================

text_in::~text_in( void )
{
    CloseFile();
    delete[] m_pData;
    x_free( m_Record.Field );
    x_free( m_FieldMatch );
}

//=========================================================================

xbool text_in::ReportError( char* pStr, ... )
{
    char Buff[512];
    x_va_list   Args;
    x_va_start( Args, pStr );

    x_vsprintf( Buff, pStr, Args );

    if( m_Record.Name[0] )
    {
        x_sprintf( s_Error, "%s(%d) : error : In Record %s, %s\n", 
            m_Tokenizer.GetFilename(), 
            m_Tokenizer.GetLineNumber(),
            GetHeaderName(),
            Buff );
    }
    else
    {
        x_sprintf( s_Error, "%s(%d) : error : %s\n", 
            m_Tokenizer.GetFilename(), 
            m_Tokenizer.GetLineNumber(),
            Buff );
    }
    
    x_DebugMsg( s_Error );

    return FALSE;
}

//=========================================================================

void text_in::CloseFile( void )
{
    m_Tokenizer.CloseFile();
}

//=========================================================================

xbool text_in::OpenFile( const char* pFileName )
{
    // Reset the class
    m_nValidFields   = 0;
    m_Record.nFields = 0;
    m_Record.Name[0] = 0;

    // Set our list of delimeters
    m_Tokenizer.SetDelimeter( ":,[]{}()<>" );

    // Open the file
    if( m_Tokenizer.OpenFile( pFileName ) == FALSE ) 
    {
        x_throw( xfs("Unable to open file [%s]", pFileName) );
        return FALSE ;
    }

    return TRUE ;
}

//=========================================================================

xbool text_in::ReadAllFields( void )
{
    token_stream::type Type;

    // Reset the current field count to zero
    m_nValidFields   = 0;
    m_Record.nFields = 0;

    //
    // First lets make sure that we start with the right delimeter
    //
    Type = m_Tokenizer.Read();
    if( Type != token_stream::TOKEN_DELIMITER )
    {
        return FALSE;
    }

    if( m_Tokenizer.Delimiter() != '{' ) 
    {
        return ReportError( "Puntuation missing. Expecting { found %c", m_Tokenizer.Delimiter() );
    }

    //
    // Now we are ready to read fields
    //
    while( 1 )
    {
        // Update number of fields allocated
        if( m_Record.nFields >= m_Record.nFieldsAllocated )
        {
            m_Record.nFieldsAllocated = MAX(32,(m_Record.nFields+1)+(m_Record.nFields/2));
            m_Record.Field = (field*)x_realloc( m_Record.Field, sizeof(field)*m_Record.nFieldsAllocated );
            ASSERT( m_Record.Field );
        }
        field&      Field = m_Record.Field[ m_Record.nFields ];

        //
        // First must find a string
        //
        Type = m_Tokenizer.Read();
        if( Type != token_stream::TOKEN_SYMBOL ) 
        {
            // Are we done reading fields?
            if( Type == token_stream::TOKEN_DELIMITER )
            {
                if( m_Tokenizer.Delimiter() == '}' ) 
                {
                    break;
                }                
            }

            return ReportError( "Expecting to read a string for the name of the field but found something else" );
        }

        // Read the name of the field
        x_strncpy( Field.Name, m_Tokenizer.String(), TEXTFILE_MAX_STRLENGTH-1 );

        //
        // Now we must find a delimeter
        // 
        Type = m_Tokenizer.Read();
        if( Type != token_stream::TOKEN_DELIMITER ) 
        {
            return ReportError( "Puntuation missing. Expecting : found somthing else" );
        }

        if( m_Tokenizer.Delimiter() != ':' ) 
        {
            return ReportError( "Puntuation missing. Expecting : found %c", m_Tokenizer.Delimiter() );
        }

        //
        // Now we should read a string
        //
        Type = m_Tokenizer.Read();
        if( Type != token_stream::TOKEN_SYMBOL ) 
        {
            return ReportError( "Expecting the type of the field but found something else" );
        }

        //
        // We must interpret the string into types
        //
        {
            char* pStr   = m_Tokenizer.String();

            ASSERT(pStr);
            Field.nTypes =  0;
            Field.ID     = -1;

            while( *pStr )
            {
                ASSERT(Field.nTypes < TEXTFILE_MAX_TYPES);
                switch( *pStr )
                {
                case 'd':
                case 'D': 
                    Field.Type[ Field.nTypes++ ] = TYPE_INTEGER; 
                    break;

                case 'f':
                case 'F': 
                    Field.Type[ Field.nTypes++ ] = TYPE_FLOAT; 
                    break;

                case 'S':
                case 's': 
                    Field.Type[ Field.nTypes++ ] = TYPE_STRING; 
                    break;

                case 'G':
                case 'g':
                    Field.Type[ Field.nTypes++ ] = TYPE_GUID;
                    break;

                default:
                    return ReportError( "Unkown type %c from %s field", *pStr, Field.Name );
                }

                // Get the next character
                pStr++;
            }
            ASSERT(Field.nTypes < TEXTFILE_MAX_TYPES);
        }

        //
        // Okay we are done with this field
        //
        m_Record.nFields++;
    }

    // DONE
    return TRUE;
}

//=========================================================================

xbool text_in::ReadHeader( void )
{
    token_stream::type Type;

    // Clear the record name
    m_Record.Name[0]    = 0;
    m_Record.Count      = -1;
    m_RecordLineNumber  = 0;
    m_nFieldMatches     = 0;

    //
    // Read Delimiters untill we find the right one
    //
    while( 1 )
    {
        Type = m_Tokenizer.Read();

        if( Type == token_stream::TOKEN_EOF || 
            Type == token_stream::TOKEN_NONE ) 
        {
            return FALSE;
        }

        if( Type == token_stream::TOKEN_DELIMITER ) 
        {
            if( m_Tokenizer.Delimiter() == '[' ) break;
        }
    }

    //
    // Now it should be fallow by a string
    //
    Type = m_Tokenizer.Read();
    if( Type != token_stream::TOKEN_SYMBOL )
    {
        return ReportError( "Expecting a string found something else" );
    }

    x_strcpy( m_Record.Name, m_Tokenizer.String() );

    //
    // Now if must be fallow by a delimeter
    //
    Type = m_Tokenizer.Read();
    if( Type != token_stream::TOKEN_DELIMITER )
    {
        return ReportError( "Expecting a ] or : But found something else" );
    }

    // If it is the end of the block we are then done
    if( m_Tokenizer.Delimiter() == ']' )
    {
        return ReadAllFields();
    }

    // We don't know that is going on so just return
    if( m_Tokenizer.Delimiter() != ':' )
    {
        return ReportError( "Expecting a ] or : But found %c",  m_Tokenizer.Delimiter() );
    }

    //
    // Must be a integer at this point
    //
    Type = m_Tokenizer.Read();
    if( Type != token_stream::TOKEN_NUMBER )
    {
        return ReportError( "After : specting an integer but found something else" );
    }

    // Get our count
    m_Record.Count = m_Tokenizer.Int();
    
    //
    // Now it must be a nother delimerter
    //
    Type = m_Tokenizer.Read();
    if( Type != token_stream::TOKEN_DELIMITER )
    {
        return ReportError( "Expecting ] But stead found something else" );
    }

    //
    // Now start reading fields
    //
    return ReadAllFields();
}

//=========================================================================
	#define TEXTFILE_MAX_FLOATS		 16
	#define TEXTFILE_MAX_INTS		 16
	#define TEXTFILE_MAX_STRINGS	 4
	#define TEXTFILE_MAX_GUIDS		 1

xbool text_in::ReadFields( void )
{
    if( m_Record.nFields > m_nDataEntriesAllocated )
    {
        delete[] m_pData;
        m_nDataEntriesAllocated = m_Record.nFields;
        m_pData = new data[ m_nDataEntriesAllocated ];
    }
    for( s32 f=0; f<m_Record.nFields; f++ )
    {
        field&              Field = m_Record.Field[ f ];
        data&               Data  = m_pData[f];

        Data.nFloats   = 0;
        Data.nIntegers = 0;
        Data.nStrings  = 0;
        Data.nGuids    = 0;

        for( s32 i=0; i<Field.nTypes; i++ )
        {
            token_stream::type  Type = m_Tokenizer.Read();

            // Read the 
            switch( Field.Type[i] )
            {
            case TYPE_FLOAT:    
                {
					ASSERT( Data.nFloats < TEXTFILE_MAX_FLOATS );
                    if( Type != token_stream::TOKEN_NUMBER && m_Tokenizer.IsFloat()== TRUE )
                        return ReportError( "Expecting a FLOAT but found something else" );

                    

                    Data.Float[ Data.nFloats++ ] = m_Tokenizer.Float();

                    break;
                }
            case TYPE_INTEGER:
                {
					ASSERT( Data.nIntegers < TEXTFILE_MAX_INTS );
                    if( Type != token_stream::TOKEN_NUMBER && m_Tokenizer.IsFloat()== FALSE )
                        return ReportError( "Expecting a INTEGER but found something else" );

                    Data.Integer[ Data.nIntegers++ ] = m_Tokenizer.Int();

                    break;
                }
            case TYPE_STRING:
                {
					ASSERT( Data.nStrings < TEXTFILE_MAX_STRINGS );
                    if( Type != token_stream::TOKEN_STRING )
                        return ReportError( "Expecting a STRING but found something else" );

                    x_strcpy( Data.String[ Data.nStrings++ ], m_Tokenizer.String() );

                    break;
                }

            case TYPE_GUID:
                {
					ASSERT( Data.nGuids < TEXTFILE_MAX_GUIDS );
                    if( Type != token_stream::TOKEN_STRING )
                        return ReportError( "Expecting a GUID but found something else" );

                        
                    guid GUID;

                    //
                    // Take from guid.cpp
                    //
                    {
                        GUID.Guid = 0;
                        const char* pGUID = m_Tokenizer.String();

                        while( *pGUID )
                        {
                            char c = *pGUID;
                            pGUID++;

                            if(c == ':') continue;
                            u32 v = 0;
                            if( c>'9' ) v = (c-'A')+10;
                            else        v = (c-'0');

                            GUID.Guid <<= 4;
                            GUID.Guid |= (v&0xF);
                        }
                    }

                    Data.Guid[ Data.nGuids++ ] = GUID;

                    break;
                }

            default :
                ASSERT( FALSE );
            }
        }
    }

    // Increase the line number
    m_RecordLineNumber++;
    m_RecordTypeNumber = 0;

    return TRUE;
}

//=========================================================================

s32 text_in::Stricmp( const char* pStr1,  const char* pStr2, s32 Count )
{
    char Buf[256];
    x_strncpy(Buf,pStr2,Count);
    Buf[Count]=0;
    return x_stricmp(pStr1,Buf);

}

//=========================================================================

xbool text_in::GetField( const char* pFieldName, ... )
{
    
    //
    // Veryfy that all is good in the first line
    //
    if( m_RecordLineNumber == 1 )
    {
        s32   Index = -1;
        s32   j;
        char* pType;
        pType = x_strstr( pFieldName, ":" );

        if( pType == NULL ) 
        {
            return ReportError( "User forgot to put the type in %s field", pFieldName );
        }

        //
        // Make sure that we have that field
        //
        {
            s32 Length = (s32)(pType - pFieldName);        
            for( j=0; j<m_Record.nFields; j++ )
            {
                if( Stricmp( m_Record.Field[ j ].Name, pFieldName, Length ) == 0 )
                {               
                    if( m_Record.Field[ j ].ID != -1 )
                    {
                        return ReportError( "User register the same type (%s) twice", pFieldName );
                    }

                    // Set the user ID
                    m_Record.Field[ j ].ID = 0;
                    Index                  = j;

                    // There was found and set
                    m_nValidFields++;
                    break;
                }            
            }

            if( j == m_Record.nFields )
            {
                return ReportError( "The field %s was not found in the file", pFieldName );
            }
        }
        ASSERT( Index >= 0);

        //
        // Check the types against what we know form the file
        //
        {
            s32     nValidTypes = 0;
            field&  Field = m_Record.Field[ Index ];

            // Skip the : 
            pType++;

            while( pType[nValidTypes] )
            {
                type Type;
                switch( pType[nValidTypes] )
                {
                case 'd':
                case 'D': 
                    Type = TYPE_INTEGER; 
                    break;

                case 'f':
                case 'F': 
                    Type = TYPE_FLOAT; 
                    break;

                case 'S':
                case 's': 
                    Type = TYPE_STRING; 
                    break;

                case 'G':
                case 'g':
                    Type = TYPE_GUID;
                    break;

                default:
                    return ReportError( "Unkown type %c from %s field", pType[nValidTypes], Field.Name );
                }

                if( Type != Field.Type[ nValidTypes++ ] )
                {
                    return ReportError( "For %s field the types don't match.", Field.Name );
                }
            }

            if( nValidTypes != Field.nTypes )
            {
                return ReportError( "For field %s the types are different from the user specify one.", Field.Name );
            }

            // Resize array if needed
            if( m_RecordTypeNumber >= m_nFieldMatchesAllocated )
            {
                m_nFieldMatchesAllocated = MAX(32,(m_RecordTypeNumber+1) + (m_RecordTypeNumber/2));
                m_FieldMatch = (field_match*)x_realloc( m_FieldMatch, sizeof(field_match)*m_nFieldMatchesAllocated );
                ASSERT( m_FieldMatch );
            }
            //
            // Set the new type
            //
            m_FieldMatch[ m_RecordTypeNumber ].UserString = pFieldName;
            m_FieldMatch[ m_RecordTypeNumber ].Index      = Index;
        }
    }

    // SB:
    // Crash fix - if we have past the last field, then this field is not present so found bail elegantly
    if( m_RecordTypeNumber == m_Record.nFields )
        return FALSE;

    //
    // Now give the user the data 
    //
    if( m_FieldMatch[ m_RecordTypeNumber ].UserString == pFieldName )
    {
        data&   Data  = m_pData[ m_FieldMatch[ m_RecordTypeNumber ].Index ];
        field&  Field = m_Record.Field[ m_FieldMatch[ m_RecordTypeNumber ].Index ];
		// Make sure we're giving the correct data.
		if(xstring(pFieldName).Find(xstring(Field.Name)) == -1)
			return FALSE;

        x_va_list   Args;
        x_va_start( Args, pFieldName );

        s32 I=0, F=0, S=0, G=0;

        for( s32 i=0; i<Field.nTypes; i++ )
        {
            switch( Field.Type[i] )
            {
            case TYPE_INTEGER: 
                {
                    s32* p = (va_arg( Args, s32* ));
                    ASSERT(p);
                    *p = Data.Integer[I++]; 
                    break;
                }
            case TYPE_FLOAT:
                {
                    f32* p = (va_arg( Args, f32* ));
                    ASSERT(p);
                    *p= Data.Float[F++]; 
                    break;
                }
            case TYPE_STRING:
                {
                    char* p = (va_arg( Args, char* ));
                    ASSERT(p);
                    x_strcpy( p, Data.String[S++] );
                    break;
                }
            case TYPE_GUID:
                {
                    guid* p = (va_arg( Args, guid* ));
                    ASSERT(p);
                    *p = Data.Guid[G++];
                    break;
                }
            }
        }        
    }
    else
    {
        // Must do a manual search for the field.
        // Right now we don't handle this case.
        return FALSE;
    }

    m_RecordTypeNumber++;

    return TRUE;
}

//=========================================================================

xbool text_in::SkipToNextHeader( void )
{
    s32 i,n;

    // Determine whether we have a count or not
    // if not then we assume that there is only one line
    if( m_Record.Count == -1 )  n = 1;
    else                        n = m_Record.Count;

    // Read all the lines
    for( i = 0; i<n; i++ )
    {
        ReadFields();
    }

    return TRUE;
}

//=========================================================================

xbool text_in::GetVector3( const char* pName, vector3& V )
{
    return GetField( xfs("%s:fff",pName), &V.GetX(), &V.GetY(), &V.GetZ() );
}

//=========================================================================

xbool text_in::GetColor( const char* pName, xcolor& C )
{
    s32 R,G,B,A;
    xbool Result = GetField( xfs("%s:dddd",pName), &R, &G, &B, &A );
    C.R = (byte)(R & 0xFF);
    C.G = (byte)(G & 0xFF);
    C.B = (byte)(B & 0xFF);
    C.A = (byte)(A & 0xFF);
    return Result;
}

//=========================================================================

xbool text_in::GetF32( const char* pName, f32& F )
{
    return GetField( xfs("%s:f",pName), &F );
}

//=========================================================================

xbool text_in::GetS32( const char* pName, s32& I )
{
    return GetField( xfs("%s:d",pName), &I );
}

//=========================================================================

xbool text_in::GetString( const char* pName, char* pStr )
{
    return GetField( xfs("%s:s",pName), pStr );
}

//=========================================================================

xbool text_in::GetBBox( const char* pName, bbox& BBox )
{
    return GetField( xfs("%s:ffffff",pName), 
                     &BBox.Min.GetX(), &BBox.Min.GetY(), &BBox.Min.GetZ(), 
                     &BBox.Max.GetX(), &BBox.Max.GetY(), &BBox.Max.GetZ() );
}

//=========================================================================

xbool text_in::GetRadian3( const char* pName, radian3& Orient )
{
    f32 P,Y,R;
    xbool Result = GetField( xfs("%s:fff",pName), &P, &Y, &R );

    Orient.Pitch = DEG_TO_RAD( P );
    Orient.Yaw   = DEG_TO_RAD( Y );
    Orient.Roll  = DEG_TO_RAD( R );

    return Result;
}

//=========================================================================

xbool text_in::GetQuaternion( const char* pName, quaternion& Q )
{
    return GetField(xfs("%s:ffff",pName),&Q.X,&Q.Y,&Q.Z,&Q.W);
}

//=========================================================================

xbool text_in::GetBool( const char* pName, xbool& Bool )
{
    return GetField(xfs("%s:d",pName),&Bool);
}

//=========================================================================

xbool text_in::GetGuid( const char* pName, guid& Guid )
{
    return GetField(xfs("%s:g",pName),&Guid);
}
