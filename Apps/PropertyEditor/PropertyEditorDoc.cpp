// PropertyEditorDoc.cpp : implementation file
//

#include "stdafx.h"
#include "PropertyEditorDoc.h"
#include "PropertyEditorView.h"
#include "PropertyEditorView.h"
#include "Auxiliary\MiscUtils\Guid.hpp"
#include "..\Apps\WorldEditor\WorldEditor.hpp"



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//forward decl
s32 ComparePropEnumsForSort( const void* pA, const void* pB );

/////////////////////////////////////////////////////////////////////////////
// CPropertyEditorDoc

IMPLEMENT_DYNCREATE(CPropertyEditorDoc, CDocument)

CPropertyEditorDoc::CPropertyEditorDoc() : 
m_pProperties(NULL),
m_pCommandHandler(NULL)
{
}

BOOL CPropertyEditorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CPropertyEditorDoc::~CPropertyEditorDoc()
{
}


BEGIN_MESSAGE_MAP(CPropertyEditorDoc, CDocument)
	//{{AFX_MSG_MAP(CPropertyEditorDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertyEditorDoc diagnostics

#ifdef _DEBUG
void CPropertyEditorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPropertyEditorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPropertyEditorDoc serialization

void CPropertyEditorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyEditorDoc commands

s32 ComparePropEnumsForSort( const void* pA, const void* pB )
{
    CString strItem1((char*)pA);
    CString strItem2((char*)pB);

    if( strItem1 < strItem2 )   return( -1 );
    if( strItem1 > strItem2 )   return(  1 );
    else 
    {
        x_throw(xfs("error: duplicate object of name %s", (const char*)strItem1));
        return(  0 );
    }
}

void CPropertyEditorDoc::SaveProperty(CGridTreeItem* lpItem, BOOL bReloadObject)
{
    x_try;

    BOOL bReturn = FALSE;
    CGridItemInfo::CONTROLTYPE ctrlType;
    if (m_pProperties && lpItem->m_lpNodeInfo->GetControlType(0,ctrlType))
    {
        CString strName = lpItem->m_strIdentifier;
        CString strValue = lpItem->m_lpNodeInfo->GetSubItem(0);
        prop_query pq;

        x_try;

        switch( ctrlType )
		{
        case CGridItemInfo::GCT_DIR_BUTTON:
            {
                pq.WQueryFileName( strName, strValue);
                if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
			break;
        case CGridItemInfo::GCT_STRING_EDIT:
            {
                pq.WQueryString( strName, strValue);
                if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
			break;
        case CGridItemInfo::GCT_BOOL:
            {
                if (strValue.CompareNoCase("true")==0)
                    pq.WQueryBool( strName, true );
                else
                    pq.WQueryBool( strName, false );
             
				if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
			break;
		case CGridItemInfo::GCT_FLOAT_EDIT:     
            {
                f32 f = (f32)atof(strValue);
                pq.WQueryFloat( strName, f );
				if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
			break;
		case CGridItemInfo::GCT_NUMERIC_EDIT:       
            {
                s32 i = atoi(strValue);
                pq.WQueryInt( strName, i );
				if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
			break;
        case CGridItemInfo::GCT_2D_COORDINATE:
            {
                CPropertyTree2DPos* pSpItem = (CPropertyTree2DPos*)lpItem;
                vector2 v2(pSpItem->m_x, pSpItem->m_y);
                pq.WQueryVector2( strName, v2 );
                if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
            break;
		case CGridItemInfo::GCT_3D_COORDINATE:
            {
                CPropertyTree3DPos* pSpItem = (CPropertyTree3DPos*)lpItem;
                vector3 v3(pSpItem->m_x,pSpItem->m_y,pSpItem->m_z);
                pq.WQueryVector3( strName, v3 );
				if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
			break;
		case CGridItemInfo::GCT_DEGREE_EDIT:     
            {
                radian r = DEG_TO_RAD(atof(strValue));
                pq.WQueryAngle( strName, r );
				if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
			break;
		case CGridItemInfo::GCT_ROTATION_EDIT:  			
            {
                CPropertyTreeRotation* pSpItem = (CPropertyTreeRotation*)lpItem;
                radian3 r3(DEG_TO_RAD(pSpItem->m_Pitch),DEG_TO_RAD(pSpItem->m_Yaw),DEG_TO_RAD(pSpItem->m_Roll));
                pq.WQueryRotation( strName, r3 );
				if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
			break;
		case CGridItemInfo::GCT_BOUNDING_BOX:      
            {
                CPropertyTreeBoundingBox* pSpItem = (CPropertyTreeBoundingBox*)lpItem;
                vector3 vMin(pSpItem->m_pVectorMin->m_x,pSpItem->m_pVectorMin->m_y,pSpItem->m_pVectorMin->m_z);
                vector3 vMax(pSpItem->m_pVectorMax->m_x,pSpItem->m_pVectorMax->m_y,pSpItem->m_pVectorMax->m_z);
                bbox box(vMin,vMax);
                pq.WQueryBBox( strName, box );
				if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
			break;
		case CGridItemInfo::GCT_COLOR_BUTTON:      
            {
                CPropertyTreeColor* pSpItem = (CPropertyTreeColor*)lpItem;
                xcolor xc(GetRValue(pSpItem->m_cr),GetGValue(pSpItem->m_cr),GetBValue(pSpItem->m_cr),pSpItem->m_alpha);
                pq.WQueryColor( strName, xc );
				if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
			break;
		case CGridItemInfo::GCT_COMBOBOX:  
            {
                pq.WQueryEnum( strName, strValue);
                if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
			break;
        case CGridItemInfo::GCT_GUID_EDIT:
            {
                pq.WQueryGUID( strName, guid_FromString(strValue));
                if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
            break;
		case CGridItemInfo::GCT_EXTERNAL: 
            {
                pq.WQueryExternal( strName, strValue );
				if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
            break;
		case CGridItemInfo::GCT_BUTTON: 
            {
                pq.WQueryButton( strName, strValue );
				if (m_pProperties->OnProperty(pq)) bReturn = TRUE;
            }
            break;
        case CGridItemInfo::GCT_NULL_ENTRY: 
        default:
			x_throw(xfs("error: could not save property %s", (const char*)strName));
			break;
        }

        x_catch_begin;
            Refresh();
        x_catch_end_ret;
    }

    if (bReturn)
    {
        //set an update message to the parent
        if (m_pCommandHandler)
            m_pCommandHandler->SendMessage(WM_USER_MSG_UPDATE_REQUIRED);

        if (bReloadObject) 
        {
            Refresh();
        }
        else
        {
            UpdateIndividualItem(lpItem);
        }
    }

    x_catch_display;
}

void CPropertyEditorDoc::GuidSelect( CGridTreeItem* lpItem )
{
    if (m_pCommandHandler && lpItem) 
    {
        char* pszIdentifier = lpItem->m_strIdentifier.GetBuffer(lpItem->m_strIdentifier.GetLength());
        m_pCommandHandler->SendMessage(WM_USER_MSG_GUID_SELECT_FOR_PROPERTY,(WPARAM)pszIdentifier);
    }
}

void CPropertyEditorDoc::UpdateIndividualItem(CGridTreeItem* lpItem)
{
    CPropertyEditorView* pView = GetView();
    if( pView->IsValidItem( lpItem ) )
    {
        CGridTreeItem* pParentTreeItem = NULL ;

        if (lpItem && lpItem->m_lpNodeInfo)
        {
            int iXaIndex = lpItem->m_lpNodeInfo->GetDataIndex();
            GetDataFromXarray(iXaIndex, lpItem, FALSE, pParentTreeItem);
        }
    }
    else
    {
        LOG_ERROR( "CPropertyEditorDoc::UpdateIndividualItem", "Bad Item Pointer" );
    }
}

void CPropertyEditorDoc::GetDataFromXarray(int iXaIndex, 
                                           CGridTreeItem* lpItem, 
                                           BOOL bCreateObject,
                                           CGridTreeItem* &pParentTreeItem )
{
    x_try;

    if (m_xaList.GetCount()>iXaIndex)
    {
        prop_enum::node enData = m_xaList[iXaIndex];
	    prop_type type = (prop_type)enData.GetType();
	    CString strName = enData.GetName();

        BOOL bDontShow = type & PROP_TYPE_DONT_SHOW;
        if (!bDontShow)
        {
            BOOL bHeader = type & PROP_TYPE_HEADER;
	        BOOL bReadOnly = type & PROP_TYPE_READ_ONLY;
            BOOL bMustEnum = type & PROP_TYPE_MUST_ENUM;
	        CGridItemInfo::CONTROLTYPE typeGridItem;
	        CString strValue = "";
            prop_query pq;
	        CStringList strlstData;

	        switch( type & PROP_TYPE_BASIC_MASK )
	        {
	        case PROP_TYPE_NULL:       
                {
			        typeGridItem = CGridItemInfo::GCT_NULL_ENTRY;
                }
		        break;
	        case PROP_TYPE_FILENAME:     
                {
			        typeGridItem = CGridItemInfo::GCT_DIR_BUTTON;
                    char cString[MAX_PATH];
                    pq.RQueryFileName( strName, &cString[0]);
                    if (m_pProperties->OnProperty(pq))
                    {
                        strValue = CString(cString);
                    }

                    const char* pString = enData.GetEnumType( 0 );
                    if( x_strlen( pString ) > 0 )
                    {
        		        strlstData.AddTail( pString );
                    }
                }
		        break;
	        case PROP_TYPE_STRING:     
                {
			        typeGridItem = CGridItemInfo::GCT_STRING_EDIT;
                    char cString[MAX_PATH];
                    pq.RQueryString( strName, &cString[0]);
                    if (m_pProperties->OnProperty(pq))
                    {
                        strValue = CString(cString);
                    }
                }
		        break;
	        case PROP_TYPE_FLOAT:     
                {
			        typeGridItem = CGridItemInfo::GCT_FLOAT_EDIT;
                    f32 f;
                    pq.RQueryFloat( strName, f );
			        if (m_pProperties->OnProperty(pq))
                    {
                        strValue.Format("%g",f);
                    }
                }
		        break;
	        case PROP_TYPE_INT:       
                {
			        typeGridItem = CGridItemInfo::GCT_NUMERIC_EDIT;
                    s32 i;
                    pq.RQueryInt( strName, i );
			        if (m_pProperties->OnProperty(pq))
                    {
                        strValue.Format("%d",i);
                    }
                }				
		        break;
            case PROP_TYPE_BOOL:
                {
			        typeGridItem = CGridItemInfo::GCT_BOOL;
                    xbool b;
                    pq.RQueryBool( strName, b );
			        if (m_pProperties->OnProperty(pq)) 
                    {
                        if (b)
                            strValue = "true";
                        else
                            strValue = "false";
                    }
                }
		        break;
            case PROP_TYPE_VECTOR2:
                {
                    typeGridItem = CGridItemInfo::GCT_2D_COORDINATE;
                    vector2 v2;
                    pq.RQueryVector2( strName, v2 );
                    if (m_pProperties->OnProperty(pq))
                    {
                        strValue.Format("%g, %g, %g", v2[0], v2[1]);
                    }
                }
                break;
	        case PROP_TYPE_VECTOR3:
                {
			        typeGridItem = CGridItemInfo::GCT_3D_COORDINATE;
                    vector3 v3;
                    pq.RQueryVector3( strName, v3 );
			        if (m_pProperties->OnProperty(pq))
                    {
                        strValue.Format("%g, %g, %g",v3[0],v3[1],v3[2]);
                    }
                }
		        break;
	        case PROP_TYPE_ANGLE:     
                {
			        typeGridItem = CGridItemInfo::GCT_DEGREE_EDIT;
                    radian r;
                    pq.RQueryAngle( strName, r );
			        if (m_pProperties->OnProperty(pq))
                    {
                        strValue.Format("%g",RAD_TO_DEG(r));
                    }
                }
		        break;
	        case PROP_TYPE_GUID:     
                {
			        typeGridItem = CGridItemInfo::GCT_GUID_EDIT;
                    guid g;
                    pq.RQueryGUID( strName, g );
			        if (m_pProperties->OnProperty(pq))
                    {
                        strValue = guid_ToString(g);
                    }
                }
		        break;
	        case PROP_TYPE_ROTATION:  			
                {
			        typeGridItem = CGridItemInfo::GCT_ROTATION_EDIT;
                    radian3 r3;
                    pq.RQueryRotation( strName, r3 );
			        if (m_pProperties->OnProperty(pq))
                    {
                        strValue.Format("%g, %g, %g",RAD_TO_DEG(r3.Roll),RAD_TO_DEG(r3.Pitch),RAD_TO_DEG(r3.Yaw));
                    }
                }
		        break;
	        case PROP_TYPE_BBOX:      
                {
			        typeGridItem = CGridItemInfo::GCT_BOUNDING_BOX;
                    bbox box;
                    pq.RQueryBBox( strName, box );
			        if (m_pProperties->OnProperty(pq))
                    {
                        vector3 vExtent = box.GetSize();
                        strValue.Format("Extent {%g, %g, %g}",vExtent[0], vExtent[1], vExtent[2]);
		                strlstData.AddTail(xfs("%g,%g,%g",box.Min.GetX(),box.Min.GetY(),box.Min.GetZ()));
		                strlstData.AddTail(xfs("%g,%g,%g",box.Max.GetX(),box.Max.GetY(),box.Max.GetZ()));
                    }
                }
		        break;
	        case PROP_TYPE_COLOR:      
                {
			        typeGridItem = CGridItemInfo::GCT_COLOR_BUTTON;
                    xcolor xc;
                    pq.RQueryColor( strName, xc );
			        if (m_pProperties->OnProperty(pq))
                    {
                        strValue.Format("%d, %d, %d, %d",xc.R,xc.G,xc.B,xc.A);
                    }
                }
		        break;
	        case PROP_TYPE_ENUM:  
                {
			        typeGridItem = CGridItemInfo::GCT_COMBOBOX;
                    for (int j=0; j < enData.GetEnumCount(); j++)
                    {
        		        strlstData.AddTail(enData.GetEnumType(j));
                    }
                    char cString[MAX_PATH];
                    pq.RQueryEnum( strName, &cString[0]);
                    if (m_pProperties->OnProperty(pq))
                    {
                        strValue = CString(cString);
                    }
                }
		        break;
            case PROP_TYPE_EXTERNAL:
                {
			        typeGridItem = CGridItemInfo::GCT_EXTERNAL;

                    char cString[MAX_PATH];
                    pq.RQueryExternal( strName, &cString[0]);
                    if (m_pProperties->OnProperty(pq))
                    {
                        strValue = CString(cString);
                    }
                }
		        break;
	        case PROP_TYPE_BUTTON:     
                {
			        typeGridItem = CGridItemInfo::GCT_BUTTON;

                    char cString[MAX_PATH];
                    pq.RQueryButton( strName, &cString[0]);
                    if (m_pProperties->OnProperty(pq))
                    {
                        strValue = CString(cString);
                    }
                }
		        break;
	        }

	        CString strComment = enData.GetComment();

            if (bCreateObject)
            {
                //add the new object
	            if (!GetView()->AddGridDataElement(strName, strValue, strComment, 
                                               typeGridItem, strlstData, iXaIndex,
                                               bReadOnly, bMustEnum, bHeader, pParentTreeItem))
                {
                    //invalid format for an item
                    x_throw(xfs("error: failure adding property %s to grid", (const char*)strName));
                }
            }
            else
            {
                //just edit an existing object
                if (lpItem && lpItem->m_lpNodeInfo)
                {
                    CString strShortName = strName;
                    int iIndex = strName.ReverseFind('\\');
                    if (iIndex != -1) 
                    {
                        strShortName = strName.Right(strName.GetLength()-(iIndex+1));
                    }

                    lpItem->m_strComment = strComment;
                    lpItem->m_lpNodeInfo->SetItemText(strShortName);
	                lpItem->m_lpNodeInfo->SetSubItemText(0,strValue);
            	    lpItem->m_lpNodeInfo->SetListData(0, &strlstData);
                    lpItem->m_lpNodeInfo->SetIsHeader(bHeader);

                    GetView()->UpdateItem(lpItem);
                }
                else
                {
                    x_throw(xfs("error: failure editing property %s", (const char*)strName));
                }
            }
        }
    }

    x_catch_display;
}

void CPropertyEditorDoc::SetInterface( prop_interface &pi, xbool bReadOnly )
{
    m_bReadOnly   = bReadOnly;
    m_pProperties = &pi;
    Refresh();
}

prop_interface* CPropertyEditorDoc::GetInterface( void )
{
    return m_pProperties;
}

void CPropertyEditorDoc::ClearInterface( void )
{
    CONTEXT( "CPropertyEditorDoc::ClearInterface" );

    m_pProperties = NULL;
    Refresh();
}

void CPropertyEditorDoc::ClearGrid()
{
    GetView()->ClearGrid();
}

void CPropertyEditorDoc::Refresh( void )
{
    x_try;

    GetView()->DisableRedraw();

    GetView()->SaveStructure();
    GetView()->ClearGrid();

    if( m_pProperties == NULL )
    {
        GetView()->EnableRedraw();
        return;
    }


    CGridTreeItem* pParentTreeItem = NULL ;

	CPropertyEditorView *pView = GetView();
	if (pView && m_pProperties)
	{
        m_xaList.Clear();
        m_pProperties->OnEnumProp(m_xaList);

        if (m_xaList.GetCount()>0)
        {
            int iXaIndex;

            /*
            // DBS 5/17/04: Commenting out this chunk of code that seems to be broken.
            // Basically properties are grayed out unless you have an object selected
            // and the selected object is in a writable layer, so things like grid
            // snap and resource descriptions are marked as read-only by accident.
            // (The breakage occurred in changelist 55090) We need a proper fix, but
            // several people are blocked so I'm checking this in.

            BOOL IsLayerWriteAble = FALSE;  
            //find out if object a layer that is editable
            if(!g_WorldEditor.OverRideReadOnly())
            {
                if(g_WorldEditor.GetSelectedCount())
                {
                    guid ObjGuid = g_WorldEditor.GetSelectedObjectsByIndex(0);
                    if(ObjGuid)
                    {
                        editor_layer& Layer = g_WorldEditor.FindObjectsLayer(ObjGuid,TRUE);
                        if (!Layer.IsNull && !Layer.Name.IsEmpty())
                        {
                            IsLayerWriteAble = Layer.IsEditable;
                        }
                    }
                }
            }
            else
                //Override Locked Layer
                IsLayerWriteAble = TRUE;
                */

            if( m_bReadOnly /*|| !IsLayerWriteAble*/)
            {
                for( iXaIndex = 0; iXaIndex < m_xaList.GetCount(); iXaIndex++ )
                {
                    m_xaList[iXaIndex].SetFlags( PROP_TYPE_READ_ONLY );
                }
            }

            //now we must sort the list for fast view placement!!!!!
            //sorting removed, now requires object to be given in valid order
            //x_qsort( &list[0], list.GetCount(), sizeof(prop_enum), ComparePropEnumsForSort );

            for( iXaIndex = 0; iXaIndex < m_xaList.GetCount(); iXaIndex++)
		    {
                GetDataFromXarray(iXaIndex, NULL, TRUE, pParentTreeItem);
            }
		}
        else
        {
            TRACE("CPropertyEditorDoc::Refresh() warning: empty enum list for object\n");
        }
	}
    else
    {
        x_throw("error: invalid prop_interface");
    }

    GetView()->ExpandRoots();

    x_catch_display;

    GetView()->EnableRedraw();

    GetView()->RestoreSavedSelection();

    GetView()->Invalidate( FALSE );
}

void CPropertyEditorDoc::SetGridBackgroundColor(COLORREF cr)
{
    CPropertyEditorView* pView = GetView();
    ASSERT(pView);
    if (pView)
    {
        pView->SetGridBackgroundColor(cr);
    }
}
