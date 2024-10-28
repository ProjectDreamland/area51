#include <x_files.hpp>

#include "igfmgr.hpp"


//=============================================================================
// TO DO:
//      Right now can't add fields after loading binary without leaking memory.  Fix. (Easy)
//      Tokenizer will only read integers as s32 values and floats as f32

//=============================================================================
// Indent level, for indenting text-format files
static u32 s_IndentLevel = 0;

//=============================================================================
// In case somebody doesn't like 4
#define     INDENT_SIZE     4

//=============================================================================
// Default constructor
igfmgr::igfmgr( )
{
    x_strcpy( m_Header.m_DocName, "" );
    x_strcpy( m_Header.m_Creator, "" );

    m_Header.m_Version  = 1.0f;
    m_pFixup            = NULL;
    m_DictionarySize    = 0;
    m_pLoad             = NULL;
    m_pRoot             = NULL;
    m_ppHead            = &m_pRoot;
    m_pCurGroup         = NULL;
    m_pCursor           = NULL;
    m_pDictionary       = NULL;

    m_GroupStack.SetCapacity(40);   // should be WAY more than you need
    m_GroupStack.SetGrowAmount(10);
}

//=============================================================================
igfmgr::igfmgr( const char* pFileName )
{
    igfmgr();
    Load( pFileName );
}

//=============================================================================
igfmgr::~igfmgr( )
{
    Clear();
}

//=============================================================================
void igfmgr::Clear( )
{
    // not sure why I need "this" in front of destructor call
    u32             i;
    data_entry*     pEntry;
    data_entry*     pNext;

    // delete all dictionary entries
    if ( !m_pLoad )
    {
        pEntry = m_pDictionary;
        for ( i = 0; i < m_DictionarySize; i++ )
        {
            pNext = pEntry->m_pNext;
            IterateFields( NullifyReferences, m_pRoot, (u32)pEntry );
            if ( pEntry->m_pData )
                delete[] pEntry->m_pData;

            delete pEntry;
            pEntry = pNext;
        }

        // now delete all the fields
        IterateFields( DeleteField, m_pRoot, 0 );
    }
    else
        delete[] m_pLoad;

    x_strcpy( m_Header.m_DocName, "" );
    x_strcpy( m_Header.m_Creator, "" );

    // clear vars
    m_Header.m_Version  = 1.0f;
    m_pFixup            = NULL;
    m_DictionarySize    = 0;
    m_pLoad             = NULL;
    m_pRoot             = NULL;
    m_ppHead            = &m_pRoot;
    m_pCursor           = NULL;
    m_pDictionary       = NULL;

}

//=============================================================================
igfmgr::data_entry* igfmgr::AddDataEntry( const void* pData, u32 Length )
{
    data_entry* pEntry = m_pDictionary;
    data_entry* pLast;

    while( pEntry )
    {
        // check for existing entry, simply return if found
        if ( pEntry->m_Length == Length )
        {
            if ( x_memcmp(pEntry->m_pData, pData, Length) == 0 )
            {
                // match found, just return
                return pEntry;
            }
        }

        // remember our last valid entry
        pLast = pEntry;

        pEntry = pEntry->m_pNext;
    }

    // if not the first entry, allocate it, otherwise just use the first entry
    if ( m_pDictionary == NULL )
    {
        m_pDictionary = new data_entry;
        pEntry = m_pDictionary;
    }
    else
    {
        pLast->m_pNext = new data_entry;
        pEntry = pLast->m_pNext;
        
    }

    // add the new entry
    pEntry->m_pData = new u8[Length];
    x_memmove( pEntry->m_pData, pData, Length );
    pEntry->m_pNext = NULL;
    pEntry->m_Length = Length;

    // keep count
    m_DictionarySize++;

    return pEntry;
}

//=============================================================================
igfmgr::data_entry* igfmgr::FindDataEntry( const void* pData, u32 Length )
{
    data_entry* pEntry = m_pDictionary;

    while( pEntry )
    {
        // check for existing entry, simply return if found
        if ( pEntry->m_Length == Length )
        {
            if ( x_memcmp(pEntry->m_pData, pData, Length) == 0 )
            {
                // match found, just return
                return pEntry;
            }
        }

        pEntry = pEntry->m_pNext;
    }

    // not found
    return NULL;
}

//=============================================================================
void igfmgr::SetHeaderInfo(  const char*    pDocName,     
                              const char*   pAuthor,
                              f32           Version )
{
    // set the document name (or NULL)
    if ( pDocName )
    {
        if ( x_strlen(pDocName) < 32 )
        {
            x_strcpy( m_Header.m_DocName, pDocName );
        }
        else
        {
            x_memmove( m_Header.m_DocName, pDocName, 31 );
            m_Header.m_DocName[31] = '\0';
        }
    }

    // set the author (or NULL)
    if ( pAuthor )
    {
        if ( x_strlen(pAuthor) < 32 )
        {
            x_strcpy( m_Header.m_Creator, pAuthor );
        }
        else
        {
            x_memmove( m_Header.m_Creator, pAuthor, 31 );
            m_Header.m_Creator[31] = '\0';
        }
    }

    // version information
    m_Header.m_Version = Version;
   
}

//=============================================================================
xbool igfmgr::LoadBinary( X_FILE* fp )
{
    ASSERT( fp );

    // read the whole thing all at once, then fix up pointers
    u32 Size;
    u32 i;

    x_fseek( fp, 0, X_SEEK_END );
    Size = x_ftell( fp );
    x_fseek( fp, 0, X_SEEK_SET );

    u8* pChunk = new u8[Size];

    if ( !pChunk )
    {
        x_fclose(fp);
        return FALSE;
    }

    // make a pointer to walk thru the data
    u8* pData = pChunk;

    // read in all in one go
    x_fread( pChunk, Size, 1, fp );

    // fixup pointers
    m_pLoad = pData;
    
    // skip past the 'B'
    pData++;

    // first, the header
    m_Header = *((igfmgr::header*)pData);
    pData += sizeof(header);

    // next comes the dictionary entry count
    m_DictionarySize = *((u32*)pData);
    pData += sizeof( u32 );

    // validate dictionary size (must be at least 1)
    if ( !m_DictionarySize )
    {
        x_fclose(fp);
        delete[] pChunk;
        return FALSE;
    }

    // now fixup each entry
    m_pDictionary = (data_entry*)pData;
    data_entry* pNext = m_pDictionary;

    for ( i = 0; i < m_DictionarySize; i++ )
    {
        pData += sizeof( data_entry );
        // set the data pointer
        pNext->m_pData = pData;
        // skip past the data
        pData += pNext->m_Length;
        pNext->m_pNext = (data_entry*)pData;
        pNext = (data_entry*)pData;
    }
    
    // header is all fixed, now read fields
    *m_ppHead = (field*)pData;

    FixupGroup( *m_ppHead );

    return TRUE;
}

//=============================================================================
igfmgr::data_entry* igfmgr::FixupDataEntry( u32 Index )
{
    data_entry* pData = m_pDictionary;
    u32 i;

    // NULL still means NULL!
    if ( !Index )
        return NULL;

    // subtract one from index...(see comment during save binary)
    Index--;

    for ( i = 0; i < m_DictionarySize; i++ )
    {
        if ( i == Index )
            return pData;

        pData = pData->m_pNext;
    }

    return NULL;
}

//=============================================================================
igfmgr::field* igfmgr::FixupGroup( igfmgr::field* pGroup )
{
    u8* pData = (u8*)pGroup;
    field* pPrev = NULL;

    while ( (pGroup->m_Type != igfmgr::GROUP_END) &&
            (pGroup->m_Type != igfmgr::UNDEFINED) )
    {
        // not owned...loaded, so don't try to delete it later
        pGroup->m_IsDataOwned = FALSE;

        // set the data pointer to the data, and move ahead
        pData = ((u8*)pGroup) + sizeof(field);
        pPrev = pGroup;

        if ( (pGroup->m_Type != igfmgr::STRING) &&
             (pGroup->m_Type != igfmgr::BYTE_ARRAY) &&
             (pGroup->m_Type != igfmgr::COMMENT) )
            pGroup->m_pData = (void*)pData;
        else
            pGroup->m_pData = (void*)FixupDataEntry( (u32)pGroup->m_pData );

        // fix the strings
        pGroup->m_pName = FixupDataEntry( (u32)pGroup->m_pName );
        pGroup->m_pComment = FixupDataEntry( (u32)pGroup->m_pComment );
        
        // figure out how far to skip ahead
        igfmgr::atom Type = pGroup->m_Type;

        switch( Type )
        {
            // these 3 already dealt with
            case igfmgr::STRING:            
            case igfmgr::BYTE_ARRAY:
            case igfmgr::COMMENT:
                break;

            case igfmgr::BOOL:              pData += sizeof(xbool);      break;
            case igfmgr::U8:                pData += sizeof(u8);         break;
            case igfmgr::S8:                pData += sizeof(s8);         break;
            case igfmgr::U16:               pData += sizeof(u16);        break;
            case igfmgr::S16:               pData += sizeof(s16);        break;
            case igfmgr::U32:               pData += sizeof(u32);        break;
            case igfmgr::S32:               pData += sizeof(s32);        break;
            case igfmgr::U64:               pData += sizeof(u64);        break;
            case igfmgr::S64:               pData += sizeof(s64);        break;
            case igfmgr::F32:               pData += sizeof(f32);        break;
            case igfmgr::F64:               pData += sizeof(f64);        break;
            case igfmgr::COLOR:             pData += sizeof(xcolor);     break;
            case igfmgr::VECTOR2:           pData += sizeof(vector2);    break;
            case igfmgr::VECTOR3:           pData += sizeof(vector3);    break;
            case igfmgr::RADIAN3:           pData += sizeof(radian3);    break;
            case igfmgr::MATRIX4:           pData += sizeof(matrix4);    break;
            case igfmgr::QUATERNION:        pData += sizeof(quaternion); break;
            case igfmgr::GROUP:             
                {
                    field* pNext;
                    pNext = FixupGroup( (field*)pGroup->m_pData );
                    pGroup->m_pNext = pNext;
                    pGroup = pNext;
                }
                break;
            default:
                pGroup->m_pData = NULL;
                break;
        }
        
        if ( Type != igfmgr::GROUP )
        {
            pGroup->m_pNext = (field*)pData;
            pGroup = (field*)pData;
        }

        pPrev->m_pNext->m_pPrev = pPrev;

    }

    // the terminator fields should not be there after load.  They are generated during
    // save so that the loader can determine where the end of a group is.  Just set the
    // pointer to NULL, it will get deleted when m_pLoad gets deleted.
    if ( pPrev )
        pPrev->m_pNext = NULL;

    // skip terminator
    pGroup++;

    return pGroup;
}

//=============================================================================
void igfmgr::ParseGroup( token_stream& Stream )
{
    // look for tokens, recurse on groups
    char*               pSym;
    xstring             Name;
    xstring             Comment;
    token_stream::type  Type;


    // check for comments (associative and non-associative)
    // non-associative
    while( !Stream.IsEOF() )
    {
        Type = Stream.Read();

        // end of group?
        pSym = Stream.String();

        if ( x_strcmp( pSym, "}" ) == 0 )
        {
            return;
        }

        if ( Type = token_stream::TOKEN_SYMBOL )
        {
            //---------------------------------------------------------------------
            if ( pSym[0] == '#' )
            {
                pSym = Stream.ReadToSymbol('\r');
                AddComment( pSym );
            }        
            //---------------------------------------------------------------------
            else if ( pSym[0] == '[' )
            {
                pSym = Stream.ReadToSymbol( ']' );
                Comment = pSym;
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "BOOL") == 0 )
            {
                xbool tf;

                Name = Stream.ReadString();
                pSym = Stream.ReadSymbol();
                if ( x_strcmp( pSym, "TRUE" ) == 0 )
                    tf =  TRUE;
                else
                if ( x_strcmp( pSym, "FALSE" ) == 0 )
                    tf = FALSE;
                else
                {
                    ASSERT( FALSE );
                }

                AddBool( Name, tf, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "U8") == 0 )
            {
                s32 Val;
                Name = Stream.ReadString();
                Val = Stream.ReadInt();
                AddU8( Name, Val, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "S8") == 0 )
            {
                s32 Val;
                Name = Stream.ReadString();
                Val = Stream.ReadInt();
                AddS8( Name, Val, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "U16") == 0 )
            {
                s32 Val;
                Name = Stream.ReadString();
                Val = Stream.ReadInt();
                AddU16( Name, Val, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "S16") == 0 )
            {
                s32 Val;
                Name = Stream.ReadString();
                Val = Stream.ReadInt();
                AddS16( Name, Val, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "U32") == 0 )
            {
                s32 Val;
                Name = Stream.ReadString();
                Val = Stream.ReadInt();
                AddU32( Name, Val, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "S32") == 0 )
            {
                s32 Val;
                Name = Stream.ReadString();
                Val = Stream.ReadInt();
                AddS32( Name, Val, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "U64") == 0 )
            {
                s32 Val;
                Name = Stream.ReadString();
                Val = Stream.ReadInt();
                AddU64( Name, Val, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "S64") == 0 )
            {
                s32 Val;
                Name = Stream.ReadString();
                Val = Stream.ReadInt();
                AddS64( Name, Val, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "F32") == 0 )
            {
                f32 Val;
                Name = Stream.ReadString();
                Val = Stream.ReadFloat();
                AddF32( Name, Val, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "F64") == 0 )
            {
                f64 Val;
                Name = Stream.ReadString();
                Val = Stream.ReadFloat();
                AddF64( Name, Val, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "COLOR") == 0 )
            {
                Name = Stream.ReadString();
                xcolor Color;
                Color.R = Stream.ReadInt();
                Color.G = Stream.ReadInt();
                Color.B = Stream.ReadInt();
                Color.A = Stream.ReadInt();
                AddColor( Name, Color, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "STRING") == 0 )
            {
                Name = Stream.ReadString();
                AddString( Name, Stream.ReadString(), Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "BYTE_ARRAY") == 0 )
            {
                s32 Count;
                u8* pData;
                s32 i;

                Name = Stream.ReadString();
                
                // find out how many elements
                pSym = Stream.ReadSymbol();
                // skip the 'x'
                pSym++;
                Count = x_atoi(pSym);
                // allocate
                pData = new u8[Count];

                for ( i = 0; i < Count; i++ )
                    pData[i] = Stream.ReadHex();

                AddByteArray( Name, pData, Count, Comment );
                delete[] pData;
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "VECTOR2") == 0 )
            {
                Name = Stream.ReadString();
                vector2 V;
                V.X = Stream.ReadFloat();
                V.Y = Stream.ReadFloat();
                AddV2( Name, V, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "VECTOR3") == 0 )
            {
                Name = Stream.ReadString();
                vector3 V;
                V.GetX() = Stream.ReadFloat();
                V.GetY() = Stream.ReadFloat();
                V.GetZ() = Stream.ReadFloat();
                AddV3( Name, V, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "RADIAN3") == 0 )
            {
                Name = Stream.ReadString();
                radian3 R;
                R.Pitch = Stream.ReadFloat();
                R.Yaw   = Stream.ReadFloat();
                R.Roll  = Stream.ReadFloat();
                AddR3( Name, R, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "MATRIX4") == 0 )
            {
                Name = Stream.ReadString();
                matrix4 M;
                s32 i, j;

                for ( j = 0; j < 4; j++ )
                {
                    for ( i = 0; i < 4; i++ )
                    {
                        M(i,j) = Stream.ReadFloat();
                    }
                }

                AddM4( Name, M, Comment );
                Comment = "";            
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "QUATERNION") == 0 )
            {
                Name = Stream.ReadString();
                quaternion Q;

                Q.X = Stream.ReadFloat();
                Q.Y = Stream.ReadFloat();
                Q.Z = Stream.ReadFloat();
                Q.W = Stream.ReadFloat();

                AddQuat( Name, Q, Comment );
                Comment = "";
            }
            //---------------------------------------------------------------------
            else if ( x_strcmp(pSym, "GROUP") == 0 )
            {
                Name = Stream.ReadString();
                hfield hField;
                hField = AddGroup( Name, Comment );

                // remember current group
                //field* pCur = (field*)*m_ppHead;
                //SetGroup( hField );
                EnterGroup( hField );
                
                // recurse group
                ParseGroup( Stream );
                // SetGroup( (hfield)pCur );
                ExitGroup();
                Comment = "";
            }
        }
    }
}

//=============================================================================
xbool igfmgr::Load( const char *pFileName )
{
    X_FILE*     fp;
    char        Mode;

    fp = x_fopen( pFileName, "rb" );

    if ( !fp )
        return FALSE;
    
    x_fread( &Mode, 1, 1, fp );

    switch( Mode )
    {
        case 'B':       
            return LoadBinary( fp );       

        case 'T':
            {                
                x_fclose(fp);
                // open the token stream
                token_stream Stream;
                Stream.OpenFile( pFileName );
                
                // get the header information first

                // DOCUMENT NAME
                if ( Stream.Find( "Document:" ) )
                {
                    char *pDocName = Stream.ReadString();

                    if ( pDocName )
                        x_strcpy( m_Header.m_DocName, pDocName );
                    else
                        return FALSE;
                }
                else
                    return FALSE;

                // DOCUMENT OWNER
                if ( Stream.Find( "Owner:" ) )
                {
                    char *pOwner = Stream.ReadString();

                    if ( pOwner )
                        x_strcpy( m_Header.m_Creator, pOwner );
                    else
                        return FALSE;
                }
                else
                    return FALSE;

                // DOCUMENT VERSION
                if ( Stream.Find( "Version:" ) )
                {
                    m_Header.m_Version = Stream.ReadFloat();
                }
                else
                    return FALSE;

                ParseGroup( Stream );
            }
            break;

        default:
            // invalid file
            x_fclose(fp);
            return FALSE;
    }   

    return TRUE;
}

//=============================================================================
void igfmgr::NullifyReferences( field* pField, u32 Data )
{
    data_entry* pData = (data_entry*)Data;

    if ( !pField )
        return;

    // set any references to data to NULL, 
    // because they have been deleted already.
    if ( pField->m_pData == pData )
    {
        pField->m_IsDataOwned = FALSE;
        pField->m_pData = NULL;
    }

    pField->m_pName = NULL;
    pField->m_pComment = NULL;
  
}

//=============================================================================
void igfmgr::DeleteField( field* pField, u32 Data )
{
    if ( !pField )
        return ;

    // we exclude m_pData if pField is a GROUP, because m_pData
    // will get nuked as the group is entered.
    if ( (pField->m_IsDataOwned) && 
         (pField->m_pData) && 
         (pField->m_Type != igfmgr::GROUP) )
        delete pField->m_pData;

    delete pField;
}

//=============================================================================
void igfmgr::AddRefCt_NoComments( field* pField, u32 Data )
{
    ASSERT( m_pFixup );
    (void)Data;

    // don't care about end of group
    if ( !pField )
        return;

    // if it's a comment, just return
    if ( pField->m_Type == igfmgr::COMMENT )
        return;

    for ( u32 i = 0; i < m_DictionarySize; i++ )
    {
        if ( m_pFixup[i].m_pEntry == (data_entry*)pField->m_pName )
            m_pFixup[i].m_RefCt++;
        else
        if ( m_pFixup[i].m_pEntry == (data_entry*)pField->m_pData )
            m_pFixup[i].m_RefCt++;
    }
}

//=============================================================================
void igfmgr::AddRefCt_WithComments( field* pField, u32 Data )
{
    ASSERT( m_pFixup );
    (void)Data;

    // don't care about end of group
    if ( !pField )
        return;

    for ( u32 i = 0; i < m_DictionarySize; i++ )
    {
        if ( m_pFixup[i].m_pEntry == (data_entry*)pField->m_pName )
            m_pFixup[i].m_RefCt++;
        else        
        if ( m_pFixup[i].m_pEntry == (data_entry*)pField->m_pComment )
            m_pFixup[i].m_RefCt++;
        else
        if ( m_pFixup[i].m_pEntry == (data_entry*)pField->m_pData )
            m_pFixup[i].m_RefCt++;
    }
}

//=============================================================================
igfmgr::fixup_map* igfmgr::FindFixupEntry( data_entry* pEntry )
{
    u32 i;

    for ( i = 0; i < m_DictionarySize; i++ )
    {
        if ( m_pFixup[i].m_pEntry == pEntry )
            return &m_pFixup[i];
    }
    
    // not found
    return NULL;
}

//=============================================================================
void igfmgr::SaveFields_Binary( field* pField, u32 Data )
{
    ASSERT( Data   );

    // Data is actually a file pointer
    X_FILE* fp = (X_FILE*)Data;

    // first, check the end-of-group condition (null pField)
    if ( !pField )
    {
        field NewField;
        NewField.m_Type = igfmgr::GROUP_END;
        NewField.m_IsDataOwned = FALSE;
        NewField.m_pNext = NULL;
        NewField.m_pPrev = NULL;
        NewField.m_pData = NULL;
        NewField.m_pName = NULL;
        NewField.m_pComment = NULL;
        x_fwrite( &NewField, sizeof(field), 1, fp );
        return;
    }

    // if it is a comment, but comment is NULL, don't write at all
    if ( (pField->m_Type == igfmgr::COMMENT) &&
         (pField->m_pComment == NULL) )
         return;

    // Make duplicate
    field NewField = *pField;
    NewField.m_IsDataOwned = FALSE;

    // fixup the dictionary entries (convert the pointers to indices)
    if ( NewField.m_pComment )
    {
        fixup_map* pFix = FindFixupEntry( NewField.m_pComment );
        ASSERT( pFix );
        NewField.m_pComment = (data_entry*)pFix->m_Index;
    }
    if ( NewField.m_pName )
    {
        fixup_map* pFix = FindFixupEntry( NewField.m_pName );
        ASSERT( pFix );
        NewField.m_pName = (data_entry*)pFix->m_Index;
    }
    if ( (NewField.m_Type == igfmgr::BYTE_ARRAY) ||
         (NewField.m_Type == igfmgr::STRING) )
    {
        ASSERT( NewField.m_pData );
        fixup_map* pFix = FindFixupEntry( (data_entry*)NewField.m_pData );
        ASSERT( pFix );
        NewField.m_pData = (data_entry*)pFix->m_Index;
    }

    // write the prepared field entry
    x_fwrite( &NewField, sizeof(field), 1, fp );

    // Now append the actual data for that field immediately afterward
    switch( pField->m_Type )
    {
        case igfmgr::UNDEFINED:                                                                    break;
        case igfmgr::COMMENT:                                                                      break;
        case igfmgr::STRING:                                                                       break;
        case igfmgr::BYTE_ARRAY:                                                                   break;
        case igfmgr::BOOL:             x_fwrite( NewField.m_pData, sizeof(xbool), 1, fp );         break;
        case igfmgr::U8:               x_fwrite( NewField.m_pData, sizeof(u8), 1, fp );            break;
        case igfmgr::S8:               x_fwrite( NewField.m_pData, sizeof(s8), 1, fp );            break;
        case igfmgr::U16:              x_fwrite( NewField.m_pData, sizeof(u16), 1, fp );           break;
        case igfmgr::S16:              x_fwrite( NewField.m_pData, sizeof(s16), 1, fp );           break;
        case igfmgr::U32:              x_fwrite( NewField.m_pData, sizeof(u32), 1, fp );           break;
        case igfmgr::S32:              x_fwrite( NewField.m_pData, sizeof(s32), 1, fp );           break;
        case igfmgr::U64:              x_fwrite( NewField.m_pData, sizeof(u64), 1, fp );           break;
        case igfmgr::S64:              x_fwrite( NewField.m_pData, sizeof(s64), 1, fp );           break;
        case igfmgr::F32:              x_fwrite( NewField.m_pData, sizeof(f32), 1, fp );           break;
        case igfmgr::F64:              x_fwrite( NewField.m_pData, sizeof(f64), 1, fp );           break;
        case igfmgr::COLOR:            x_fwrite( NewField.m_pData, sizeof(xcolor), 1, fp );        break;
        case igfmgr::VECTOR2:          x_fwrite( NewField.m_pData, sizeof(vector2), 1, fp );       break;
        case igfmgr::VECTOR3:          x_fwrite( NewField.m_pData, sizeof(vector3), 1, fp );       break;
        case igfmgr::RADIAN3:          x_fwrite( NewField.m_pData, sizeof(radian3), 1, fp );       break;
        case igfmgr::MATRIX4:          x_fwrite( NewField.m_pData, sizeof(matrix4), 1, fp );       break;
        case igfmgr::QUATERNION:       x_fwrite( NewField.m_pData, sizeof(quaternion), 1, fp );    break;
        case igfmgr::GROUP:            /*x_fwrite( NewField.m_pData, sizeof(field), 1, fp );*/     break;
        default:
            ASSERT( FALSE );
            break;
    }    
}

//=============================================================================
void igfmgr::SaveFields_Text_NoComments( field* pField, u32 Data )
{
    // strip out any comments and pass it on to SaveFields_Text_WithComments
    if ( pField )
    {
        field NewField = *pField;

        NewField.m_pComment = NULL;

        if ( NewField.m_Type != igfmgr::COMMENT )
            SaveFields_Text_WithComments( &NewField, Data );
    }
    else
        SaveFields_Text_WithComments( pField, Data );
    
}

//=============================================================================
void igfmgr::SaveFields_Text_WithComments( field* pField, u32 Data )
{   
    X_FILE* fp = (X_FILE*)Data;
    char* pName;
    char* pComment;
    char* pString;

    // check for end of group condition
    if ( !pField )
    {
        s_IndentLevel -= INDENT_SIZE;
        x_fprintf( fp, "%*.s}\n\n", s_IndentLevel, " " );
        return;
    }

    // otherwise, find relevant information to store in the file
    if ( pField->m_pName )
        pName = (char*)pField->m_pName->m_pData;
    else
        pName = "";

    if ( pField->m_pComment )
        pComment = (char*)pField->m_pComment->m_pData;
    else
        pComment = "";

    if ( pField->m_Type == igfmgr::STRING )
    {
        if ( pField->m_pData )
            pString = (char*)((data_entry*)pField->m_pData)->m_pData;
        else
            pString = "";
    }

    // print item comments first (excluding actual comment type)
    // this is differentiated to allow comments to be specifically associated with fields
    if ( pField->m_Type != igfmgr::COMMENT )
    {
        if ( x_strlen(pComment) )
            x_fprintf( fp, "%*.s[ %s ]\n", s_IndentLevel, " ", pComment );
    }

    // write out data based on the atom type
    switch ( pField->m_Type )
    {
        case igfmgr::UNDEFINED:  
            break;
        case igfmgr::COMMENT:
            // don't write anything if requested to strip comments
            if ( x_strlen(pComment) )
                x_fprintf( fp, "# %s", pComment );
            break;
        case igfmgr::STRING:     
            x_fprintf( fp, "%*.sSTRING \"%s\" \"%s\"", s_IndentLevel, " ", pName, pString );
            break;
        case igfmgr::BYTE_ARRAY: 
            x_fprintf( fp, "%*.sBYTE_ARRAY \"%s\"", s_IndentLevel, " ", pName, pString );
            {
                data_entry* pData = (data_entry*)pField->m_pData;
                u8* pBytes = (u8*)(pData->m_pData);
                u32 i, j = pData->m_Length;
                u32 k = 0;

                x_fprintf( fp, "x%d \n", pData->m_Length );
                x_fprintf( fp, "%*.s  ",   s_IndentLevel + 4, " " );
                for ( i = 0; i < j; i++ )
                {
                    x_fprintf( fp, "0x%-2.2X ", pBytes[k++] );

                    if ( !(k%8) )
                        x_fprintf( fp, "\n%*.s  ",   s_IndentLevel + 4, " " );
                }
            }
            break;
        case igfmgr::BOOL:         
            {
                xbool* pBool = (xbool*)pField->m_pData;
                x_fprintf( fp, "%*.sBOOL \"%s\" ", s_IndentLevel, " ", pName );
                if ( *pBool == TRUE )
                    x_fprintf( fp, "TRUE" );
                else
                    x_fprintf( fp, "FALSE" );
            }
            break;
        case igfmgr::U8:         
            x_fprintf( fp, "%*.sU8 \"%s\" %d", s_IndentLevel, " ", pName, *((u8*)pField->m_pData) );
            break;
        case igfmgr::S8:         
            x_fprintf( fp, "%*.sS8 \"%s\" %d", s_IndentLevel, " ", pName, *((s8*)pField->m_pData) );
            break;
        case igfmgr::U16:        
            x_fprintf( fp, "%*.sU16 \"%s\" %d", s_IndentLevel, " ", pName, *((u16*)pField->m_pData) );
            break;
        case igfmgr::S16:        
            x_fprintf( fp, "%*.sS16 \"%s\" %d", s_IndentLevel, " ", pName, *((s16*)pField->m_pData) );
            break;
        case igfmgr::U32:        
            x_fprintf( fp, "%*.sU32 \"%s\" %d", s_IndentLevel, " ", pName, *((u32*)pField->m_pData) );
            break;
        case igfmgr::S32:        
            x_fprintf( fp, "%*.sS32 \"%s\" %d", s_IndentLevel, " ", pName, *((s32*)pField->m_pData) );
            break;
        case igfmgr::U64:        
            x_fprintf( fp, "%*.sU64 \"%s\" %d", s_IndentLevel, " ", pName, *((u64*)pField->m_pData) );
            break;
        case igfmgr::S64:        
            x_fprintf( fp, "%*.sS64 \"%s\" %d", s_IndentLevel, " ", pName, *((s64*)pField->m_pData) );
            break;
        case igfmgr::F32:        
            x_fprintf( fp, "%*.sF32 \"%s\" %f", s_IndentLevel, " ", pName, *((f32*)pField->m_pData) );
            break;
        case igfmgr::F64:        
            x_fprintf( fp, "%*.sF64 \"%s\" %f", s_IndentLevel, " ", pName, *((f64*)pField->m_pData) );
            break;
        case igfmgr::COLOR:      
            x_fprintf( fp, "%*.sCOLOR \"%s\" ", s_IndentLevel, " ", pName );
            {
                xcolor* pColor = (xcolor*)pField->m_pData;
                x_fprintf( fp, "%d %d %d %d", pColor->R, pColor->G, pColor->B, pColor->A );
            }
            break;
        case igfmgr::VECTOR2:    
            x_fprintf( fp, "%*.sVECTOR2 \"%s\" ", s_IndentLevel, " ", pName );
            {
                vector2* pVec2 = (vector2*)pField->m_pData;
                x_fprintf( fp, " %f %f", pVec2->X, pVec2->Y );
            }
            break;
        case igfmgr::VECTOR3:    
            x_fprintf( fp, "%*.sVECTOR3 \"%s\" ", s_IndentLevel, " ", pName );
            {
                vector3* pVec3 = (vector3*)pField->m_pData;
                x_fprintf( fp, "%f %f %f", pVec3->GetX(), pVec3->GetY(), pVec3->GetZ() );
            }
            break;
        case igfmgr::RADIAN3:    
            x_fprintf( fp, "%*.sRADIAN3 \"%s\" ", s_IndentLevel, " ", pName );
            {
                radian3* pRad3 = (radian3*)pField->m_pData;
                x_fprintf( fp, "%f %f %f", pRad3->Pitch, pRad3->Yaw, pRad3->Roll );
            }
            break;
        case igfmgr::MATRIX4:    
            x_fprintf( fp, "%*.sMATRIX4 \"%s\" \n", s_IndentLevel, " ", pName );
            {
                matrix4* pMat4 = (matrix4*)pField->m_pData;
                x_fprintf( fp, "%*.s  %f %f %f %f\n",   s_IndentLevel + 4, " " , (*pMat4)(0,0), (*pMat4)(1,0), (*pMat4)(2,0), (*pMat4)(3,0) );
                x_fprintf( fp, "%*.s  %f %f %f %f\n",   s_IndentLevel + 4, " " , (*pMat4)(0,1), (*pMat4)(1,1), (*pMat4)(2,1), (*pMat4)(3,1) );
                x_fprintf( fp, "%*.s  %f %f %f %f\n",   s_IndentLevel + 4, " " , (*pMat4)(0,2), (*pMat4)(1,2), (*pMat4)(2,2), (*pMat4)(3,2) );
                x_fprintf( fp, "%*.s  %f %f %f %f  ", s_IndentLevel + 4, " " , (*pMat4)(0,3), (*pMat4)(1,3), (*pMat4)(2,3), (*pMat4)(3,3) );
            }
            break;
        case igfmgr::QUATERNION: 
            x_fprintf( fp, "%*.sQUATERNION \"%s\" ", s_IndentLevel, " ", pName );
            {
                quaternion* pQuat = (quaternion*)pField->m_pData;
                x_fprintf( fp, "%f %f %f %f", pQuat->X, pQuat->Y, pQuat->Z, pQuat->W );
            }
            break;
        case igfmgr::GROUP:      
            x_fprintf( fp, "%*.sGROUP \"%s\"\n%*.s{", s_IndentLevel, " ", pName, s_IndentLevel, " " );
            s_IndentLevel += INDENT_SIZE;
            break;
        default:
            ASSERT( FALSE );
            break;
    }

    x_fprintf( fp, "\n\n" );
}

//=============================================================================
void igfmgr::IterateFields( iterate_fn IterateFn, field* pField, u32 Data )
{
    // walk all fields calling iterate function for each field
    while( pField )
    {
        // store off information in case pField gets nuked during callback
        field*         pFieldData = (field*)pField->m_pData;
        field*         pFieldNext = pField->m_pNext;
        igfmgr::atom   Type       = pField->m_Type;

        // call the callback first
        (this->*IterateFn)(pField, Data);

        // don't rely on pField henceforth...might be nuked.  Use pFieldData instead.
        if ( Type == igfmgr::GROUP )
        {
            // should never be a case where you have a group without at least one node.
            // ASSERT( pFieldData );            

            // update indent, in case we're saving (doesn't hurt anything otherwise)
            if ( pFieldData )
            {
                IterateFields( IterateFn, (field*)pFieldData, Data );
                // special treatment...call the IterateFn with a NULL pField to let it know 
                // that a group just closed
                (this->*IterateFn)(NULL, Data);
            }
        }

        // move on to the next one
        pField = pFieldNext;
    }
}

//=============================================================================
xbool igfmgr::SaveBinary( const char* pFileName, xbool StripComments ) 
{
    X_FILE*     fp;
    u32         i;
    u32         Count = 0;
    data_entry* pEntry;
    field       End;

    fp = x_fopen( pFileName, "wb" );

    if ( !fp )
        goto FAIL;

    // write the type flag (B for Binary)
    x_fwrite( "B", 1, 1, fp );

    // write the header
    x_fwrite( &m_Header, sizeof(igfmgr::header), 1, fp );

    // allocate the fixup map
    ASSERT( m_pFixup == NULL );

    m_pFixup = new fixup_map[m_DictionarySize];
    if ( !m_pFixup )
        goto FAIL;
    
    x_memset( m_pFixup, 0, sizeof(fixup_map)*m_DictionarySize );

    // copy the dictionary pointers into the fixup array
    pEntry = m_pDictionary;

    for ( i = 0; i < m_DictionarySize; i++ )
    {
        m_pFixup[i].m_pEntry = pEntry;
        pEntry = pEntry->m_pNext;
    }

    // step thru all the data fields
    if ( StripComments )
        IterateFields( AddRefCt_NoComments, m_pRoot, 0 );
    else
        IterateFields( AddRefCt_WithComments, m_pRoot, 0 );

    // count the dictionary entries that have at least one ref ct.
    for ( i = 0; i < m_DictionarySize; i++ )
    {
        if ( m_pFixup[i].m_RefCt )
        {
            // record the index
            m_pFixup[i].m_Index = ++Count;      // start with 1, so fixup on load doesn't get confused
        }
    }

    // write the count of dictionary entries
    x_fwrite( &Count, sizeof(u32), 1, fp );

    // write the dictionary entries that have at least one ref ct.
    for ( i = 0; i < m_DictionarySize; i++ )
    {
        if ( m_pFixup[i].m_RefCt )
        {
            // first write the entry
            x_fwrite( m_pFixup[i].m_pEntry, sizeof(igfmgr::data_entry), 1, fp );
            // followed by the actual data, which will be fixed up at load time
            x_fwrite( m_pFixup[i].m_pEntry->m_pData, m_pFixup[i].m_pEntry->m_Length, 1, fp );
        }
    }

    // Now simply write all the data
    IterateFields( SaveFields_Binary, m_pRoot, (u32)fp );

    // write a delimiter field
    End.m_Type = igfmgr::UNDEFINED;
    End.m_IsDataOwned = FALSE;
    End.m_pName = NULL;
    End.m_pData = NULL;
    End.m_pComment = NULL;
    x_fwrite( &End, sizeof(field), 1, fp );

    // Done, close and cleanup
    x_fclose(fp);

    delete[] m_pFixup;
    m_pFixup = NULL;

    return TRUE;

FAIL:
    // clean up before exiting
    if ( m_pFixup )
    {
        delete[] m_pFixup;
        m_pFixup = NULL;
    }

    if ( fp )
        x_fclose(fp);

    return FALSE;
}

//=============================================================================
xbool igfmgr::SaveText( const char* pFileName, xbool StripComments ) 
{
    X_FILE*     fp;
    u32         Count = 0;

    fp = x_fopen( pFileName, "wt" );

    if ( !fp )
        goto FAIL;

    x_fprintf( fp, "Text Format .IGF File\n\n" );

    x_fprintf( fp, "Document: \"%s\"\n", m_Header.m_DocName );
    x_fprintf( fp, "Owner: \"%s\"\n", m_Header.m_Creator );
    x_fprintf( fp, "Version: %f\n\n", m_Header.m_Version );

    // simply iterate thru the fields writing as you go
    s_IndentLevel = 0;
    
    if ( !StripComments )
        IterateFields( SaveFields_Text_WithComments, m_pRoot, (u32)fp );
    else
        IterateFields( SaveFields_Text_NoComments, m_pRoot, (u32)fp );

    // close the file and return
    x_fclose(fp);

    return TRUE;

FAIL:
    if ( fp )
        x_fclose(fp);

    return FALSE;
}

//=============================================================================
xbool igfmgr::Save( const char* pFileName, xbool IsBinary, xbool StripComments ) 
{
    if ( IsBinary )
        return SaveBinary( pFileName, StripComments );
    else
        return SaveText  ( pFileName, StripComments );    
}

//=============================================================================
// DATA MINING (AND PROSPECTING)
//=============================================================================

//=============================================================================
igfmgr::atom igfmgr::GetType( void ) const
{
    ASSERT( m_pCursor );
    return m_pCursor->m_Type;
}

//=============================================================================
xbool igfmgr::GetBool( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::BOOL );
    return *((xbool*)m_pCursor->m_pData);
}

//=============================================================================
u8 igfmgr::GetU8( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::U8 );
    return *((u8*)m_pCursor->m_pData);
}

//=============================================================================
s8 igfmgr::GetS8( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::S8 );
    return *((s8*)m_pCursor->m_pData);
}

//=============================================================================
u16 igfmgr::GetU16( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::U16 );
    return *((u16*)m_pCursor->m_pData);
}

//=============================================================================
s16 igfmgr::GetS16( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::S16 );
    return *((s16*)m_pCursor->m_pData);
}

//=============================================================================
u32 igfmgr::GetU32( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::U32 );
    return *((u32*)m_pCursor->m_pData);
}

//=============================================================================
s32 igfmgr::GetS32( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::S32 );
    return *((s32*)m_pCursor->m_pData);
}

//=============================================================================
u64 igfmgr::GetU64( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::U64 );
    return *((u64*)m_pCursor->m_pData);
}

//=============================================================================
s64 igfmgr::GetS64( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::S64 );
    return *((s64*)m_pCursor->m_pData);
}

//=============================================================================
f32 igfmgr::GetF32( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::F32 );
    return *((f32*)m_pCursor->m_pData);
}

//=============================================================================
f64 igfmgr::GetF64( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::F64 );
    return *((f64*)m_pCursor->m_pData);
}

//=============================================================================
const char* igfmgr::GetString( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::STRING );
    return (const char*)(((data_entry*)m_pCursor->m_pData)->m_pData);
}

//=============================================================================
const char* igfmgr::GetByteArray( u32& ByteCount ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::BYTE_ARRAY );
    ByteCount = ((data_entry*)m_pCursor->m_pData)->m_Length;
    return (const char*)(((data_entry*)m_pCursor->m_pData)->m_pData);
}

//=============================================================================
const xcolor& igfmgr::GetColor( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::COLOR );
    return *((xcolor*)m_pCursor->m_pData);
}

//=============================================================================
const vector2& igfmgr::GetV2( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::VECTOR2 );
    return *((vector2*)m_pCursor->m_pData);
}

//=============================================================================
const vector3& igfmgr::GetV3( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::VECTOR3 );
    return *((vector3*)m_pCursor->m_pData);
}

//=============================================================================
const radian3& igfmgr::GetRad( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::RADIAN3 );
    return *((radian3*)m_pCursor->m_pData);
}

//=============================================================================
const matrix4& igfmgr::GetM4( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::MATRIX4 );
    return *((matrix4*)m_pCursor->m_pData);
}

//=============================================================================
const quaternion& igfmgr::GetQuat( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::QUATERNION );
    return *((quaternion*)m_pCursor->m_pData);
}

//=============================================================================
hfield igfmgr::GetGroup ( void ) const
{
    ASSERT( m_pCursor->m_Type == igfmgr::GROUP );
    return (hfield)((field*)m_pCursor);
}

//=============================================================================
// COMBINING FINDING AND RETRIEVING ACTUAL VALUES
//=============================================================================

xbool igfmgr::GetBool( const char* pName )
{
    Find(pName);
    return GetBool();
}

//=============================================================================

u8 igfmgr::GetU8( const char* pName )
{
    Find(pName);
    return GetU8();
}

//=============================================================================
s8 igfmgr::GetS8( const char* pName )
{
    Find(pName);
    return GetS8();
}

//=============================================================================
u16 igfmgr::GetU16( const char* pName )
{
    Find(pName);
    return GetU16();
}

//=============================================================================
s16 igfmgr::GetS16( const char* pName )
{
    Find(pName);
    return GetS16();
}

//=============================================================================
u32 igfmgr::GetU32( const char* pName )
{
    Find(pName);
    return GetU32();
}

//=============================================================================
s32 igfmgr::GetS32( const char* pName )
{
    Find(pName);
    return GetS32();
}

//=============================================================================
u64 igfmgr::GetU64( const char* pName )
{
    Find(pName);
    return GetU64();
}

//=============================================================================
s64 igfmgr::GetS64( const char* pName )
{
    Find(pName);
    return GetS64();
}

//=============================================================================
f32 igfmgr::GetF32( const char* pName )
{
    Find(pName);
    return GetF32();
}

//=============================================================================
f64 igfmgr::GetF64( const char* pName )
{
    Find(pName);
    return GetF64();
}

//=============================================================================
const char* igfmgr::GetString( const char* pName )
{
    Find(pName);
    return GetString();
}

//=============================================================================
const char* igfmgr::GetByteArray( const char* pName, u32& ByteCount )
{
    Find(pName);
    return GetByteArray(ByteCount);
}

//=============================================================================
const xcolor& igfmgr::GetColor( const char* pName )
{
    Find(pName);
    return GetColor();
}

//=============================================================================
const vector2& igfmgr::GetV2( const char* pName )
{
    Find(pName);
    return GetV2();
}

//=============================================================================
const vector3& igfmgr::GetV3( const char* pName )
{
    Find(pName);
    return GetV3();
}

//=============================================================================
const radian3& igfmgr::GetRad( const char* pName )
{
    Find(pName);
    return GetRad();
}

//=============================================================================
const matrix4& igfmgr::GetM4( const char* pName )
{
    Find(pName);
    return GetM4();
}

//=============================================================================
const quaternion& igfmgr::GetQuat( const char* pName )                  
{
    Find(pName);
    return GetQuat();
}

//=============================================================================
hfield igfmgr::GetGroup( const char* pName )
{
    if ( Find(pName) )
        return GetGroup();
    else
        return BAD_HFIELD;
}

//=============================================================================
// CREATING DATA FROM SCRATCH (ADD DATA AFTER HFIELD, RETURN THE NEW HFIELD)
//=============================================================================

//=============================================================================
// The private function for adding a field
igfmgr::field* igfmgr::AddField( const char* pName, const char* pComment, hfield hField )
{
    // Allocate the new field!
    field* pField = new field;

    // check for this being an insertion rather than an append
    field* pInsert = NULL;

    // check for empty comments
    if ( pComment && x_strlen(pComment) == 0 )
        pComment = NULL;
    
    if ( m_pCursor )
    {
        if ( m_pCursor->m_pNext )
            pInsert = m_pCursor;
    }
    
    // if hField is not null, that supercedes above
    if ( hField )
    {
        pInsert = (field*)hField;
    }

    // setup name and comment
    if ( pName )
    {
        pField->m_pName = AddDataEntry( pName, x_strlen(pName) + 1 );
    }
    else
        pField->m_pName = NULL;

    if ( pComment )
    {
        pField->m_pComment = AddDataEntry( pComment, x_strlen(pComment) + 1 );
    }
    else
        pField->m_pComment = NULL;

    // Also, we know we own the data
    pField->m_IsDataOwned = TRUE;
    
    // Next ptr is NULL
    pField->m_pNext = NULL;

    // now figure out where to put it

    // first field in the current group, just set it as the first
    if ( *m_ppHead == NULL )
    {
        *m_ppHead = pField;
        pField->m_pPrev = NULL;
    }
    else
    {
        if ( pInsert != NULL )
        {
            // should definitely have a cursor here
            ASSERT( m_pCursor );
        
            // should it be simply inserted? (next is not NULL, so must be somewhere in the middle)
            if ( pInsert->m_pNext != NULL )
            {
                field* pNext = pInsert->m_pNext;
                field* pPrev = pInsert->m_pPrev;
            
                // insert at very beginning?
                if ( pPrev == NULL )
                {
                    field* pOldHead = *m_ppHead;
                    *m_ppHead = pField;
                    pField->m_pNext = pOldHead;
                    pField->m_pPrev = NULL;
                    pOldHead->m_pPrev = pField;
                    // pOldHead->m_pNext doesn't change
                }
                else
                {
                    pInsert->m_pPrev = pField;
                    pField->m_pNext = pInsert;
                }
            }
        }
        else
        {
            m_pCursor->m_pNext = pField;
            pField->m_pPrev = m_pCursor;
        }
    }

    m_pCursor = pField;

    // return it
    return pField;
}

//=============================================================================

hfield igfmgr::AddBool( const char* pName, xbool Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::BOOL;
    pField->m_pData = new xbool;
    *((xbool*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================

hfield igfmgr::AddU8( const char* pName, u8  Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::U8;
    pField->m_pData = new u8;
    *((u8*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddS8( const char* pName, s8  Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::S8;
    pField->m_pData = new s8;
    *((s8*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddU16( const char* pName, u16 Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::U16;
    pField->m_pData = new u16;
    *((u16*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddS16( const char* pName, s16 Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::S16;
    pField->m_pData = new s16;
    *((s16*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddU32( const char* pName, u32 Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::U32;
    pField->m_pData = new u32;
    *((u32*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddS32( const char* pName, s32 Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::S32;
    pField->m_pData = new s32;
    *((s32*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddU64( const char* pName, u64 Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::U64;
    pField->m_pData = new u64;
    *((u64*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddS64( const char* pName, s64 Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::S64;
    pField->m_pData = new s64;
    *((s64*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddF32( const char* pName, f32 Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::F32;
    pField->m_pData = new f32;
    *((f32*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddF64( const char* pName, f64 Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::F64;
    pField->m_pData = new f64;
    *((f64*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddColor( const char* pName, xcolor& Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::COLOR;
    pField->m_pData = new xcolor;
    *((xcolor*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddString( const char* pName, const char* pStr, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::STRING;
    pField->m_pData = AddDataEntry( pStr, x_strlen(pStr) + 1 );

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddByteArray( const char* pName, const u8* pBytes, u32 ByteCount, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::BYTE_ARRAY;
    pField->m_pData = AddDataEntry( pBytes, ByteCount );

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddComment( const char* pComment, hfield hField )
{
    field* pField = AddField( NULL, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::COMMENT;
    pField->m_pData = NULL;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddV2( const char* pName, vector2& Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::VECTOR2;
    pField->m_pData = new vector2;
    *((vector2*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddV3( const char* pName, vector3& Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::VECTOR3;
    pField->m_pData = new vector3;
    *((vector3*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddR3( const char* pName, radian3& Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::RADIAN3;
    pField->m_pData = new radian3;
    *((radian3*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddM4( const char* pName, matrix4& Val, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::MATRIX4;
    pField->m_pData = new matrix4;
    *((matrix4*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddQuat( const char* pName, quaternion& Val,const char* pComment, hfield hField )                  
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::QUATERNION;
    pField->m_pData = new quaternion;
    *((quaternion*)pField->m_pData) = Val;

    return (hfield)pField;
}

//=============================================================================
hfield igfmgr::AddGroup( const char* pName, const char* pComment, hfield hField )
{
    field* pField = AddField( pName, pComment );
    
    // validate
    if ( !pField )
        return NULL;

    // fill out
    pField->m_Type = igfmgr::GROUP;
    pField->m_pData = NULL;

    return (hfield)pField;
}

//=============================================================================
xbool igfmgr::SetGroup( hfield Group )
{
    field* pField = (field*)Group;

    // setting back to main group?
    if ( pField == m_pRoot )
        pField = NULL;

    // if NULL, set the main group
    if ( !pField )
    {
        m_pCurGroup = (field*)Group;
        m_ppHead = (field**)&m_pRoot;
        m_pCursor = *m_ppHead;

        if ( m_pCursor )
        {
            while ( m_pCursor->m_pNext )
                m_pCursor = m_pCursor->m_pNext;
        }

        return TRUE;
    }

    // assert that this is a group
    ASSERT( pField->m_Type == igfmgr::GROUP );

    // set the new root
    m_ppHead = (field**)&pField->m_pData;
    m_pCurGroup = pField;

    // move the cursor to the end
    m_pCursor = (field*)pField->m_pData;

    if ( m_pCursor )
    {
        while ( m_pCursor->m_pNext )
            m_pCursor = m_pCursor->m_pNext;
    }

    return TRUE;    
}

//=============================================================================
hfield igfmgr::GetCurGroup( void )
{
    return (hfield)(m_pCurGroup);
}

//=============================================================================
xbool igfmgr::EnterGroup( hfield Group )
{
    // bookmark the current location and enter the new group
    group_stack_entry& Item = m_GroupStack.Append();

    Item.m_pGroup = m_pCurGroup;
    Item.m_pItem = m_pCursor;
    Item.m_ppHead = m_ppHead;

    SetGroup( Group );

    return TRUE;
}

//=============================================================================
hfield igfmgr::ExitGroup( )
{
    s32 i;

    i = m_GroupStack.GetCount();

    if ( i )
    {
        m_pCurGroup = m_GroupStack[i-1].m_pGroup;
        m_pCursor =   m_GroupStack[i-1].m_pItem;
        m_ppHead =    m_GroupStack[i-1].m_ppHead;
        m_GroupStack.Delete( i-1 );

        return (hfield)m_pCurGroup;
    }
    
    return (hfield)NULL;
}

//=============================================================================
hfield igfmgr::First( void )
{
    // move cursor to first item
    m_pCursor = *m_ppHead;

    return (hfield)m_pCursor;
}

//=============================================================================
hfield igfmgr::Next( void )
{
    if ( m_pCursor && m_pCursor->m_pNext )
    {
        m_pCursor = m_pCursor->m_pNext;
        return (hfield)m_pCursor;
    }
    else
        return (hfield)0;
}

//=============================================================================
hfield igfmgr::Last( void )
{
    First();

    while ( m_pCursor->m_pNext )
        m_pCursor = m_pCursor->m_pNext;

    return (hfield)m_pCursor;
}

//=============================================================================
hfield igfmgr::Find( const char* pName )
{
    field* pStart = *m_ppHead;

    while (pStart)
    {
        if ( pStart->m_pData )
        {
            if ( x_strcmp( (const char*)((data_entry*)pStart->m_pName)->m_pData, pName ) == 0 )
            {
                m_pCursor = pStart;
                return (hfield)pStart;
            }
        }

        pStart = pStart->m_pNext;
    }

    return NULL;
}
