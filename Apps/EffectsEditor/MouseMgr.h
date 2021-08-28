//==============================================================================
//
//  MouseMgr.h
//
//	A class for keeping track of the mouse
//
//==============================================================================

#ifndef __MOUSEMGR_H__
#define __MOUSEMGR_H__

//----------------------+
//	Includes			|
//----------------------+


//--------------------------+
// MouseMgr - Definition    |
//--------------------------+

class MouseMgr
{
    public:
                         MouseMgr   ( void );
        virtual         ~MouseMgr   ( void );

        // Must attach MouseMgr to a window before you can use it!!
        void            AttachToWindow  ( CWnd* pCWnd );

        void            OnButtonDown    ( UINT nFlags, CPoint point );
        void            OnButtonUp      ( UINT nFlags, CPoint point );
        void            OnMove          ( UINT nFlags, CPoint point );
        void            OnWheel         ( UINT nFlags, CPoint point, short zDelta );

        const CPoint&   GetPos          ( void )    const;
        const CPoint&   GetClickPos     ( void )    const;
        const CPoint&   GetDragPos      ( void )    const;

        const xbool&    GetButton_Left  ( void )    const;
        const xbool&    GetButton_Middle( void )    const;
        const xbool&    GetButton_Right ( void )    const;

        const xbool&    GetKey_Shift    ( void )    const;
        const xbool&    GetKey_Control  ( void )    const;
        const xbool&    GetKey_Alt      ( void )    const;

    protected:
        /*****************************************************************************************/
        /********** PLACEHOLDER: Need to figure out how to handle mouse state stuff  *************/
        /*****************************************************************************************/
        enum MouseState
        {
            MOUSE_IDLE,         // Mouse is not doing anything

            MOUSE_CLICK_L,      // Left   button was clicked...Mouse has not moved yet
            MOUSE_CLICK_M,      // Middle button was clicked...Mouse has not moved yet
            MOUSE_CLICK_R,      // Right  button was clicked...Mouse has not moved yet

            MOUSE_CANCEL,       // Right  button was clicked while in a drag mode

            MOUSE_RELEASE_L,    // Left   button was released
            MOUSE_RELEASE_M,    // Middle button was released
            MOUSE_RELEASE_R,    // Right  button was released

            MOUSE_MOVE,         // Mouse is moving with NO     buttons held down

            MOUSE_DRAG_L,       // Mouse is moving with LEFT   button held down
            MOUSE_DRAG_M,       // Mouse is moving with MIDDLE button held down
            MOUSE_DRAG_R,       // Mouse is moving with RIGHT  button held down
        };
        /*****************************************************************************************/

        CWnd*   m_pCWnd;            // Pointer to the window we're managing

        CPoint  m_Pos;              // The point the mouse is currently at
        CPoint  m_ClickPos;         // The last point you clicked - for drag operations
        CPoint  m_DragPos;          // The difference between m_Pos & m_ClickPos

        s32     m_Wheel;            // Wheel scroll value

        xbool   m_Button_Left;      // True if Left   Mouse Button is clicked
        xbool   m_Button_Middle;    // True if Middle Mouse Button is clicked
        xbool   m_Button_Right;     // True if Right  Mouse Button is clicked

        xbool   m_Key_Shift;        // True if Shift   key is down
        xbool   m_Key_Control;      // True if Control key is down
        xbool   m_Key_Alt;          // True if Alt     key is down

        //------------------------------------------------------------------------------+

        void    GetMouseButtons     ( UINT nFlags );    // Gets the state of all 3 mouse buttons
        void    GetModifierKeys     ( UINT nFlags );    // Gets the state of the SHIFT, CTRL, & ALT keys
};

//------------------------------------------------------------------------------+
//------------------------------------------------------------------------------+

#endif  //#ifndef __MOUSEMGR_H__
