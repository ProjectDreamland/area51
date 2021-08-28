// AudioPkgDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "AudioPkgDialog.h"
#include "..\EDRscDesc\RscDesc.hpp"
#include "..\WinControls\FileSearch.h"
#include "..\AudioEditor\AudioEditor.hpp"
#include "..\Editor\Project.hpp"
#include "..\..\Support\AudioMgr\AudioMgr.hpp"
#include ".\audiopkgdialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//=========================================================================

CAudioPkgDialog::CAudioPkgDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAudioPkgDialog::IDD, pParent)
{
    m_DescLoaded = FALSE;
    m_PackageLoaded.SetGrowAmount( 10 );
	//{{AFX_DATA_INIT(CAudioPkgDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//=========================================================================

void CAudioPkgDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CAudioPkgDialog)

    DDX_Control(pDX, IDC_TREE_RESOURCE_SELECTOR, m_rscTree);
	//}}AFX_DATA_MAP
}

//=========================================================================

BEGIN_MESSAGE_MAP(CAudioPkgDialog, CDialog)
	//{{AFX_MSG_MAP(CAudioPkgDialog)
	//}}AFX_MSG_MAP
    ON_BN_CLICKED(IDCLEAR, OnBnClickedClear)
END_MESSAGE_MAP()

//=========================================================================

BOOL CAudioPkgDialog::OnInitDialog() 
{
    x_try;

	CDialog::OnInitDialog();
/*        
	for (int i =0; i < g_RescDescMGR.GetRscDescCount(); i++)
    {
        rsc_desc_mgr::node &Node = g_RescDescMGR.GetRscDescIndex(i);
        CString strType(Node.pDesc->GetType());
    
        if (strType.CompareNoCase("audiopkg")==0)
        {
            CString strName(Node.pDesc->GetName());

            m_PackageLoaded.Append( (LPCTSTR)strName );
        }
    }

    m_DescLoaded = TRUE;
*/

	// Create the image list used by the tree control.
	if (!m_imageList.Create (IDB_LAYERLIST_ICONS, 16, 1, RGB(0,255,0)))
		return -1;
	
	// Set the image list for the tree control.
	m_rscTree.SetImageList(&m_imageList, TVSIL_NORMAL);

    //iterate through the resources
	for (int i =0; i < g_RescDescMGR.GetRscDescCount(); i++)
    {
        rsc_desc_mgr::node &Node = g_RescDescMGR.GetRscDescIndex(i);
        CString strType(Node.pDesc->GetType());
        
        if (strType.CompareNoCase("audiopkg")==0)
        {
            CString strName(Node.pDesc->GetName());
            m_PackageLoaded.Append( (LPCTSTR)strName );
        }
    }
	
    //m_btnOk.EnableWindow( FALSE );

    OnLoadPackage();

    x_catch_display;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//=========================================================================

void CAudioPkgDialog::OnLoadPackage ( void )
{
    for( s32 i = 0; i < m_PackageLoaded.GetCount(); i++ )
    {
        xarray<xstring> DescName;

        char FilePath[256];
        sprintf( FilePath, "%s\\PC\\%s", g_Settings.GetReleasePath(), m_PackageLoaded[i] );

//        g_AudioManager.LoadPackageStrings( FilePath, DescName );
        
        X_FILE*                 pPS2File                = NULL;
        char*                   pIdentifierStringTable  = NULL;
        descriptor_identifier*  pIdentifierTable        = NULL;

        x_try;

            pPS2File = x_fopen( FilePath, "rb" );
        
            if( pPS2File == NULL )
                x_throw( xfs("Unable to open file [%s]", FilePath) );

            package_identifier      PackageID;
            package_header          PackageHeader;

            // Read in the package identifier.
            x_fread( &PackageID, sizeof(package_identifier), 1, pPS2File );

            // Correct version?
            if( !x_strncmp( PackageID.VersionID, PC_PACKAGE_VERSION, VERSION_ID_SIZE ) )
            {
                // Correct platform?
                if( !x_strncmp( PackageID.TargetID, PC_TARGET_ID, TARGET_ID_SIZE ) )
                {

                    // Now read in the header.
                    x_fread( &PackageHeader, sizeof(package_header), 1, pPS2File );
                
                    pIdentifierStringTable = new char[ PackageHeader.StringTableFootprint ];

                
                    // Allocate memory for the descriptor identifiers
                    pIdentifierTable = new descriptor_identifier[ PackageHeader.nIdentifiers * sizeof(descriptor_identifier) ];


                    // Read in the string table.
                    x_fread( pIdentifierStringTable, PackageHeader.StringTableFootprint, 1, pPS2File );

                    // Read in the music data.
                    if( PackageHeader.MusicDataFootprint )
                        x_fseek( pPS2File, PackageHeader.MusicDataFootprint, SEEK_CUR );

                    // Read in the lipsync data
                    if( PackageHeader.LipSyncTableFootprint )
                        x_fseek( pPS2File, PackageHeader.LipSyncTableFootprint, SEEK_CUR );

                    // Read in the breakpoint data
                    if( PackageHeader.BreakPointTableFootprint )
                        x_fseek( pPS2File, PackageHeader.BreakPointTableFootprint, SEEK_CUR );

                    // Read in the identifiers.
                    x_fread( pIdentifierTable, sizeof(descriptor_identifier), PackageHeader.nIdentifiers, pPS2File );
                }
                else
                {
                    x_throw( xfs("Incorrect audio package PLATFORM [%s]", FilePath) );
                }
            }
            else
            {
                x_throw( xfs("Incorrect audio package VERSION [%s]", FilePath) );
            }


            x_fclose( pPS2File );
            pPS2File = NULL;

        
            for( s32 j = 0; j < PackageHeader.nIdentifiers; j++ )
            {
                u32   Offset;
                char* pString;

                Offset  = pIdentifierTable[ j ].StringOffset;
                pString = pIdentifierStringTable + Offset;

                CString String = m_PackageLoaded[i];
                //String += '\\';
                //String += DescName[j];
                CString StrName = pString;

                // Add the item to the tree.
                AddPathToTree( String, StrName );
            }

            // Nuke the memory.
            if( pIdentifierStringTable )
                delete [] pIdentifierStringTable;
            if( pIdentifierTable )
                delete [] pIdentifierTable;
        
        x_catch_begin;
            
        x_display_exception_msg( xfs("Could not load audiopkg:\n%s",FilePath) );

            if( pPS2File )
                x_fclose( pPS2File );

            // Nuke the memory.
            if( pIdentifierStringTable )
                delete [] pIdentifierStringTable;
            if( pIdentifierTable )
                delete [] pIdentifierTable;

        x_catch_end;
    }
}

//=========================================================================

void CAudioPkgDialog::OnOK( )
{
    x_try;
/*
    CString String;
    s32 Index = m_DescListBox.GetCurSel();
    if( Index == LB_ERR )
        return;
    m_DescListBox.GetText(Index, String );

    //s32 PkgIndex = String.Find( '\\', 0 );
    //xstring Pkg = String.Left( PkgIndex );
    //String.Delete( 0, PkgIndex+1 );
    m_DescName = String;
*/

	// TODO: Add extra validation here
    HTREEITEM hItem = m_rscTree.GetSelectedItem();
    if (hItem)
    {
        xhandle hHandle = m_rscTree.GetItemData(hItem);
        if (hHandle != HNULL)
        {
            m_DescLoaded = TRUE;
            CString& strData = m_xaPaths(hHandle);
            //m_strPath = strData;
            m_DescName = strData;
            m_DescName += '\\';
            m_DescName += m_rscTree.GetItemText(hItem);
        }
    }
    
/*
    char FilePath[256];
    sprintf( FilePath, "%s\\PC\\%s", g_Settings.GetReleasePath(), Pkg );
    
    for( s32 i = 0; i < PackageLoaded.GetCount(); i++ )
    {
        if( PackageLoaded[i] == Pkg )
        {
            CDialog::OnOK();
            return;
        }
    }

    PackageLoaded.Append( Pkg );
    // Load the package.
    if( !(g_AudioMgr.LoadPackage( FilePath )) )
    {
        x_throw( xfs("Unable to load the audio package[%s], please check your PC release directory", Pkg ) );
    }
*/
    CDialog::OnOK();

    x_catch_display;
}

//=========================================================================

void CAudioPkgDialog::AddPathToTree(CString strPath, CString strName)
{
    xhandle hHandle = HNULL;
    CString& strPathAdded = m_xaPaths.Add(hHandle);
    strPathAdded = strPath;
    CFileSearch::FormatPath(strPath); //strPath now in form "c:\gamedata\a51\temp"
    strPath += "\\"; //ensure a final slash

    char cThemePath[MAX_PATH];
    CString strThemePath;
    for( int j = g_Project.GetFirstResourceDir( cThemePath ); j != -1; j = g_Project.GetNextResourceDir( j, cThemePath ) )
    {
        if (strPath.Find(cThemePath) != -1)
        {
            //found base path
            CString strNewPath(cThemePath);
            if (strNewPath.GetLength() > strThemePath.GetLength())
            {
                //we want the longest match here
                strThemePath = strNewPath;
            }
        }
    }

    if (!strThemePath.IsEmpty())
    {
        int nThemeLen = strThemePath.GetLength();
        CFileSearch::FormatPath(strThemePath);

        CString strThemeName;
        strThemeName = strThemePath.Left(strThemePath.ReverseFind('\\'));
        strThemeName = strThemeName.Right(strThemeName.GetLength() - strThemeName.ReverseFind('\\') - 1);

        strPath = strThemeName + strPath.Right( strPath.GetLength() - nThemeLen);
    }

    ASSERT(hHandle != HNULL);

    int nIndex = strPath.Find('\\');
    HTREEITEM hParent = TVI_ROOT;
    while (nIndex!=-1)
    {
        CString strCurrent = strPath.Left(nIndex);
        strPath = strPath.Right ( strPath.GetLength() - nIndex - 1);

        //does strCurrent exist at this level?
        HTREEITEM hCurrent = DoesChildExist(strCurrent, hParent);
        if ( hCurrent )
        {
            hParent = hCurrent;
        }
        else
        {
            //need to insert this item
            hParent = m_rscTree.InsertItem(strCurrent, 0, 1, hParent);
            m_rscTree.SetItemData(hParent, HNULL);
        }

        nIndex = strPath.Find('\\');
    }

    HTREEITEM hItem = m_rscTree.InsertItem(strName, 2, 3, hParent);
    m_rscTree.SetItemData(hItem, hHandle);
}

//=========================================================================

HTREEITEM CAudioPkgDialog::DoesChildExist(CString strCurrent, HTREEITEM hParent)
{
    HTREEITEM hNextItem;
    HTREEITEM hChildItem;
    
    if (hParent == TVI_ROOT)
    {
        hChildItem = m_rscTree.GetRootItem();
        if (!hChildItem)
        {
            return NULL;
        }
    }
    else
    {
        if (m_rscTree.ItemHasChildren(hParent))
        {   
            hChildItem = m_rscTree.GetChildItem(hParent);
        }
        else
        {
            return NULL;
        }
    }

    while (hChildItem != NULL)
    {
        hNextItem = m_rscTree.GetNextSiblingItem(hChildItem);
        if (strCurrent.CompareNoCase(m_rscTree.GetItemText(hChildItem)) == 0)
        {
            //Found it!
            return hChildItem;
        }
        hChildItem = hNextItem;
    }

    return NULL;
}

//=========================================================================

BOOL CAudioPkgDialog::DestroyWindow() 
{
	m_imageList.DeleteImageList();
	
	return CDialog::DestroyWindow();
}

//=========================================================================
void CAudioPkgDialog::OnBnClickedClear()
{
    m_DescLoaded = TRUE;
    m_DescName = "<null>";

    EndDialog( IDCLEAR );
}
