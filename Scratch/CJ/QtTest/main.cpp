#include "qapplication.h"
#include "qlabel.h"
#include "qsplitter.h"
#include "qtextedit.h"
#include "qdockwindow.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qwidget.h"
#include "qmainwindow.h"
#include "qaction.h"

#include "MyWidget.h"

QMainWindow* g_pMainWidget = 0;

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    g_pMainWidget = new QMainWindow;
    g_pMainWidget->setCaption( QObject::tr("Splitter") );
    a.setMainWidget( g_pMainWidget );

    QSplitter splitter( Qt::Horizontal, g_pMainWidget );
    g_pMainWidget->setCentralWidget( &splitter );

    splitter.setOpaqueResize();
    splitter.setHandleWidth( 4 );

    pIconDockLeft[0]       = new QImage( QImage::fromMimeSource("icon_dock_left.png")           );
    pIconDockLeft[1]       = new QImage( QImage::fromMimeSource("icon_dock_left_hi.png")        );
    pIconDockRight[0]      = new QImage( QImage::fromMimeSource("icon_dock_right.png")          );
    pIconDockRight[1]      = new QImage( QImage::fromMimeSource("icon_dock_right_hi.png")       );
    pIconDockTop[0]        = new QImage( QImage::fromMimeSource("icon_dock_top.png")            );
    pIconDockTop[1]        = new QImage( QImage::fromMimeSource("icon_dock_top_hi.png")         );
    pIconDockBottom[0]     = new QImage( QImage::fromMimeSource("icon_dock_bottom.png")         );
    pIconDockBottom[1]     = new QImage( QImage::fromMimeSource("icon_dock_bottom_hi.png")      );
    pIconDockTab[0]        = new QImage( QImage::fromMimeSource("icon_dock_tab.png")            );
    pIconDockTab[1]        = new QImage( QImage::fromMimeSource("icon_dock_tab_hi.png")         );
    pIconDockBackground[0] = new QImage( QImage::fromMimeSource("icon_dock_background.png")     );
    pIconDockBackground[1] = new QImage( QImage::fromMimeSource("icon_dock_background_hi.png")  );

    for( int i=1 ; i<=2 ; i++ )
    {
        MyWidget* pWidget = new MyWidget( &splitter, "Test", 0 ); //Qt::WNoAutoErase );
        pWidget->m_Id = i;
//        pWidget->setBackgroundColor( QColor( 255, 255, 255 ) );
        splitter.setResizeMode( pWidget, QSplitter::Stretch );
        pWidget->show();
    }

    g_pMainWidget->resize( 500, 500 );
    g_pMainWidget->showMaximized();

    a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( quit() ) );
    return a.exec();
}
