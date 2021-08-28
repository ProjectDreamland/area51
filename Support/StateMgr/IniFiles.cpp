//=========================================================================
//
//  IniFiles.cpp 
//
//=========================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "IniFiles.hpp"

//=========================================================================
//  DEFINES
//========================================================================

#define INI_PATHFILENAME            "C:\\GameData\\A51\\Release\\A51.ini"
#define TAB     9
#define SPACE   32


//=========================================================================
//  FUNCTIONS
//=========================================================================

//=========================================================================

ini_file::ini_file( void )
{
    x_strcpy(m_InIPathFileName, INI_PATHFILENAME);
} 

//=========================================================================

ini_file::~ini_file( void )
{
}

//=========================================================================

xbool ini_file::Load(const char* FileName)
{    
    m_LoadedData.Clear();
    if(!m_LoadedData.LoadFile(FileName))
        return FALSE;
    return TRUE;
}

//=========================================================================

const char * ini_file::GetInIPathFileName( void )
{
    return m_InIPathFileName;
}

//=========================================================================

void ini_file::SetInIPathFileName( const char * PathName )
{
    if(x_strlen(PathName) < X_MAX_PATH)
        x_strcpy( m_InIPathFileName, PathName );
    return;
}

//=========================================================================

xbool ini_file::GetValue( const char * pSection, const char* pKey, xstring &Result)
{
    xstring StrResult;
    StrResult.Clear();
    StrResult = ParseStringData(pSection, pKey);
    if(!StrResult.IsEmpty())
    {
        Result = StrResult;
        return TRUE;
    }
    return FALSE;
}

//=========================================================================

xbool ini_file::GetValue_s32( const char * pSection, const char* pKey, s32 &Result)
{
    xstring StrResult;
    StrResult.Clear();
    StrResult = ParseStringData(pSection, pKey);
    if(!StrResult.IsEmpty())
    {
        Result = x_atoi(StrResult);
        return TRUE;
    }
    return FALSE;
}
//=========================================================================

xbool ini_file::GetValue_f32( const char * pSection, const char* pKey, f32 &Result)
{
    xstring StrResult;
    StrResult.Clear();
    StrResult = ParseStringData(pSection, pKey);
    if(!StrResult.IsEmpty())
    {
       Result = x_atof(StrResult);
       return TRUE;
    }
    return FALSE;
}

//=========================================================================

const char * ini_file::ParseStringData (const char * pSection, const char* pKey)
{
    char StrDelimter;
    xstring StrField;
    StrField.Clear();

    //Check to see if we have any data
    if(!m_LoadedData.IsEmpty())
    {
        //Look for Section
        for(int i = 0, index = 0; i < m_LoadedData.GetLength(); i++)
        {
            StrDelimter = m_LoadedData.GetAt(i);

            if(StrDelimter == '[')
            {
                //Advance one passed starting delimter
                if( i < m_LoadedData.GetLength())
                    i++;
                else
                    return FALSE;
            
                index = SearchForSection(pSection, i);
                if(index != -1)
                {
                    StrField.Clear();
                    StrField = SearchForField(pKey, index);
                    if(!StrField.IsEmpty())
                        return StrField;
                }
            }
        }
    }
    return StrField;
}

//=========================================================================

s32 ini_file::SearchForSection(const char * pSection, s32 StartIndex)
{
    xstring  StrSection;
    s32 SectionCount, ii;

    StrSection.Clear();
    SectionCount = 0;
    ii = StartIndex;

    while(ii <  m_LoadedData.GetLength() && m_LoadedData.GetAt(ii) != ']' && m_LoadedData.GetAt(ii) != '\n')
    {
        StrSection += m_LoadedData.GetAt(ii);
        ii++;
        SectionCount++;
    }
    //Strip Spaces
    StripChars(StrSection, SPACE);
    StripChars(StrSection, TAB);

    if(StrSection == pSection)
    {
        //found Proper section
        //Get passed delimter and end of line and linefeed
        return ii + 3;
    }
    //Not found
    return -1;
}

//=========================================================================

const char* ini_file::SearchForField(const char * pKey,  s32 StartIndex)
{
    xstring  StrField;
    xstring  Value;

    s32 StrFieldCount, ii, ValueCount;

    StrField.Clear();
    Value.Clear();
    StrFieldCount = 0;

    ii = StartIndex;

    while(ii <  m_LoadedData.GetLength() && m_LoadedData.GetAt(ii) != '[')
    {
        StrField.Clear();
        StrFieldCount = 0;

        if(m_LoadedData.GetAt(ii) ==';')
        {
            //get passed comment field
            while(ii <  m_LoadedData.GetLength() && m_LoadedData.GetAt(ii) !='\n' && m_LoadedData.GetAt(ii) != '\r')
                ii++;
            ii+=2;
        }

        while(ii < m_LoadedData.GetLength() && m_LoadedData.GetAt(ii) !='\n' && m_LoadedData.GetAt(ii) !=';'
            &&  m_LoadedData.GetAt(ii) != '=' )
        {
            //Try to get a field
            StrField += m_LoadedData.GetAt(ii);
            ii++;
            StrFieldCount++;

        }

        //strip spaces
        StripChars(StrField, SPACE);
        //Strip tabs
        StripChars(StrField, TAB);

        if(StrField == pKey)
        {
            //Try to get value
            ii++;
            ValueCount = 0;
            Value.Clear();
            while(ii <  m_LoadedData.GetLength() && m_LoadedData.GetAt(ii) !='\n' && m_LoadedData.GetAt(ii) != '\r'
                    && m_LoadedData.GetAt(ii) != ';') 
            {
                Value += m_LoadedData.GetAt(ii);
                ValueCount++;
                ii++;
            }
            StripChars(Value, TAB);  
            StripLeadingTrailingChars( Value, SPACE);
            return Value;
        }
        ii++;
    }
    return Value;

}

//=========================================================================

void ini_file::StripChars(xstring &String, const char RemoveChar)
{
    s32 index;
    for(int ii = 0; ii < String.GetLength(); ii++)
    {
        index = String.Find(RemoveChar);
        if(index !=-1)
        {
            String.Delete(index, 1);
            if(ii > 0)
                --ii;
        }
        else
            break;
    }
}

//=========================================================================

void ini_file::StripLeadingTrailingChars(xstring &String, const char RemoveChar)
{
    s32 ii;

    //Do the left Leading char
    for(ii = 0; ii < String.GetLength(); ii++)
    {
        if(String.GetAt(ii) == RemoveChar)
        {
            String.Delete(ii, 1);
            if(ii > 0)
                ii--;
        }
        else
            break;
    }
    ii = String.Find(RemoveChar, 0);
    if(ii == -1)
        return;  //Done, no more lead or trailing characters

    //Do the right trialing chars
    for(ii = String.GetLength() - 1; ii >=0;  ii--)
    {
        if(String.GetAt(ii) == RemoveChar)
        {
            String.Delete(ii, 1);
            if(ii < String.GetLength() -1)
                ii++;
        }
        else
            break;
    }
}

