// ManipulatorMgr.cpp : implementation file
//

#include "stdafx.h"
#include "ManipulatorMgr.h"
#include "Manipulator.h"
#include "ManipTranslate.h"
#include "ManipScale.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//============================================================================

CManipulatorMgr g_ManipulatorMgr;

//============================================================================
// CManipulatorMgr

CManipulatorMgr::CManipulatorMgr()
{
    m_pHighlightedManipulator   = NULL;
    m_IsDragging                = FALSE;
}

CManipulatorMgr::~CManipulatorMgr()
{
    // Iterate through manipulators deleting them
    POSITION Pos = m_Manipulators.GetHeadPosition();
    while( Pos != NULL )
    {
        CManipulator* pManipulator = (CManipulator*)m_Manipulators.GetAt( Pos );
        ASSERT( pManipulator );
        POSITION OldPos = Pos;
        m_Manipulators.GetNext( Pos );
        m_Manipulators.RemoveAt( OldPos );
        delete pManipulator;
    }
}

//============================================================================

void CManipulatorMgr::Render( const view& View )
{
    eng_Begin( "Manipulators" );

    // Iterate through manipulators rendering
    POSITION Pos = m_Manipulators.GetHeadPosition();
    while( Pos != NULL )
    {
        CManipulator* pManipulator = (CManipulator*)m_Manipulators.GetAt( Pos );
        ASSERT( pManipulator );

        pManipulator->Render( View );

        m_Manipulators.GetNext( Pos );
    }

    eng_End();
}

//============================================================================

CManipTranslate* CManipulatorMgr::NewManipTranslate( void )
{
    CManipTranslate* pManipulator = new CManipTranslate;
    ASSERT( pManipulator );
    m_Manipulators.AddTail( pManipulator );
    return pManipulator;
}

//============================================================================

CManipScale* CManipulatorMgr::NewManipScale( void )
{
    CManipScale* pManipulator = new CManipScale;
    ASSERT( pManipulator );
    m_Manipulators.AddTail( pManipulator );
    return pManipulator;
}

//============================================================================

void CManipulatorMgr::DeleteManipulator( CManipulator* pManipulator )
{
    if( pManipulator == m_pHighlightedManipulator )
        m_pHighlightedManipulator = NULL;

    POSITION Pos = m_Manipulators.Find( pManipulator );
    ASSERT( Pos );
    m_Manipulators.RemoveAt( Pos );
    delete pManipulator;
}

//============================================================================

xbool CManipulatorMgr::Update( const view& View, f32 ScreenX, f32 ScreenY )
{
    xbool           Dirty           = FALSE;
    f32             ClosestDistance = F32_MAX;
    vector3         Intersection;

    // If dragging then continue to drag the manipulator
    if( m_IsDragging )
    {
        if( m_pHighlightedManipulator )
        {
            m_pHighlightedManipulator->UpdateDrag( View, ScreenX, ScreenY );
        }
        return TRUE;
    }

    // Iterate through manipulators to find closest one that ray hits
    POSITION Pos = m_Manipulators.GetHeadPosition();
    while( Pos != NULL )
    {
        CManipulator* pManipulator = (CManipulator*)m_Manipulators.GetAt( Pos );
        ASSERT( pManipulator );
        
        xbool Hit = pManipulator->HitTest( View, ScreenX, ScreenY, Intersection );
        if( Hit )
        {
            f32 Distance = (Intersection-View.GetPosition()).Length();
            if( Distance < ClosestDistance )
            {
                ClosestDistance             = Distance;
                m_pHighlightedManipulator   = pManipulator;
            }
        }

        m_Manipulators.GetNext( Pos );
    }

    // Clear out highlights on all but the closest
    Pos = m_Manipulators.GetHeadPosition();
    while( Pos != NULL )
    {
        CManipulator* pManipulator = (CManipulator*)m_Manipulators.GetAt( Pos );
        ASSERT( pManipulator );

        if( pManipulator != m_pHighlightedManipulator )
            Dirty |= pManipulator->ClearHighlight();

        m_Manipulators.GetNext( Pos );
    }

    // Highlight the closest
    if( m_pHighlightedManipulator )
    {
        Dirty |= m_pHighlightedManipulator->Highlight( View, ScreenX, ScreenY );
    }

    return Dirty;
}

//============================================================================

xbool CManipulatorMgr::BeginDrag( const view& View, f32 ScreenX, f32 ScreenY )
{
    if( m_pHighlightedManipulator )
    {
        if( m_pHighlightedManipulator->BeginDrag( View, ScreenX, ScreenY ) )
            m_IsDragging = TRUE;
    }
    return m_IsDragging;
}

//============================================================================

xbool CManipulatorMgr::EndDrag( const view& View )
{
    xbool WasDragging = m_IsDragging;

    if( m_pHighlightedManipulator && m_IsDragging )
    {
        m_pHighlightedManipulator->EndDrag( View );
        m_IsDragging = FALSE;
    }

    return WasDragging;
}

//============================================================================

