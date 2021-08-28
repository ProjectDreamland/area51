#ifndef __MANIPULATORMGR_H
#define __MANIPULATORMGR_H

class CManipulator;
class CManipTranslate;
class CManipScale;

//============================================================================
// ManipulatorMgr.h
//============================================================================

class CManipulatorMgr 
{
public:

                        CManipulatorMgr();
                       ~CManipulatorMgr();

    void                Render                  ( const view& View );

    CManipTranslate*    NewManipTranslate       ( void );
    CManipScale*        NewManipScale           ( void );
    void                DeleteManipulator       ( CManipulator* pManipulator );

    xbool               Update                  ( const view& View, f32 ScreenX, f32 ScreenY );
    xbool               BeginDrag               ( const view& View, f32 ScreenX, f32 ScreenY );
    xbool               EndDrag                 ( const view& View );

private:
    CPtrList            m_Manipulators;

    CManipulator*       m_pHighlightedManipulator;
    xbool               m_IsDragging;
};

extern CManipulatorMgr g_ManipulatorMgr;

#endif //MANIPULATORMGR_H
