//=========================================================================
// SOUNDDOC.CPP
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "stdafx.h"
#include "Resource.h"
#include "..\Editor\Resource.h"
#include "SoundDoc.h"
#include "SoundView.h"
#include "AudioEditorFrame.h"
#include "..\Editor\Project.hpp"
#include "FaderDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define READ_ONLY   0x01
//=========================================================================
// REGESTER AUDIO EDITOR
//=========================================================================
REG_EDITOR( s_RegAudioEdit, "Audio Editor", "audiopkg", IDS_RSC_AUDIOEDITOR, CSoundDoc, CAudioEditorFrame, CSoundView );
void LinkAudioEditor(void){}



IMPLEMENT_DYNCREATE(CSoundDoc, CBaseDocument)


//=========================================================================
// MESSAGE PUMP
//=========================================================================

BEGIN_MESSAGE_MAP(CSoundDoc, CBaseDocument)
	//{{AFX_MSG_MAP(CSoundDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================

CSoundDoc::CSoundDoc()
{
    m_SelectionStart    = 0;
    m_SelectionEnd      = 0;
    m_SoundFileCount    = 0;
    m_MaxTime           = 0.0f;
    m_MaxTimeIndex      = 0;
    m_ElementPathName   = "";
    m_SelectedItemData  = 0;
    m_pCommandHandler   = NULL;
    m_SampleSelected    = -1;
    m_SoundUnits        = SAMPLES;
    m_RecentAudioSampleName = "";
}

//=========================================================================

CSoundDoc::~CSoundDoc()
{
    for( s32 i = (m_SoundFileCount-1); i >= 0; i-- )
        UnLoadSample( i );

    m_SelectionEnd      = 0;
    m_SoundFileCount    = 0;
}

//=========================================================================

#ifdef _DEBUG
void CSoundDoc::AssertValid() const
{
	CBaseDocument::AssertValid();
}

void CSoundDoc::Dump(CDumpContext& dc) const
{
	CBaseDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CSoundDoc commands

//=========================================================================

s32 CSoundDoc::GetSampleRate( s32 Index )
{
    if( Index >= m_SoundFileCount || Index < 0)
        return 0;

    return m_pSoundFile[Index].GetSampleRate();
}

//=========================================================================

s32 CSoundDoc::GetNumChannels( s32 Index )
{
    if( Index >= m_SoundFileCount || Index < 0)
        return 0;

    return m_pSoundFile[Index].GetNumChannels();
}

//=========================================================================

s32 CSoundDoc::GetNumSamples( s32 Index )
{
    if( Index >= m_SoundFileCount || Index < 0)
        return 0;

    return m_pSoundFile[Index].GetNumSamples();
}

//=========================================================================

xbool CSoundDoc::GetIsLoaded( s32 Index )
{
    if( Index >= m_SoundFileCount || Index < 0)
        return 0;

    return m_pSoundFile[Index].IsLoaded();
}

//=========================================================================

s32  CSoundDoc::GetBreakPoints( xarray<aiff_file::breakpoint>& BreakPoints, s32 Index )
{
    if( Index >= m_SoundFileCount || Index < 0)
        return 0;

    return m_pSoundFile[Index].GetBreakPoints( BreakPoints );
}

//=========================================================================

s16* CSoundDoc::GetChannelData( s32 iChannel, s32 Index )
{
    if( Index >= m_SoundFileCount || Index < 0)
        return 0;

    return m_pSoundFile[Index].GetChannelData( iChannel );
}

//=========================================================================

s32 CSoundDoc::GetSelectionStart( void )
{
    return m_SelectionStart;
}

//=========================================================================

s32 CSoundDoc::GetSelectionEnd( void )
{
    return m_SelectionEnd;
}

//=========================================================================

BOOL CSoundDoc::OnNewDocument()
{
	if (!CBaseDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

//=========================================================================

BOOL CSoundDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
    reg_editor::on_open& Open = *((reg_editor::on_open*)lpszPathName);

    editor_audio_package& Audio = editor_audio_package::GetSafeType( *((rsc_desc*)Open.pData) );
    

    m_AudioEditor.BeginEdit( Audio );


    return TRUE;
}

//=========================================================================

BOOL CSoundDoc::OnSaveDocument(LPCTSTR lpszPathName) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CBaseDocument::OnSaveDocument(lpszPathName);
}

//=========================================================================

void CSoundDoc::OnCloseDocument() 
{
	// TODO: Add your specialized code here and/or call the base class
	
	CBaseDocument::OnCloseDocument();
}

//=========================================================================

void CSoundDoc::OnProjectOpen ( void )
{    
    CONTEXT( "CSoundDoc::OnProjectOpen" );

    CBaseDocument::OnProjectOpen();
    UpdateAllViews( (CView*)0,OPEN_AUDIO_PROJECT );
}

//=========================================================================

void CSoundDoc::OnProjectClose ( void )
{
    CBaseDocument::OnProjectClose();
    UpdateAllViews( (CView*)0,CLOSE_AUDIO_PROJECT );
}

//=========================================================================

void CSoundDoc::OnProjectSave ( void )
{
    x_try;

    m_AudioEditor.Save();
    
    // Get the first resouce dir and go up a level to save the gobal fader list.
    char cFirstThemePath[MAX_PATH];
    g_Project.GetFirstResourceDir( cFirstThemePath );
    CString FilePath = cFirstThemePath;

    s32 ResourceIndex = FilePath.Find( "Resource" );
    FilePath.Delete( ResourceIndex, FilePath.GetLength() - ResourceIndex );
    FilePath += "FaderList.txt";

    CFileStatus FileStatus;
    CFile::GetStatus( FilePath, FileStatus );

    if( FileStatus.m_attribute & READ_ONLY )
        x_throw( xfs( "Unable to save file [%s]", (const char*)FilePath) );
	
    CFile File( (LPCTSTR)FilePath, CFile::modeCreate | CFile::modeWrite );

	File.Write( g_String.GetBuffer( g_String.GetLength() ), g_String.GetLength() );
	File.Close();
    
    x_catch_display;
}

//=========================================================================

void CSoundDoc::OnProjectNew ( void )
{
    CBaseDocument::OnProjectNew();
    UpdateAllViews( (CView*)0,NEW_AUDIO_PROJECT );
}

//=========================================================================

xbool CSoundDoc::LoadSample( LPCTSTR lpszPathName, f32 Offset  )
{
    CString SampleName( lpszPathName );
    if( SampleName.Find( ".aif", 0 ) == -1 )
        return FALSE;

    // Load sound file
    sound_file SoundFile;
    BOOL Success = SoundFile.Load( lpszPathName );
    m_pSoundFile.Append( SoundFile );
    m_pOffset.Append( Offset );

    if( m_MaxTime < SoundFile.GetTime() )
    {
        m_MaxTime = SoundFile.GetTime();
        m_MaxTimeIndex = m_pOffset.GetCount()-1;
    }

    m_SoundFileCount++;
    // Return TRUE for success, FALSE for failure

    UpdateAllViews( (CView*)0,REFRESH_SAMPLE_VIEW );

	return Success;
}

//=========================================================================

void CSoundDoc::UnLoadSample( s32 Index )
{
    // Delete the Sample.
    m_pSoundFile[Index].UnLoad();
    m_pSoundFile.Delete( Index );
    m_pOffset.Delete( Index );
    
    m_SoundFileCount--;

    // Recheck the maximum sample lenght.
    m_MaxTime = 0;
    for( s32 i = 0; i < m_SoundFileCount; i++ )
    {
        if( m_MaxTime < m_pSoundFile[i].GetTime() )
        {
            m_MaxTime = m_pSoundFile[i].GetTime();
            m_MaxTimeIndex = i;
        }
    }
}

//=========================================================================

void CSoundDoc::OnLoadFaderList ( void )
{
    // Get the first resouce dir and go up a level to load the gobal fader list.
    char cFirstThemePath[MAX_PATH];
    g_Project.GetFirstResourceDir( cFirstThemePath );
    CString FilePath = cFirstThemePath;

    s32 ResourceIndex = FilePath.Find( "Resource" );
    FilePath.Delete( ResourceIndex, FilePath.GetLength() - ResourceIndex );
    FilePath += "FaderList.txt";

	CFile File( (LPCTSTR)FilePath, CFile::modeRead );
    char tempFaderList[512];
	s32 FileLen = (s32)File.GetLength();
    File.Read( tempFaderList, FileLen );
    
    // This will force the string to know how many character there are since each fader name is followed by a 0.
    g_String.Empty();
    while( FileLen )
    {
        FileLen--;
        g_String.Insert( 0, tempFaderList[FileLen] );
    }
    g_pFaderList = g_String.GetBuffer( g_String.GetLength() );
    File.Close();
}

//=========================================================================

void CSoundDoc::ClearAllSamples ( void )
{
    for( s32 i = m_pSoundFile.GetCount() - 1; i >= 0 ; i-- )
        UnLoadSample( 0 );
    
    m_pSoundFile.Clear();

    m_SelectionStart = 0;
    m_SelectionEnd = 0;
    m_SoundFileCount = 0;
    m_MaxTime = 0.0f;

    UpdateAllViews( (CView*)0,REFRESH_SAMPLE_VIEW );
}

//=========================================================================

void CSoundDoc::OnNewAudioPackage ( void )
{
    UpdateAllViews( (CView*)0,NEW_AUDIO_PACKAGE );
}

//=========================================================================

void CSoundDoc::OnNewAudioDescriptor ( void )
{
    UpdateAllViews( (CView*)0,NEW_AUDIO_DESCRITPOR );
}

//=========================================================================

void CSoundDoc::OnNewAudioElement ( CString PathName )
{
    m_ElementPathName = PathName;
    UpdateAllViews( (CView*)0,NEW_AUDIO_ELEMENT );
}

//=========================================================================

void CSoundDoc::SetRecentAudioSampleName( CString SampleName )
{
    m_RecentAudioSampleName = SampleName;
}

//=========================================================================

CString CSoundDoc::GetRecentAudioSampleName( void )
{
    return m_RecentAudioSampleName;
}

//=========================================================================

void CSoundDoc::OnAddAudioElement ( void )
{
    UpdateAllViews( (CView*)0,ADD_AUDIO_ELEMENT );
}

//=========================================================================
    
void CSoundDoc::RefrenceDescriptor ( void )
{
    UpdateAllViews( (CView*)0,REF_AUDIO_DESCRITPOR );
}

//=========================================================================
    
DWORD CSoundDoc::GetSelectedItemData ( void )
{
    return m_SelectedItemData;
}

//=========================================================================

CString CSoundDoc::GetAudioSourcePath ( void )
{
    return m_AudioSourcePath;
}

//=========================================================================

void CSoundDoc::SetAudioSourcePath ( CString& rPath )
{
    x_try;

    if( rPath.GetLength() <= 0 )
        x_throw( "Please enter an appropriate path" );

    m_AudioSourcePath = rPath;
    UpdateAllViews( (CView*)0,SET_AUDIO_SOURCE_PATH );

    x_catch_display;
}

//=========================================================================

void CSoundDoc::SampleToDescriptor ( void )
{
    UpdateAllViews( (CView*)0,SAMPLE_TO_DESCRIPTOR );
}

//=========================================================================

void CSoundDoc::CreateDescFromSample ( CString PathName )
{
    x_try;

    if( PathName.GetLength() <= 0 )
        x_throw( "Please enter an appropriate path" );

    m_ElementPathName = PathName;
    UpdateAllViews( (CView*)0,CREATE_DESC_FROM_SAMPLE );

    x_catch_display;
}

//=========================================================================

void CSoundDoc::ElementSelectionChange  ( s32 SampleSelected )
{
    // Make sure that we don't go over bounds.
    m_SampleSelected = SampleSelected;
    if( m_SampleSelected == m_SoundFileCount )
        m_SampleSelected = m_SoundFileCount-1;

    UpdateAllViews( (CView*)0,ELEMENT_SEL_CHANGE );
    UpdateAllViews( (CView*)0,REFRESH_SAMPLE_VIEW );
}

//=========================================================================

void CSoundDoc::RefreshSampleView ( void )
{
    UpdateAllViews( (CView*)0,REFRESH_SAMPLE_VIEW );
}

//=========================================================================

void CSoundDoc::RefreshIntensityView ( void )
{
    UpdateAllViews( (CView*)0, REFRESH_INTENSITY );
}

//=========================================================================

void CSoundDoc::OnSortDescending( void )
{
    UpdateAllViews( (CView*)0, SORT_DESCENDING );
}

//=========================================================================

void CSoundDoc::OnCopy( void )
{
    UpdateAllViews( (CView*)0, AUDIO_EDITOR_COPY );
}

//=========================================================================

void CSoundDoc::OnPaste( void )
{
    UpdateAllViews( (CView*)0, AUDIO_EDITOR_PASTE );
}

//=========================================================================
