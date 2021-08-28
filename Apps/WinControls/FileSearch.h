// FileSearch.h: interface for the CFileSearch class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FileSearch_H__5BC7B291_F166_11D4_9393_0050DABB534C__INCLUDED_)
#define AFX_FileSearch_H__5BC7B291_F166_11D4_9393_0050DABB534C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//////////////////////////////////////////////////////////////////////

class CFileSearch  
{
public:
	CFileSearch();
	virtual ~CFileSearch();

	// recurse subdirectories?
	void	Recurse(bool bRecurse)	{ m_bRecurse = bRecurse;}
	// first, get the dirs
	bool	GetDirs(const char * pDirPath);
	// then get the files in those dirs
	bool	GetFiles(const char *pFilemask);
	// remove all entries
	bool	ClearDirs();
	bool	ClearFiles();

    void    AddDirPath(const char * pDirPath);

    CStringList &Files()	{ return m_lstFiles; }
    CStringList &Dirs()	    { return m_lstDirs; }

    int     GetFileCount()  { return m_lstFiles.GetCount(); }

    static BOOL DoesFileExist(const char * pPath);
	static void FormatPath(CString &path);

	//////////////////////////////////////////////////////////////////////
    // searches
    //////////////////////////////////////////////////////////////////////

    void SetFilesIndexToHead() { m_iFileIndex = 0; }
    void SetDirIndexToHead() { m_iDirIndex = 0; }

    CString GetNextDir(BOOL bWithoutPath = FALSE);
    CString GetNextFile(BOOL bWithoutPath = FALSE);

protected:

	bool	GetSubDirs(CStringList &dir_list, const CString &path);
	UINT	FindFiles(const CString & dir, const CString & filter);

	bool	m_bRecurse;
	CString	m_sSourceDir;

	CStringList m_lstDirs;
	CStringList m_lstFiles;

    int m_iDirIndex;
    int m_iFileIndex;
};

#endif // !defined(AFX_FileSearch_H__5BC7B291_F166_11D4_9393_0050DABB534C__INCLUDED_)
