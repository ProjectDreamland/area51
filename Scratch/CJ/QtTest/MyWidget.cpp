#include "qsplitter.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qwidget.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qrect.h"
#include "qwidgetlist.h"
#include "qobjectlist.h"

#define _WIN32_WINNT 0x0501
#include <qt_windows.h>

#include "MyWidget.h"

QImage* pIconDockLeft[2]       = { NULL, NULL };
QImage* pIconDockRight[2]      = { NULL, NULL };
QImage* pIconDockTop[2]        = { NULL, NULL };
QImage* pIconDockBottom[2]     = { NULL, NULL };
QImage* pIconDockTab[2]        = { NULL, NULL };
QImage* pIconDockBackground[2] = { NULL, NULL };

static DockTargetWidget*    g_pDockTargetWidgets[16] = {0};
static DockHintWidget*      g_pDragWidget = NULL;

DockTargetWidget::DockTargetWidget( dock_type Type, QWidget* pParent, QWidget* pTargetWidget, float HintSize, QImage* pImage[2], int x, int y ) :
    QWidget( pParent, 0, Qt::WStyle_Customize|Qt::WStyle_NoBorder ),
    m_Type( Type ),
    m_pTargetWidget( pTargetWidget ),
    m_HintSize( HintSize )
{
    ASSERT( pImage );
    m_pImage[0] = pImage[0];
    m_pImage[1] = pImage[1];
    QPixmap Pixmap;
    Pixmap.convertFromImage( *m_pImage[0] );
    QBitmap bm;
    bm = m_pImage[0]->createAlphaMask();
    Pixmap.setMask( bm );
    setBackgroundPixmap( Pixmap );
    setFixedSize( Pixmap.size() );
    setMask( *Pixmap.mask() );
    move( x, y );
    show();
}

DockTargetWidget::~DockTargetWidget()
{
}

void DockTargetWidget::setHighlighted( bool State )
{
    QPixmap Pixmap;

    if( State )
    {
        Pixmap.convertFromImage( *m_pImage[1] );
        setBackgroundPixmap( Pixmap );
    }
    else
    {
        Pixmap.convertFromImage( *m_pImage[0] );
        setBackgroundPixmap( Pixmap );
    }
}

void DockTargetWidget::setTargetWidget( QWidget* pTargetWidget )
{
    m_pTargetWidget = pTargetWidget;
}

DockTargetWidget::dock_type DockTargetWidget::type( void )
{
    return m_Type;
}

QWidget* DockTargetWidget::targetWidget( void )
{
    return m_pTargetWidget;
}

float DockTargetWidget::hintSize( void )
{
    return m_HintSize;
}

bool DockTargetWidget::hitTest( const QPoint& Point )
{
    ASSERT( m_pImage );
    bool Hit = false;

    if( (Point.x() >= 0) && (Point.x() < m_pImage[0]->width ()) &&
        (Point.y() >= 0) && (Point.y() < m_pImage[0]->height()) )
    {
        Hit = qAlpha( m_pImage[0]->pixel( Point.x(), Point.y() ) ) > 0;
    }

    return Hit;
}

extern QMainWindow* g_pMainWidget;

DockHintWidget::DockHintWidget( int OffsetX, int OffsetY, int SourceW, int SourceH ) :
    QWidget( 0, 0, Qt::WStyle_Tool|Qt::WStyle_Customize|Qt::WStyle_NoBorder|Qt::WNoAutoErase|Qt::WStyle_StaysOnTop ),
    m_ShownFloating( true ),
    m_Color( QColor(128,128,128) ),
    m_OffsetX( OffsetX ),
    m_OffsetY( OffsetY ),
    m_SourceW( SourceW ),
    m_SourceH( SourceH )
{
    setCaption( "" );
    setBackgroundMode( Qt::NoBackground );

/*
    QPixmap Pixmap;
    Pixmap.convertFromImage( *pIconDockLeft[0] );
    QBitmap bm;
    bm = pIconDockLeft[0]->createAlphaMask();
    Pixmap.setMask( bm );
    setBackgroundPixmap( Pixmap );
    setFixedSize( Pixmap.size() );
    setMask( *Pixmap.mask() );
*/
}

DockHintWidget::~DockHintWidget()
{
}

void DockHintWidget::showAsFloating( const QPoint& Pos )
{
    m_Color = QColor( 128,128,128 );

    // This resize reduces a visual artifact of the move happening first in setGeometry
    if( !m_ShownFloating )
        resize( QSize(0,0) );

    // Move the window & size it
    setGeometry( Pos.x() + m_OffsetX, Pos.y() + m_OffsetY, m_SourceW, m_SourceH );

    // Floating at the moment
    m_ShownFloating = true;
}

void DockHintWidget::showAsDocking( DockTargetWidget* pDockTargetWidget )
{
    m_Color = QColor( 128,128,192 );

    QWidget* pTargetWidget = pDockTargetWidget->targetWidget();
    QPoint Pos = pTargetWidget->mapToGlobal( QPoint(0,0) );
    int x = Pos.x();
    int y = Pos.y();
    int w = pTargetWidget->width();
    int h = pTargetWidget->height();
    QRect r;
    float   HintSize = pDockTargetWidget->hintSize();

    switch( pDockTargetWidget->type() )
    {
    case DockTargetWidget::left:
        w = (int)(w * HintSize);
        break;
    case DockTargetWidget::right:
        x = x + w - (int)(w * HintSize);
        w = (int)(w * HintSize);
        break;
    case DockTargetWidget::top:
        h = (int)(h * HintSize);
        break;
    case DockTargetWidget::bottom:
        y = y + h - (int)(h * HintSize);
        h = (int)(h * HintSize);
        break;
    case DockTargetWidget::tab:
        break;
    }

    // Is there a position or size change?
    if( QRect(x,y,w,h) != geometry() )
    {
        // This resize reduces a visual artifact of the move happening first in setGeometry
        resize( QSize(0,0) );

        setGeometry( x, y, w, h );
    }

    // Docking at the moment
    m_ShownFloating = false;
}

void DockHintWidget::paintEvent( QPaintEvent* pEvent )
{
    (void)pEvent;

    QPainter p( this );

//    p.setBackgroundMode( Qt::TransparentMode );
//    p.setClipRect( 0, 0, 100, 200 );
    p.fillRect( 0, 0, width(), height(), m_Color );
}



MyWidget::MyWidget( QWidget* parent, const char* name, WFlags f ) :
    QWidget( parent, name, f ),
    m_pDockWidgetUnderCursor( NULL ),
    m_pDockSourceWidget( NULL )
{
    m_Id = 1;
    setBackgroundColor( QColor( 255, 255, 255 ) );
}

MyWidget::~MyWidget()
{
}

void MyWidget::hideDockWidgets( void )
{
    for( int i=0 ; i<16 ; i++ )
    {
        if( g_pDockTargetWidgets[i] )
        {
            delete g_pDockTargetWidgets[i];
            g_pDockTargetWidgets[i] = NULL;
        }
    }
}

void MyWidget::moveDockWidget( DockTargetWidget*& pWidget, DockTargetWidget::dock_type Type, QWidget* pParent, QWidget* pTargetWidget, float HintSize, QImage* pImage[2], int x, int y )
{
    if( pWidget )
    {
        pWidget->setTargetWidget( pTargetWidget );

        if( pParent != pWidget->parent() )
            pWidget->reparent( pParent, QPoint( x, y ), TRUE );
        else
            pWidget->move( x, y );
    }
    else
    {
        pWidget = new DockTargetWidget( Type, pParent, pTargetWidget, HintSize, pImage, x, y );
    }
}

void MyWidget::showDockWidgets( QWidget* pMainWidget, QWidget* pWidget )
{
    // Widgets for child widow
    if( pWidget && (pWidget != m_pDockSourceWidget) && (pWidget != pMainWidget) && pWidget->inherits( "MyWidget" ) )
    {
        QPoint tl = pWidget->mapToGlobal( QPoint( 0, 0 ) );
        tl = pMainWidget->mapFromGlobal( tl );
        int x = tl.x();
        int y = tl.y();
        int w = pWidget->width();
        int h = pWidget->height();

        w = x + w / 2;
        x = w - 45;
        w = 45*2+1;
        h = y + h / 2;
        y = h - 45;
        h = 45*2+1;

        moveDockWidget( g_pDockTargetWidgets[4], DockTargetWidget::null    , pMainWidget, pWidget, 0.5f, pIconDockBackground, x + 45 - 37                              , y + 45 - 37                               );
        moveDockWidget( g_pDockTargetWidgets[5], DockTargetWidget::tab     , pMainWidget, pWidget, 0.5f, pIconDockTab       , x + w/2 - pIconDockTab[0]->width()/2     , y + h/2 - pIconDockTab[0]->height()/2     );
        moveDockWidget( g_pDockTargetWidgets[6], DockTargetWidget::left    , pMainWidget, pWidget, 0.5f, pIconDockLeft      , x                                        , y + h/2 - pIconDockLeft[0]->height()/2    );
        moveDockWidget( g_pDockTargetWidgets[7], DockTargetWidget::right   , pMainWidget, pWidget, 0.5f, pIconDockRight     , x + w   - pIconDockRight[0]->width()     , y + h/2 - pIconDockRight[0]->height()/2   );
        moveDockWidget( g_pDockTargetWidgets[8], DockTargetWidget::top     , pMainWidget, pWidget, 0.5f, pIconDockTop       , x + w/2 - pIconDockTop[0]->width()/2     , y                                         );
        moveDockWidget( g_pDockTargetWidgets[9], DockTargetWidget::bottom  , pMainWidget, pWidget, 0.5f, pIconDockBottom    , x + w/2 - pIconDockBottom[0]->width()/2  , y + h   - pIconDockBottom[0]->height()    );
    }
    else
    {
        delete g_pDockTargetWidgets[4]; g_pDockTargetWidgets[4] = NULL;
        delete g_pDockTargetWidgets[5]; g_pDockTargetWidgets[5] = NULL;
        delete g_pDockTargetWidgets[6]; g_pDockTargetWidgets[6] = NULL;
        delete g_pDockTargetWidgets[7]; g_pDockTargetWidgets[7] = NULL;
        delete g_pDockTargetWidgets[8]; g_pDockTargetWidgets[8] = NULL;
        delete g_pDockTargetWidgets[9]; g_pDockTargetWidgets[9] = NULL;
    }

    // Widgets for main widget
    if( pMainWidget )
    {
        int x = 10;
        int y = 10;
        int w = pMainWidget->width()  - 20;
        int h = pMainWidget->height() - 20;

        moveDockWidget( g_pDockTargetWidgets[0], DockTargetWidget::left  , pMainWidget, pMainWidget, 0.20f, pIconDockLeft  , x                                        , y + h/2 - pIconDockLeft[0]->height()/2    );
        moveDockWidget( g_pDockTargetWidgets[1], DockTargetWidget::right , pMainWidget, pMainWidget, 0.20f, pIconDockRight , x + w   - pIconDockRight[0]->width()     , y + h/2 - pIconDockRight[0]->height()/2   );
        moveDockWidget( g_pDockTargetWidgets[2], DockTargetWidget::top   , pMainWidget, pMainWidget, 0.20f, pIconDockTop   , x + w/2 - pIconDockTop[0]->width()/2     , y                                         );
        moveDockWidget( g_pDockTargetWidgets[3], DockTargetWidget::bottom, pMainWidget, pMainWidget, 0.20f, pIconDockBottom, x + w/2 - pIconDockBottom[0]->width()/2  , y + h   - pIconDockBottom[0]->height()    );
    }
    else
    {
        delete g_pDockTargetWidgets[0]; g_pDockTargetWidgets[0] = NULL;
        delete g_pDockTargetWidgets[1]; g_pDockTargetWidgets[1] = NULL;
        delete g_pDockTargetWidgets[2]; g_pDockTargetWidgets[2] = NULL;
        delete g_pDockTargetWidgets[3]; g_pDockTargetWidgets[3] = NULL;
    }
}

QWidget* MyWidget::FindDockWidgetAtGlobalPos( const QPoint& Pos )
{
    QWidget* pFound = NULL;

    QWidgetList* pWidgetList = QApplication::topLevelWidgets();
    QWidgetListIt it( *pWidgetList );
    QWidget* pWidget;
    while( ( pWidget=it.current()) != 0 )
    {
        ++it;

        QWidget* pChild = FindDockWidgetAtGlobalPos( pWidget, Pos );
        if( pChild )
        {
            pFound = pChild;
            break;
        }
    }

    delete pWidgetList;

    return pFound;
}

QWidget* MyWidget::FindDockWidgetAtGlobalPos( QWidget* pParent, const QPoint& Pos )
{
    QWidget* pFound = NULL;

    const QObjectList* pChildren = pParent->children();
    if( pChildren )
    {
        QObjectListIt it( *pChildren );
        QObject* pChildObject;
        while( (pChildObject=it.current()) != 0 )
        {
            ++it;

            if( pChildObject->isWidgetType() )
            {
                QWidget* pWidget = (QWidget*)pChildObject;
                QWidget* pChild = FindDockWidgetAtGlobalPos( pWidget, Pos );
                if( pChild )
                {
                    pFound = pChild;
                }
            }
        }
    }

    if( !pFound )
    {
        if( pParent->inherits( "DockTargetWidget" ) )
        {
            // Test the position against the image this control has
            DockTargetWidget* pDockTargetWidget = (DockTargetWidget*)pParent;
            if( pDockTargetWidget->hitTest( pDockTargetWidget->mapFromGlobal(Pos) ) )
            {
                pFound = pParent;
            }
        }
        else if( pParent->inherits( "MyWidget" ) )
        {
            QPoint p = pParent->mapFromGlobal(Pos);
            if( (p.x() >= 0) && (p.x() < pParent->width()) && (p.y() >= 0) && (p.y() < pParent->height()) )
            {
                pFound = pParent;
            }
        }
    }

    return pFound;
}

void MyWidget::mouseReleaseEvent( QMouseEvent* pEvent )
{
    (void)pEvent;
    hideDockWidgets();

    delete g_pDragWidget;
    g_pDragWidget = NULL;

    m_pDockSourceWidget = NULL;
    m_pDockWidgetUnderCursor = NULL;
}

void MyWidget::mouseMoveEvent( QMouseEvent* pEvent )
{
    if( m_pDockSourceWidget )
    {
        const QPoint& CursorPos = pEvent->globalPos();
        QWidget* pWidget = FindDockWidgetAtGlobalPos( CursorPos );

        if( (pWidget != m_pDockWidgetUnderCursor) &&
            ( pWidget && ((pWidget->inherits( "DockTargetWidget" ) && (((DockTargetWidget*)pWidget)->type() != DockTargetWidget::null)) || !pWidget->inherits( "DockTargetWidget" ) )) )
        {
            // Remove any previous highlight of a dock target
            if( m_pDockWidgetUnderCursor && m_pDockWidgetUnderCursor->inherits( "DockTargetWidget" ) )
            {
                DockTargetWidget* p = (DockTargetWidget*)m_pDockWidgetUnderCursor;
                p->setHighlighted( FALSE );
            }

            // Set new widget under cursor
            m_pDockWidgetUnderCursor = pWidget;

            // Highlight the new one if it's a dock target
            if( pWidget && pWidget->inherits( "DockTargetWidget" ) )
            {
                DockTargetWidget* p = (DockTargetWidget*)pWidget;
                p->setHighlighted( TRUE );
            }
            else
            {
                showDockWidgets( g_pMainWidget, pWidget );
            }
        }

        if( g_pDragWidget )
        {
            if( m_pDockWidgetUnderCursor && m_pDockWidgetUnderCursor->inherits( "DockTargetWidget" ) )
            {
                DockTargetWidget* pWidget = (DockTargetWidget*)m_pDockWidgetUnderCursor;
                if( pWidget->type() != DockTargetWidget::floating )
                    g_pDragWidget->showAsDocking( pWidget );
                else
                    g_pDragWidget->showAsFloating( CursorPos );
            }
            else
            {
                g_pDragWidget->showAsFloating( CursorPos );
            }
        }
    }
}

void MyWidget::mousePressEvent( QMouseEvent* pEvent )
{
    if( pEvent->button() & LeftButton )
    {
//        setBackgroundColor( QColor( rand()*255/32767, rand()*255/32767, rand()*255/32767 ) );

        const QPoint& CursorPos = pEvent->globalPos();

        m_pDockSourceWidget = this;
        m_pDockWidgetUnderCursor = this;
        showDockWidgets( g_pMainWidget, m_pDockWidgetUnderCursor );

        QPoint Pos = mapToGlobal( QPoint(0,0) );
        g_pDragWidget = new DockHintWidget( Pos.x() - CursorPos.x(), Pos.y() - CursorPos.y(), width(), height() );
        g_pDragWidget->showAsFloating( CursorPos );
        int exStyle = GetWindowLong( g_pDragWidget->winId(), GWL_EXSTYLE);
        exStyle |= WS_EX_LAYERED;
        SetWindowLong( g_pDragWidget->winId(), GWL_EXSTYLE, exStyle );
        SetLayeredWindowAttributes( g_pDragWidget->winId(), 0, 128, LWA_ALPHA );
        g_pDragWidget->show( );
    }
    else
    {
        QSplitter* pParent = (QSplitter*)parentWidget();
        if( pParent )
        {
            pParent->setUpdatesEnabled( false );

            bool bHorizontal = pParent->orientation() == Qt::Vertical;

            QValueList<int> ParentSizes = pParent->sizes();
            QSize s = size();

            QSplitter* pNewSplitter;
            if( bHorizontal )
                pNewSplitter = new QSplitter( Qt::Horizontal, pParent );
            else
                pNewSplitter = new QSplitter( Qt::Vertical, pParent );
            pNewSplitter->setOpaqueResize();

            reparent( pNewSplitter, QPoint(0,0), TRUE );
            MyWidget* pNewWidget = new MyWidget( pNewSplitter, "Test", 0 ); //Qt::WNoAutoErase );
            pNewWidget->show();
            pNewWidget->m_Id = 2;

            if( m_Id == 1 )
            {
                pParent->moveToFirst( pNewSplitter );
            }
            else
            {
                pParent->moveToLast( pNewSplitter );
            }

            m_Id = 1;

            QValueList<int> Sizes;
            if( bHorizontal )
            {
                Sizes.append( s.height()/2 );
                Sizes.append( s.height()/2 );
            }
            else
            {
                Sizes.append( s.width()/2 );
                Sizes.append( s.width()/2 );
            }
            pNewSplitter->setSizes( Sizes );
            pParent->setSizes( ParentSizes );
            pNewSplitter->show();

            pParent->setUpdatesEnabled( true );
        }
    }
}
