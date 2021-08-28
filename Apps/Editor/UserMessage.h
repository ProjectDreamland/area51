#ifndef USERMESSAGE_H
#define USERMESSAGE_H

//General Defines for all user message, so we don't accidentally dup ID's
//this is NOT in the resource.h because it is used across multiple libs

//wparam is command index of statusbar pane to update
//lparam is a pointer to a string (text to display)
#define     WM_USER_MSG_UPDATE_STATUS_BAR       WM_USER+1

// WPARAM is an ID of the menu that is to be used. (Should have 1 submenu, index 0)
// (LOWORD)LPARAM is TRUE to add, FALSE to remove menu 
// (HIWORD)LPARAM then it is assumed to be the index of the menu to insert before
// if (HIWORD)LPARAM is -1 and (LOWORD)LPARAM is TRUE then the menu item will be inserted at the end of the list
// LRESULT is the index of the inserted item or -1 for failure
#define		WM_USER_MSG_MODIFY_MENU_BAR         WM_USER+2

// WPARAM is an ID of the menu item that is to be removed
// LPARAM is the index of the MenuBar item to be altered
#define		WM_USER_MSG_REMOVE_MENU_ITEM        WM_USER+3

//wparam is an int indicating slider position
#define		WM_USER_MSG_SLIDER_MOVED		    WM_USER+4

//indicates the property pane has changed, possibly needing a redraw of another view (no params)
#define		WM_USER_MSG_UPDATE_REQUIRED		    WM_USER+5

//wparam is a string to the identifier of the newly selected item (ie base\\rotation)
#define		WM_USER_MSG_SELECTION_CHANGE	    WM_USER+6

//wparam is a pointer to a CGridTreeItem object holding the changed item
#define		WM_USER_MSG_GRID_ITEM_CHANGE		WM_USER+7

//wparam is a pointer to a CGridTreeItem object holding the changed item
#define		WM_USER_MSG_GRID_SELECTION_CHANGE 	WM_USER+8

// WPARAM is the status text.
#define		WM_USER_MSG_SET_STATUS_TEXT      	WM_USER+9

// WPARAM is the sample rate.
#define		WM_USER_MSG_SET_STATUS_SAMPLE_RATE 	WM_USER+10

// WPARAM is the sample number.
#define		WM_USER_MSG_SET_STATUS_NUM_SAMPLES 	WM_USER+11

// WPARAM is the sart of the selection.
#define		WM_USER_MSG_SET_STATUS_SELECTION_START 	WM_USER+12

// WPARAM is the end of the selection.
#define		WM_USER_MSG_SET_STATUS_SELECTION_END 	WM_USER+13

// WPARAM is the zoom of the selection.
#define		WM_USER_MSG_SET_STATUS_ZOOM      	    WM_USER+14

//wparam is a pointer to the old file path
//lparam is a pointer to the new file path (for move its just the folder)
//if lparam is empty, a file was deleted
//otherwise a file was renamed or moved
#define		WM_USER_MSG_FILE_ITEM_PRECHANGE     	WM_USER+15
#define		WM_USER_MSG_FILE_ITEM_POSTCHANGE     	WM_USER+16

//wparam is a pointer to a CGridTreeItem object holding the changed item
#define		WM_USER_MSG_GUID_SELECT_REQUEST		    WM_USER+17

//wparam is a pointer to the property that request a selection change
#define		WM_USER_MSG_GUID_SELECT_FOR_PROPERTY    WM_USER+18

//wparam is a pointer to the global name that request a selection change
#define		WM_USER_MSG_GUID_SELECT_FOR_GLOBAL      WM_USER+19

//wparam is a pointer to the guid that needs to be highlighted
#define		WM_USER_MSG_GUID_HIGHLIGHT_REQUEST      WM_USER+20

#endif //USERMESSAGE_H
