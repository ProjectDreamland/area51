#include "qsplitter.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qwidget.h"
#include "qmainwindow.h"

extern QImage* pIconDockLeft[2];
extern QImage* pIconDockRight[2];
extern QImage* pIconDockTop[2];
extern QImage* pIconDockBottom[2];
extern QImage* pIconDockTab[2];
extern QImage* pIconDockBackground[2];


class DockTargetWidget : public QWidget
{
    Q_OBJECT

public:
    enum dock_type
    {
        null,
        floating,
        tab,
        left,
        right,
        top,
        bottom,
    };

protected:
    dock_type   m_Type;
    QWidget*    m_pTargetWidget;
    float       m_HintSize;
    QImage*     m_pImage[2];

public:
                DockTargetWidget            ( dock_type Type, QWidget* parent, QWidget* pTargetWidget, float HintSize, QImage* pImage[2], int x, int y );
               ~DockTargetWidget            ( );

    void        setHighlighted              ( bool State );
    void        setTargetWidget             ( QWidget* pTargetWidget );
    dock_type   type                        ( void );
    QWidget*    targetWidget                ( void );
    float       hintSize                    ( void );
    bool        hitTest                     ( const QPoint& Point );
};

class DockHintWidget : public QWidget
{
    Q_OBJECT

protected:
    bool        m_ShownFloating;
    QColor      m_Color;
    int         m_OffsetX;
    int         m_OffsetY;
    int         m_SourceW;
    int         m_SourceH;

public:
                DockHintWidget              ( int OffsetX, int OffsetY, int SourceW, int SourceH );
               ~DockHintWidget              ( );

    void        showAsFloating              ( const QPoint& Pos );
    void        showAsDocking               ( DockTargetWidget* pDockTargetWidget );

protected:
    void        paintEvent                  ( QPaintEvent* pEvent );
};

class MyWidget : public QWidget
{
    Q_OBJECT

protected:
    QWidget*    m_pDockWidgetUnderCursor;
    QWidget*    m_pDockSourceWidget;

public:
    int         m_Id;

                MyWidget                    ( QWidget* parent=0, const char* name=0, WFlags f=0 );
               ~MyWidget                    ( );

    void        hideDockWidgets             ( void );
    void        showDockWidgets             ( QWidget* pMainWidget, QWidget* pWidget );
    QWidget*    FindDockWidgetAtGlobalPos   ( const QPoint& Pos );
    QWidget*    FindDockWidgetAtGlobalPos   ( QWidget* pParent, const QPoint& Pos );

    void        mouseReleaseEvent           ( QMouseEvent* pEvent );
    void        mouseMoveEvent              ( QMouseEvent* pEvent );
    void        mousePressEvent             ( QMouseEvent* pEvent );

protected:
    void        moveDockWidget              ( DockTargetWidget*& pWidget, DockTargetWidget::dock_type Type, QWidget* pParent, QWidget* pTargetWidget, float HintSize, QImage* pImage[2], int x, int y );

};
