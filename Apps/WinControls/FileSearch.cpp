// FileSearch.cpp: implementation of the CFileSearch class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FileSearch.h"
#include <io.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFileSearch::CFileSearch()
{
    m_bRecurse = false;
    m_iDirIndex = 0;
    m_iFileIndex = 0;
}

//////////////////////////////////////////////////////////////////////

CFileSearch::~CFileSearch()
{

}

//////////////////////////////////////////////////////////////////////

bool CFileSearch::GetDirs(const char * pDirPath)
{
    m_sSourceDir = pDirPath;

    FormatPath(m_sSourceDir);

    GetSubDirs(m_lstDirs, m_sSourceDir);

    return true;	
}

//////////////////////////////////////////////////////////////////////

bool CFileSearch::GetSubDirs(CStringList &dir_list, const CString &path)
{
    CString newPath;

    CString searchString;
    searchString = path;
    searchString+= "\\*.*";

	try 
	{
        struct _finddata_t  c_file;
        long fhandle;

        if ((fhandle=_findfirst( searchString, &c_file ))!=-1) 
        {
            // we only care about subdirs
            if ((c_file.attrib & _A_SUBDIR)==_A_SUBDIR) 
            {
                // add c_file.name to the string array

                // we'll handle parents on our own
                if ((strcmp(c_file.name, ".")!=0) && (strcmp(c_file.name, "..")!=0)) 
                {
                    newPath = path;
                    newPath+= "\\";
                    newPath+= c_file.name;
                    if (m_bRecurse)
                    {
                        GetSubDirs(dir_list, newPath);
                    }

                    dir_list.AddTail(newPath);
                }
            }

            // find the rest of them	
            while(_findnext( fhandle, &c_file ) == 0 ) 
            {
                if ((c_file.attrib & _A_SUBDIR)==_A_SUBDIR) 
                {
                    // we'll handle parents on our own
                    if ((strcmp(c_file.name, ".")!=0) && (strcmp(c_file.name, "..")!=0)) 
                    {
                        newPath = path;
                        newPath+= "\\";
                        newPath+= c_file.name;
                        if (m_bRecurse)
                        {
                            GetSubDirs(dir_list, newPath);
                        }
                        dir_list.AddTail(newPath);
                    }
                }
            }
            _findclose(fhandle);
        }
	} 
	catch (...) 
	{
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////

bool CFileSearch::ClearFiles()
{
	m_lstFiles.RemoveAll();
    m_iFileIndex = 0;
	return true;
}

//////////////////////////////////////////////////////////////////////

bool CFileSearch::ClearDirs()
{
	m_lstDirs.RemoveAll();
    m_iDirIndex = 0;
	return true;
}

//////////////////////////////////////////////////////////////////////

CString CFileSearch::GetNextDir(BOOL bWithoutPath)
{
    CString strDir;
    if (m_lstDirs.GetCount() > m_iDirIndex)
    {
        strDir = m_lstDirs.GetAt(m_lstDirs.FindIndex(m_iDirIndex));
        m_iDirIndex++; //increment index

        if (bWithoutPath)
        {
            strDir = strDir.Right(strDir.GetLength() - strDir.ReverseFind('\\') - 1);
        }
    }

    return strDir;
}

//////////////////////////////////////////////////////////////////////

CString CFileSearch::GetNextFile(BOOL bWithoutPath)
{
    CString strFile;
    if (m_lstFiles.GetCount() > m_iFileIndex)
    {
        strFile = m_lstFiles.GetAt(m_lstFiles.FindIndex(m_iFileIndex));
        m_iFileIndex++; //increment index

        if (bWithoutPath)
        {
            strFile = strFile.Right(strFile.GetLength() - strFile.ReverseFind('\\') - 1);
        }
    }

    return strFile;
}

//////////////////////////////////////////////////////////////////////

/*static*/
BOOL CFileSearch::DoesFileExist(const char * pPath)
{
    CFileStatus rStatus;
    if (CFile::GetStatus( pPath, rStatus ))
    {
        return TRUE;
    }
    return FALSE;
}

//////////////////////////////////////////////////////////////////////

void CFileSearch::AddDirPath(const char * pDirPath)
{
    m_lstDirs.AddTail(pDirPath);
}

//////////////////////////////////////////////////////////////////////

bool CFileSearch::GetFiles(const char *pFilemask)
{
	// get the files in each of our directories
	for (int i=0; i<m_lstDirs.GetCount(); i++) 
	{
		CString curDir = m_lstDirs.GetAt(m_lstDirs.FindIndex(i));

        // sanity check
		if (curDir.IsEmpty())
		{
			continue;
		}
		
		if (!FindFiles(curDir, pFilemask))
		{
			return false;
		}
	}

	return true;
}	

//////////////////////////////////////////////////////////////////////

// /* static */              chop off trailing "\"
void CFileSearch::FormatPath(CString &path)
{
	CString inPath = path;
	inPath.TrimRight();
	CString tmp;
	
	int iLastSlashPos = inPath.ReverseFind('\\');
	if (iLastSlashPos == -1)
	{
		iLastSlashPos = inPath.ReverseFind('/');
	}
	
	if (iLastSlashPos!=-1) 
	{
		if (iLastSlashPos==inPath.GetLength()-1) 
		{
			path = inPath.Left(iLastSlashPos);

			FormatPath(path); // in case the incoming path is "C:\temp\\\..."
		}
	} 
}

//////////////////////////////////////////////////////////////////////

UINT CFileSearch::FindFiles(const CString & dir, const CString & filter)
{
    // make sure the path ends in a single "\"
    CString baseName = dir;
    FormatPath(baseName);
    baseName+='\\';

    CString fullPath = baseName;
    fullPath += filter;

    CString fileName;

    try 
    {
        // find first file in current directory
        struct _finddata_t  c_file;
        long fhandle;

        if ((fhandle=_findfirst( fullPath, &c_file ))!=-1) 
        {
            // we only care about files, not subdirs
            if ((c_file.attrib & _A_SUBDIR)!=_A_SUBDIR) 
            {
                fileName = baseName;
                fileName += c_file.name;

                m_lstFiles.AddTail(fileName);
            }

            // find the rest of them	
            while(_findnext( fhandle, &c_file ) == 0 ) 
            {
                if ((c_file.attrib & _A_SUBDIR)!=_A_SUBDIR) 
                {
                    fileName=baseName;
                    fileName += c_file.name;

                    m_lstFiles.AddTail(fileName);
                }
            }
            _findclose(fhandle);
        }
    } 
    catch (...) 
    {
        return false;
    }

    return true;
}

