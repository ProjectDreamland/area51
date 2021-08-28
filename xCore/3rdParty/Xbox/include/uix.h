/*==========================================================================;
 *
 *  uix.h -- This module defines the Xbox Drop-In UI APIs
 *
 *  Copyright (C) Microsoft Corporation.  All Rights Reserved.
 *
 ***************************************************************************/

#ifndef __UIX__
#define __UIX__

#ifdef _XBOX
#ifndef __XONLINE__
#include "xonline.h"
#endif
#endif _XBOX

#pragma warning( push )
#pragma warning( disable : 4100 ) // unreferenced formal parameter

#ifndef UIXINLINE
#define UIXINLINE __forceinline
#endif

#ifdef __cplusplus
extern "C" {
#endif

//
// Misc. constants
//

#define UIX_MAX_LOGON_SERVICES                  10
#define UIX_INVALID_OBJECT_ID                   ((DWORD)-1)
#define UIX_INVALID_VALUE                       ((DWORD)-1)
#define UIX_MAX_ICONS_IN_TEXT                   20

//
// Features identifiers
//

typedef PVOID                                   UIX_FEATURE;

extern const UIX_FEATURE                        _uix_logon_feature;
extern const UIX_FEATURE                        _uix_friends_feature;
extern const UIX_FEATURE                        _uix_players_feature;
#ifdef _DEBUG
extern const UIX_FEATURE                        _uix_uitest_feature;
#endif

#define UIX_LOGON_FEATURE                       _uix_logon_feature
#define UIX_FRIENDS_FEATURE                     _uix_friends_feature
#define UIX_PLAYERS_FEATURE                     _uix_players_feature
#ifdef _DEBUG
#define UIX_UITEST_FEATURE                      _uix_uitest_feature
#endif


//
// Engine DoWork flags
//

#define UIX_DOWORK_NEED_TO_RENDER               0x00000001
#define UIX_DOWORK_NOTIFICATIONS                0x00000002
#define UIX_DOWORK_NEED_TO_REBOOT               0x00000004
#define UIX_DOWORK_FEATURE_EXIT                 0x00000008
#define UIX_DOWORK_PROCESSING_INPUT             0x00000010

//
// Ports for notifications
//

#define UIX_PORT_0                              0x10000000
#define UIX_PORT_1                              0x20000000
#define UIX_PORT_2                              0x40000000
#define UIX_PORT_3                              0x80000000

//
// Notifications flags per user
//

#define UIX_DOWORK_NOTIFY_FRIEND_REQUEST        0x00010000
#define UIX_DOWORK_NOTIFY_GAME_INVITE           0x00020000

//
// Controller input translated to something the screens and plugin can work with
//

#define UIX_INPUT_DIRECTIONAL_FLAG              0x00001000
#define UIX_INPUT_NOTIFICATION_FLAG             0x00000100

//
// Information required to layout icons such as the A button bitmap embedded in
// text strings
//

#define UIX_ICON_INSIDE_TEXT                    0x00000001
#define UIX_ICON_RIGHT_JUSTIFIED                0x00000002

//
// Flags for uix skin layout
//

#define UIX_LAYOUT_FLAG_CENTER_ALIGN            0x0001
#define UIX_LAYOUT_FLAG_RIGHT_ALIGN             0x0002

//
// Flags for players feature display
//

#define UIX_PLAYERS_DISPLAY_CURRENT_PLAYERS     0x00000001
#define UIX_PLAYERS_DISPLAY_DEPARTED_PLAYERS    0x00000002
#define UIX_PLAYERS_DISPLAY_LOBBY_MODE          0x00000004

//
// Feature types
//

typedef DWORD                                   UIX_FEATURE_TYPE;

#define UIX_FEATURE_TYPE_EXTENSION              0x0000F000

//
// The purpose for which ILiveEngine::GetNotifications is being called
//

typedef enum _UIX_NOTIFICATION_PURPOSE
{
    UIX_NOTIFICATION_MENU                       = 0,
    UIX_NOTIFICATION_IN_GAME_FLASH              = 1,

    UIX_NOTIFICATION_FORCE_DWORD                = 0x7fffffff
} UIX_NOTIFICATION_PURPOSE;

//
// Property types
//

typedef enum _UIX_PROPERTY_TYPE
{
    UIX_PROPERTY_DISPLAY_NOTIFICATIONS          = 0,
    UIX_PROPERTY_HOST_NO_MIGRATION              = 1,
    UIX_PROPERTY_DISPLAY_CONNECTION_ERRORS      = 2,
    UIX_PROPERTY_ALLOW_GAME_INVITES             = 3,
    UIX_PROPERTY_VOICE_MAIL_ENGINE              = 4,
    UIX_PROPERTY_VOICE_MAIL_TO_SPEAKERS         = 5,

    UIX_PROPERTY_FORCE_DWORD                    = 0x7fffffff
} UIX_PROPERTY_TYPE;

//
// Screen instance type
//

typedef PVOID                                   UIX_SCREEN;

//
// Screen message type
//

typedef DWORD                                   UIX_SCREENMSG_TYPE;

#define UIX_SCREENMSG_HIDE                      0
#define UIX_SCREENMSG_SHOW                      1
#define UIX_SCREENMSG_POPUP_SCREEN              2

//
// Feature message type
//

typedef DWORD                                   UIX_FEATUREMSG_TYPE;

#define UIX_FEATUREMSG_LOGON                    0
#define UIX_FEATUREMSG_LOGOFF                   1
#define UIX_FEATUREMSG_POPUP_SCREEN             2
#define UIX_FEATUREMSG_GET_EXIT_INFO            3
#define UIX_FEATUREMSG_GET_SELECTION_INFO       4
#define UIX_FEATUREMSG_GET_FRIENDS_LIST         11
#define UIX_FEATUREMSG_GET_NOTIFICATIONS_FLASH  12
#define UIX_FEATUREMSG_GET_NOTIFICATIONS_MENU   13
#define UIX_FEATUREMSG_NOTIFICATIONS_DISPLAY    14
#define UIX_FEATUREMSG_HOST_NO_MIGRATION        15
#define UIX_FEATUREMSG_CLEAR_NOTIFICATIONS      16
#define UIX_FEATUREMSG_SUSPEND_NOTIFICATIONS    17
#define UIX_FEATUREMSG_DISPLAY_CONNECTION_ERRORS 18
#define UIX_FEATUREMSG_ALLOW_GAME_INVITES       19
#define UIX_FEATUREMSG_LOGOFF_ALL_USERS         20
#define UIX_FEATUREMSG_GET_LAST_LOGON_TYPE      21
#define UIX_FEATUREMSG_VOICE_MAIL_DATA          22

//
// Types used by the GetExitInfo function
//

typedef DWORD                                   UIX_EXIT_CODE_TYPE;

#define UIX_EXIT_NONE                           0
#define UIX_EXIT_LOGON_SUCCESSFUL               1
#define UIX_EXIT_LOGON_FAILED                   2
#define UIX_EXIT_LOGON_USER_EXIT                3
#define UIX_EXIT_FRIENDS_NORMAL_EXIT            4
#define UIX_EXIT_FRIENDS_JOIN_GAME              5
#define UIX_EXIT_FRIENDS_JOIN_GAME_CROSS_TITLE  6
#define UIX_EXIT_FRIENDS_SIGNED_OUT             7
#define UIX_EXIT_PLAYERS_NORMAL_EXIT            8

#ifdef _DEBUG
#define UIX_EXIT_UITEST_NORMAL_EXIT             0x7ffffffe
#endif

//
// Object types for objects that are handled by a UIPlugin
//

typedef enum _UIX_OBJECT_TYPE
{
    UIX_OBJECT_NONE                             = 0,
    UIX_OBJECT_TEXTBOX                          = 1,
    UIX_OBJECT_LISTBOX                          = 2,
    UIX_OBJECT_BACKGROUND                       = 3,

    UIX_OBJECT_FORCE_DWORD                      = 0x7fffffff
} UIX_OBJECT_TYPE;

//
// Object states for objects that are handled by a UIPlugin
//

typedef enum _UIX_OBJSTATE_TYPE
{
    UIX_OBJSTATE_LIST_SELECTION_INDEX           = 0,
    UIX_OBJSTATE_LIST_ITEM_GREYED               = 1,
    UIX_OBJSTATE_LIST_ITEM_HIGHLIGHTED          = 2,
    UIX_OBJSTATE_WORD_WRAP                      = 3,

    UIX_OBJSTATE_FORCE_DWORD                    = 0x7fffffff
} UIX_OBJSTATE_TYPE;

//
// Input types used by a UIPlugin
//

typedef enum _UIX_INPUT_TYPE
{
    UIX_INPUT_NONE                          = 0x0,
    UIX_INPUT_A                             = 0x1,
    UIX_INPUT_B                             = 0x2,
    UIX_INPUT_X                             = 0x3,
    UIX_INPUT_Y                             = 0x4,
    UIX_INPUT_UP                            = 0x5 | UIX_INPUT_DIRECTIONAL_FLAG,
    UIX_INPUT_DOWN                          = 0x6 | UIX_INPUT_DIRECTIONAL_FLAG,
    UIX_INPUT_LEFT                          = 0x7 | UIX_INPUT_DIRECTIONAL_FLAG,
    UIX_INPUT_RIGHT                         = 0x8 | UIX_INPUT_DIRECTIONAL_FLAG,
    UIX_INPUT_DPAD_UP                       = 0x9 | UIX_INPUT_DIRECTIONAL_FLAG,
    UIX_INPUT_DPAD_DOWN                     = 0xA | UIX_INPUT_DIRECTIONAL_FLAG,
    UIX_INPUT_DPAD_LEFT                     = 0xB | UIX_INPUT_DIRECTIONAL_FLAG,
    UIX_INPUT_DPAD_RIGHT                    = 0xC | UIX_INPUT_DIRECTIONAL_FLAG,
    UIX_INPUT_LEFT_TRIGGER                  = 0xD,
    UIX_INPUT_RIGHT_TRIGGER                 = 0xE,
    UIX_INPUT_START                         = 0xF,
    UIX_INPUT_BACK                          = 0x10,
    UIX_INPUT_BLACK                         = 0x11,
    UIX_INPUT_WHITE                         = 0x12,

    UIX_INPUT_CONTROLLER_IN                 = 0x1 | UIX_INPUT_NOTIFICATION_FLAG,
    UIX_INPUT_CONTROLLER_OUT                = 0x2 | UIX_INPUT_NOTIFICATION_FLAG,
    UIX_INPUT_ALL_CONTROLLERS_OUT           = 0x3 | UIX_INPUT_NOTIFICATION_FLAG,

    UIX_INPUT_FORCE_DWORD                   = 0x7fffffff
} UIX_INPUT_TYPE;

//
// Structure definitions to describe layout information for an object
//

#pragma pack(push, 1)

typedef struct _UIX_SKIN_LAYOUT_INFO
{
    WORD        X;
    WORD        Y;
    WORD        Width;
    WORD        Height;
    DWORD       ImageOffset;
    D3DCOLOR    BackColor;
    D3DCOLOR    TextColor;
    D3DCOLOR    DisabledTextColor;
    D3DCOLOR    SelectionBackColor;
    D3DCOLOR    HighlightedTextColor;
    WORD        FontHeight;
    WORD        Flags;
    WORD        XOffset;
    WORD        YOffset;
    DWORD       CustomParam;
} UIX_SKIN_LAYOUT_INFO, *PUIX_SKIN_LAYOUT_INFO;

typedef struct _UIX_SKIN_ICON_INFO
{
    DWORD       IconResID;
    DWORD       InsertPosInText;
    DWORD       Flags;
} UIX_SKIN_ICON_INFO, *PUIX_SKIN_ICON_INFO;

#pragma pack(pop)


#ifdef _XBOX

//
// Logon behavior specified in UIX_LOGON_PARAMS
//

typedef enum _UIX_LOGON_TYPE
{
    UIX_LOGON_TYPE_NORMAL                   = 0,
    UIX_LOGON_TYPE_SILENT                   = 1,
    UIX_LOGON_TYPE_RETRIEVED_STATE          = 2,
    UIX_LOGON_TYPE_RETRIEVED_GAME_INVITE    = 3,

    UIX_LOGON_FORCE_DWORD                   = 0x7fffffff
} UIX_LOGON_TYPE;


//
// Communicator status type
//

typedef enum _UIX_VOICE_STATUS_TYPE
{
    UIX_VOICE_STATUS_COMMUNICATOR           = 0,    // Voice through communicator
    UIX_VOICE_STATUS_SPEAKERS               = 1,    // Voice through speakers
    UIX_VOICE_STATUS_NONE                   = 2,    // No voice

    UIX_VOICE_STATUS_FORCE_DWORD            = 0x7fffffff
} UIX_VOICE_STATUS_TYPE;

//
// Voice Mail entry point
//

typedef PVOID                                   UIX_VOICE_MAIL_ENTRY_POINT;
extern const UIX_VOICE_MAIL_ENTRY_POINT        _uix_voice_mail;
#define UIX_VOICE_MAIL                         _uix_voice_mail


#pragma pack(push, 4)


//
// Logon feature parameters structure.  Must zero-init this structure
// and set the StructSize field with size of the structure first.
//
//      StructSize      - The size of the structure
//
//      LogonType       - Type of logon such as normal, silent etc
//
//      LogonUserCount  - Number of users allowed to logon.  The
//          value can be 1, 2, 3 or 4.  The feature will select the
//          the correct screen for logon based on this value
//
//      LogonServiceIDs - Service IDs of the required services.  All
//          service values should be consecutively specified starting
//          from index zero.
//
//      pLogonState     - Saved logon state to use for logon
//
//      pGameInvite     - Game invite to use for logon
//

typedef struct _UIX_LOGON_PARAMS
{
    SIZE_T                       StructSize;
    UIX_LOGON_TYPE               LogonType;
    DWORD                        LogonUserCount;
    DWORD                        LogonServiceIDs[UIX_MAX_LOGON_SERVICES];
    XONLINE_LOGON_STATE*         pLogonState;
    XONLINE_ACCEPTED_GAMEINVITE* pGameInvite;

} UIX_LOGON_PARAMS, *PUIX_LOGON_PARAMS;


//
// Friends feature parameters structure.  Must zero-init this structure
// and set the StructSize field with size of the structure first.
//
//      StructSize      - The size of the structure
//
//      UserPort        - The port of the controller the user is logged on with
//
//      SelectedFriendXUID - Highlighted friend in initial friends list
//
//      SignOutEnabled  - TRUE if the game wants th Sign Out button available
//                        from the friends screen
//

typedef struct _UIX_FRIENDS_PARAMS {

    SIZE_T      StructSize;
    DWORD       UserPort;
    XUID        SelectedFriendXUID;
    BOOL        SignOutEnabled;

} UIX_FRIENDS_PARAMS, *PUIX_FRIENDS_PARAMS;


#ifdef _DEBUG
typedef struct _UIX_UITEST_PARAMS {

    SIZE_T      StructSize;

} UIX_UITEST_PARAMS, *PUIX_UITEST_PARAMS;
#endif


//
// Information returned in response to UIX_FEATUREMSG_GET_EXIT_INFO feature message.
//

typedef struct _UIX_EXIT_INFO
{
    UIX_FEATURE         FeatureID;
    UIX_EXIT_CODE_TYPE  ExitCode;
    HRESULT             hr;
    PVOID               pExitData;

} UIX_EXIT_INFO, *PUIX_EXIT_INFO;


//
// Information passed with UIX_FEATUREMSG_POPUP_SCREEN feature message.
//

typedef struct _UIX_POPUP_INFO
{
    DWORD               Reserved1;
    UIX_INPUT_TYPE      InputKey;
    DWORD               Reserved2;
    DWORD               InputPort;
    DWORD               SelectedStringResID;

} UIX_POPUP_INFO, *PUIX_POPUP_INFO;

//
// Information passed with UIX_FEATUREMSG_VOICE_MAIL_DATA feature message.
//

typedef struct _UIX_VOICE_MAIL_INFO
{
    DWORD               InputPort;
    DWORD               RecordedVoiceMailDataDuration;
    DWORD               RecordedVoiceMailDataSize;
} UIX_VOICE_MAIL_INFO, *PUIX_VOICE_MAIL_INFO;


//
// Information passed with UIX_EXIT_LOGON_SUCCESSFUL feature message
// It indicates which were the controllers used to log on.
// (can be -1 if no controller was used in the logon process for the
// given screen part). In silent logon it's always (0, -1, -1, -1)
//

typedef struct _UIX_LOGON_EXIT_DATA
{
    DWORD               pMappedControllers[XONLINE_MAX_LOGON_USERS];

} UIX_LOGON_EXIT_DATA, *PUIX_LOGON_EXIT_DATA;


#ifdef _DEBUG

//
// Counts of active allocations
//

extern DWORD UIX__EngineAllocations;
extern DWORD UIX__TextureAllocations;
#endif

#pragma pack(pop)


//-----------------------------------------------------------------------------
// Interfaces
//-----------------------------------------------------------------------------

typedef struct PluginSupport            PluginSupport;
typedef struct LiveFriendsList          LiveFriendsList;
typedef struct LiveEngine               LiveEngine;


#define IPluginSupport                  PluginSupport
#define ILiveFriendsList                LiveFriendsList
#define ILiveEngine                     LiveEngine


typedef struct PluginSupport           *LPPluginSupport, *PPluginSupport;
typedef struct LiveFriendsList         *LPLiveFriendsList, *PLiveFriendsList;
typedef struct LiveEngine              *LPLiveEngine, *PLiveEngine;


//
// ITitleFontRenderer interface
//    - Interface implemented by a font renderer enabling callers
//      to draw text
//
//      Release - Called when the client is finished using this interface
//
//      SetHeight - Set the height of the font
//
//      SetColor - Set the color of the font
//
//      DrawText - Draw pText (null-terminated) on the current render target
//                 at X, Y location in screen coordinates.  MaxWidth is maximum
//                 width allowed so the font can be faded or truncated if too
//                 long.  The value can also be (DWORD)-1 which means that
//                 there is no maximum width
//
//      GetTextSize - Return the width and height of pText (null terminated) in
//                    screen coordinates
//

#undef  INTERFACE
#define INTERFACE   ITitleFontRenderer

DECLARE_INTERFACE(ITitleFontRenderer)
{

    STDMETHOD_(ULONG, Release)(THIS) PURE;

    STDMETHOD(SetHeight)(      THIS_ IN  DWORD                Height)    PURE;

    STDMETHOD(SetColor)(       THIS_ IN  D3DCOLOR             Color)     PURE;

    STDMETHOD(DrawText)(       THIS_ IN  LPCWSTR              pText,
                                     IN  DWORD                X,
                                     IN  DWORD                Y,
                                     IN  DWORD                MaxWidth)  PURE;

    STDMETHOD(GetTextSize)(    THIS_ IN  LPCWSTR              pText,
                                     OUT DWORD               *pWidth,
                                     OUT DWORD               *pHeight)   PURE;
};

typedef struct ITitleFontRenderer         *LPTitleFontRenderer, *PTitleFontRenderer;


//
// ITitleUIPlugin interface
//    - Interface implemented by a UIPlugin.  This interface is used
//      by the engine to do its UI related work
//

#undef  INTERFACE
#define INTERFACE   ITitleUIPlugin

DECLARE_INTERFACE(ITitleUIPlugin)
{
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    STDMETHOD(DoWork)(THIS)          PURE;

    STDMETHOD(SetPluginSupport)(     THIS_ IN  IPluginSupport            *pPluginSupport) PURE;

    STDMETHOD(CreateObject)(         THIS_ IN  UIX_OBJECT_TYPE            ObjectType,
                                           IN  DWORD                      ScreenInstance,
                                           IN  DWORD                      ScreenResID,
                                           IN  DWORD                      ObjectResID,
                                           OUT DWORD                     *pObjectID)      PURE;

    STDMETHOD(DestroyObject)(        THIS_ IN  DWORD                      ObjectID)       PURE;

    STDMETHOD(DestroyScreenObjects)( THIS_ IN  DWORD                      ScreenInstance) PURE;

    STDMETHOD(SetRenderTarget)(      THIS_ IN  IDirect3DSurface8         *pSurface)       PURE;

    STDMETHOD(RenderObject)(         THIS_ IN  DWORD                      ObjectID)       PURE;

    STDMETHOD(SetText)(              THIS_ IN  DWORD                      ObjectID,
                                           IN  DWORD                      ItemIndex,
                                           IN  LPCWSTR                    pText,
                                           IN  DWORD                      IconCount,
                                           IN  const UIX_SKIN_ICON_INFO  *pIconInfo)      PURE;

    STDMETHOD(InsertItem)(           THIS_ IN  DWORD                      ObjectID,
                                           IN  DWORD                      ItemIndex,
                                           OUT DWORD                     *pReturnIndex)   PURE;

    STDMETHOD(SetObjectState)(       THIS_ IN  DWORD                      ObjectID,
                                           IN  DWORD                      ItemIndex,
                                           IN  UIX_OBJSTATE_TYPE          State,
                                           IN  DWORD                      Value)          PURE;

    STDMETHOD(GetObjectState)(       THIS_ IN  DWORD                      ObjectID,
                                           IN  DWORD                      ItemIndex,
                                           IN  UIX_OBJSTATE_TYPE          State,
                                           OUT DWORD                     *pValue)         PURE;

    STDMETHOD(PassInputToObject)(    THIS_ IN  DWORD                      ObjectID,
                                           IN  UIX_INPUT_TYPE             InputKey)       PURE;

    STDMETHOD(Clear)(                THIS_ IN  DWORD                      ObjectID,
                                           IN  BOOL                       ResetSelectionIndex) PURE;

    STDMETHOD(GetFont)(              THIS_ OUT ITitleFontRenderer       **ppFont) PURE;
};

typedef struct ITitleUIPlugin                          *LPTitleUIPlugin, *PTitleUIPlugin;


//
// ITitleAudioPlugin interface
//    - Interface implemented by a sound system to allow a caller
//      to play simple sounds
//

#undef  INTERFACE
#define INTERFACE   ITitleAudioPlugin

DECLARE_INTERFACE(ITitleAudioPlugin)
{
    STDMETHOD_(ULONG, Release)( THIS)             PURE;

    STDMETHOD(PlaySound)(THIS_  LPCSTR SoundName) PURE;

    STDMETHOD(DoWork)(          THIS)             PURE;
};


typedef struct ITitleAudioPlugin      *LPTitleAudioPlugin, *PTitleAudioPlugin;


//
// ITitlePlayersListItem interface
//    - Interface implemented by the title that allows us to get information
//      about each player
//

#undef  INTERFACE
#define INTERFACE   ITitlePlayersListItem

DECLARE_INTERFACE(ITitlePlayersListItem)
{
    STDMETHOD_(CONST WCHAR*, GetName)   (THIS)                        CONST PURE;

    STDMETHOD_(CONST XUID*, GetXUID)    (THIS)                        CONST PURE;

    STDMETHOD_(UIX_VOICE_STATUS_TYPE, GetVoiceStatus)(THIS)           CONST PURE;

    STDMETHOD_(BOOL, IsTalking)         (THIS)                        CONST PURE;

    STDMETHOD_(DWORD, GetBitflags)      (THIS)                        CONST PURE;

#ifdef __cplusplus
    STDMETHOD_(INT, Compare)(CONST ITitlePlayersListItem *)           CONST
    {
        return 0;
    }
#else
    STDMETHOD_(INT, Compare)(CONST ITitlePlayersListItem *)           CONST PURE;
#endif
};

//
// IUIXScreen interface
//      - Implemented by a UIX screen object
//

#undef  INTERFACE
#define INTERFACE   IUIXScreen

DECLARE_INTERFACE(IUIXScreen)
{
    STDMETHOD(CreateScreen)(THIS)                                           PURE;

    STDMETHOD_(VOID, Output)(THIS)                                          PURE;

    STDMETHOD_(VOID, Input)(THIS_       IN DWORD Port,
                                        IN UIX_INPUT_TYPE InputValue)       PURE;

    STDMETHOD(ReceiveMessage)(THIS_     IN UIX_SCREENMSG_TYPE Msg,
                                        IN const VOID* pParam)              PURE;
};

typedef struct IUIXScreen               *LPUIXScreen, *PUIXScreen;


//
// IUIXEngineInternal
//      - This interface is passed to in the UIX_FEATURE_CONTEXT structure
//        to an extended feature.  The extended feature can use internal
//        UIX engine functionality using this interface.  It is not meant
//        to be used by an application using the UIX feature.
//

#undef  INTERFACE
#define INTERFACE   IUIXEngineInternal

DECLARE_INTERFACE(IUIXEngineInternal)
{
    STDMETHOD(CreateScreen)(THIS_       OUT UIX_SCREEN* pScreenObject,
                                        IN DWORD ScreenResID,
                                        IN IUIXScreen* pScreenInterface)    PURE;

    STDMETHOD(DestroyScreen)(THIS_      IN UIX_SCREEN ScreenObject)         PURE;

    STDMETHOD(ShowScreen)(THIS_         IN UIX_SCREEN ScreenObject,
                                        IN BOOL ReplaceCurrent)             PURE;

    STDMETHOD(HideTopScreen)(THIS)                                          PURE;

    STDMETHOD(EnableScreenInput)(THIS_  IN UIX_SCREEN ScreenObject,
                                        IN BOOL Enable)                     PURE;

    STDMETHOD(AllowStartAndBack)(THIS_  IN UIX_SCREEN ScreenObject,
                                        IN BOOL Allow)                      PURE;

    STDMETHOD(CreateObject)(THIS_       OUT DWORD* pObjectID,
                                        IN UIX_SCREEN ScreenObject,
                                        IN UIX_OBJECT_TYPE ObjectType,
                                        IN DWORD ObjectResID)               PURE;

    STDMETHOD(SetText)(THIS_            IN UIX_SCREEN ScreenObject,
                                        IN DWORD ObjectID,
                                        IN LPCWSTR pText)                   PURE;

    STDMETHOD(SetTextWithResID)(THIS_   IN UIX_SCREEN ScreenObject,
                                        IN DWORD ObjectID,
                                        IN DWORD StringResID)               PURE;

    STDMETHOD(AddListItem)(THIS_        IN UIX_SCREEN ScreenObject,
                                        IN DWORD ObjectID,
                                        IN LPCWSTR pText,
                                        IN DWORD IconCount,
                                        IN const UIX_SKIN_ICON_INFO* pIconInfo) PURE;

    STDMETHOD(AddGreyListItem)(THIS_    IN UIX_SCREEN ScreenObject,
                                        IN DWORD ObjectID,
                                        IN LPCWSTR pText,
                                        IN DWORD IconCount,
                                        IN const UIX_SKIN_ICON_INFO* pIconInfo) PURE;

    STDMETHOD(ClearList)(THIS_          IN UIX_SCREEN ScreenObject,
                                        IN DWORD ObjectID,
                                        IN BOOL ResetSelectionIndex)        PURE;

    STDMETHOD(SeparateTextAndIcons)(THIS_ IN OUT LPCWSTR* ppText,
                                        OUT DWORD* pIconCount,
                                        OUT UIX_SKIN_ICON_INFO** ppIcons)   PURE;

    STDMETHOD(SendMessageToAllFeatures)(THIS_ IN UIX_FEATUREMSG_TYPE Msg,
                                        IN const VOID* pParam)              PURE;

    STDMETHOD(PlaySound)(THIS_          IN DWORD SoundResID)                PURE;

    STDMETHOD(LaunchDash)(THIS_         IN DWORD Reason,
                                        IN DWORD Parameter1,
                                        IN DWORD Parameter2)                PURE;

    STDMETHOD(ShowPopup)(THIS_          IN LPCWSTR  pTitleString,
                                        IN DWORD    MessageStringResID,
                                        IN DWORD    ActionButtonStringResID,
                                        IN DWORD    BackButtonStringResID,
                                        IN OPTIONAL LPCWSTR  pParamString)  PURE;

    STDMETHOD_(BOOL, CanUseVoiceMailForPort)(THIS_ IN DWORD       Port)   PURE;                                        
                                        
    STDMETHOD(ShowVoiceMailScreen)(THIS_ IN DWORD       Port,
                                         IN XUID        xuidUser, 
                                         IN LPCWSTR     pTitle,
                                         IN LPCWSTR     pSubject,
                                         IN BOOL        Recording,
                                         IN DWORD       MessageDuration,
                                         IN DWORD       VoiceMessageBufferSize,
                                         IN BYTE       *pVoiceMessageBuffer)  PURE;
    
    STDMETHOD(SetVoiceMailPlayScreenData)(THIS_ IN DWORD       Port,
											    IN DWORD       MessageDuration,
												IN DWORD       VoiceMessageBufferSize,
												IN BYTE       *pVoiceMessageBuffer)  PURE;                                         
                                         
};


//
// Information returned in response to UIX_FEATUREMSG_GET_SELECTION_INFO
// feature message.
//

typedef struct _UIX_SELECTION_INFO
{
    UIX_FEATURE             FeatureID;
    XONLINE_FRIEND          SelectedFriend;
    ITitlePlayersListItem*  pSelectedCurrentPlayer;
    PVOID                   pSelectedOther;
    BOOL                    CanDisplay;

} UIX_SELECTION_INFO, *PUIX_SELECTION_INFO;


//
// Information passed to IUIXFeature::SetContext for extended features
//

typedef struct _UIX_FEATURE_CONTEXT
{
    ILiveEngine*        pEngine;
    IUIXEngineInternal* pEngineInternal;
    IPluginSupport*     pPluginSupport;
    ITitleUIPlugin*     pUIPlugin;
    ITitleFontRenderer* pFont;

} UIX_FEATURE_CONTEXT, *PUIX_FEATURE_CONTEXT;


//
// IUIXFeature interface
//      - Implemented by a feature. Also described here is the context
//        information passed to SetContext for extended features
//

#undef  INTERFACE
#define INTERFACE   IUIXFeature

DECLARE_INTERFACE(IUIXFeature)
{
    STDMETHOD(CreateFeature)(THIS)                                          PURE;

    STDMETHOD_(VOID, DestroyFeature)(THIS)                                  PURE;

    STDMETHOD(ActivateFeature)(THIS_    IN const VOID* pParams)             PURE;

    STDMETHOD_(VOID, HibernateFeature)(THIS)                                PURE;

    STDMETHOD_(VOID, PumpTasks)(THIS)                                       PURE;

    STDMETHOD_(VOID, Reset)(THIS)                                           PURE;

    STDMETHOD(ReceiveMessage)(THIS_     IN UIX_FEATUREMSG_TYPE Msg,
                                        IN const VOID* pParam,
                                        IN VOID* pResult,
                                        IN IUIXFeature* pFromFeature)       PURE;

    STDMETHOD(GetCurrentScreen)(THIS_   OUT UIX_SCREEN* pScreen)            PURE;

    STDMETHOD(GetType)(THIS_            OUT UIX_FEATURE_TYPE* pType)        PURE;

    STDMETHOD(GetFeatureInterface)(THIS_ IN const VOID* pParam,
                                        OUT VOID** ppInterface)             PURE;

    STDMETHOD(SetContext)(THIS_         IN UIX_FEATURE_CONTEXT* pContext)   PURE;

};

typedef struct IUIXFeature              *LPUIXFeature, *PUIXFeature;


//
// ILivePlayersList interface
//      - Interface for the players list feature
//

#undef  INTERFACE
#define INTERFACE   ILivePlayersList

DECLARE_INTERFACE(ILivePlayersList)
{
    STDMETHOD(RegisterPlayer)(THIS_     IN CONST ITitlePlayersListItem* pPlayer)  PURE;

    STDMETHOD(UnregisterPlayer)(THIS_   IN CONST ITitlePlayersListItem* pPlayer)  PURE;

    STDMETHOD(Refresh)(THIS)                                                PURE;

    STDMETHOD(SetFilterFlags)(THIS_     IN CONST DWORD Flags)               PURE;

    STDMETHOD(ClearDepartedPlayersList)(THIS)                               PURE;
};

typedef struct ILivePlayersList         *LPLivePlayersList, *PLivePlayersList;


//
// Players feature parameters structure.  Must zero-init this structure
// and set the StructSize field with size of the structure first.
//
//      StructSize               - The size of the structure
//
//      UserPort                 - The port of the controller used to control the players screen
//                                 If UserPort is set to UIX_INVALID_VALUE (-1), the feature will be
//                                 forced into lobby mode (UIX_PLAYERS_DISPLAY_LOBBY_MODE will be OR'd
//                                 with the specified DisplayType)
//
//      pPlayerControllingScreen - The player given control of the screen.  Usually the player assigned
//                                 to the port designated by UserPort.
//                                 This player will be displayed as the first player in the list
//                                 But won't be selectable (can be highlighted, nothing happens though)
//                                 Can be NULL (players will appear in registered order, or sorted if
//                                 SortCurrentPlayersList is true).
//
//      pSelectedPlayerXUID      - Pointer to the XUID of the player that should be selected when
//                                 the screen is first shown.  If NULL, the first available player
//                                 will be selected
//
//      SortCurrentPlayersList   - Sorts the current players list using the compare function in
//                                 ITitlePlayersListItem.  If false, no sorting is done.  Sorting
//                                 takes priority over pPlayerControllingScreen - the player will
//                                 appear in sorted order rather than at the top of the list
//
//      FilterFlags              - A bitmask that is ANDed against flags returned by
//                                 ITitlePlayersListItem::GetBitFlags() to filter which
//                                 players are shown on screen.  If this member is 0,
//                                 all players are shown.
//
//      DisplayType              - One or more of the flags in UIX_PLAYERS_DISPLAY_*.  This
//                                 changes the screen layout depending on the input
//                                 You must specify one of UIX_PLAYERS_DISPLAY_CURRENT_PLAYERS or
//                                 UIX_PLAYERS_DISPLAY_DEPARTED_PLAYERS.  If UIX_PLAYERS_DISPLAY_LOBBY_MODE
//                                 is specified, the UserPort will be forced to UIX_INVALID_VALUE
//

typedef struct _UIX_PLAYERS_PARAMS
{
    SIZE_T                      StructSize;
    DWORD                       UserPort;
    ITitlePlayersListItem*      pPlayerControllingScreen;
    CONST XUID*                 pSelectedPlayerXUID;
    BOOL                        SortCurrentPlayersList;
    DWORD                       FilterFlags;
    DWORD                       DisplayType;
} UIX_PLAYERS_PARAMS, *PUIX_PLAYERS_PARAMS;


//-----------------------------------------------------------------------------
// NOTE: The C version of the methods for all of the interfaces
//       are named "<interfacename>_<method name>" and have an
//       explicit pointer to the interface as the first parameter.
//       The actual definition of these methods is at the end
//       of this file.
//-----------------------------------------------------------------------------

//
// IPluginSupport interface
//    - Interface used by the plugin to access services implemented by the engine
//      such as skin support and word wrapping
//

#ifdef __cplusplus

struct PluginSupport
{
    ULONG   WINAPI AddRef();

    ULONG   WINAPI Release();

    HRESULT WINAPI GetString(      IN  DWORD                   StringResID,
                                   OUT LPCWSTR                *ppString);

    HRESULT WINAPI GetLayout(      IN  DWORD                   ScreenResID,
                                   IN  DWORD                   ObjectResID,
                                   OUT UIX_SKIN_LAYOUT_INFO  **ppLayout);

    HRESULT WINAPI GetImage(       IN  DWORD                   ImageResID,
                                   OUT IDirect3DTexture8     **ppTexture);

    HRESULT WINAPI GetScreenImage( IN  DWORD                   ScreenResID,
                                   IN  DWORD                   ImageResID,
                                   OUT IDirect3DTexture8     **ppTexture);

    HRESULT WINAPI GetWordLength(  IN  LPCWSTR                 pString,
                                   OUT DWORD                  *pWordLength);
};

#endif __cplusplus


ULONG   WINAPI PluginSupport_AddRef(         IN  PluginSupport          *pThis);

ULONG   WINAPI PluginSupport_Release(        IN  PluginSupport          *pThis);

HRESULT WINAPI PluginSupport_GetString(      IN  PluginSupport          *pThis,
                                             IN  DWORD                   StringResID,
                                             OUT LPCWSTR                *ppString);

HRESULT WINAPI PluginSupport_GetLayout(      IN  PluginSupport          *pThis,
                                             IN  DWORD                   ScreenResID,
                                             IN  DWORD                   ObjectResID,
                                             OUT UIX_SKIN_LAYOUT_INFO  **ppLayout);

HRESULT WINAPI PluginSupport_GetImage(       IN  PluginSupport          *pThis,
                                             IN  DWORD                   ImageResID,
                                             OUT IDirect3DTexture8     **ppTexture);

HRESULT WINAPI PluginSupport_GetScreenImage( IN  PluginSupport          *pThis,
                                             IN  DWORD                   ScreenResID,
                                             IN  DWORD                   ImageResID,
                                             OUT IDirect3DTexture8     **ppTexture);

HRESULT WINAPI PluginSupport_GetWordLength(  IN  PluginSupport          *pThis,
                                             IN  LPCWSTR                 pString,
                                             OUT DWORD                  *pWordLength);

//
// ILiveFriendsList interface
//    - Interface exposed by the engine for the friends list
//

#ifdef __cplusplus

struct LiveFriendsList
{
    ULONG   WINAPI AddRef();

    ULONG   WINAPI Release();

    HRESULT WINAPI Refresh();

    BOOL    WINAPI IsReady();

    BOOL    WINAPI HasChanged();

    DWORD   WINAPI Count();

    HRESULT WINAPI GetFriendByIndex( IN  DWORD                  FriendIndex,
                                     OUT XONLINE_FRIEND        *pFriend);

    HRESULT WINAPI GetFriendByXUID(  IN  XUID                   FriendXUID,
                                     OUT XONLINE_FRIEND        *pFriend);

    HRESULT WINAPI Remove(           IN  const XONLINE_FRIEND  *pFriend);

    HRESULT WINAPI Request(          IN  XUID                   UserXUID);
};

#endif __cplusplus


ULONG   WINAPI LiveFriendsList_AddRef(           IN  LiveFriendsList      *pThis);

ULONG   WINAPI LiveFriendsList_Release(          IN  LiveFriendsList      *pThis);

HRESULT WINAPI LiveFriendsList_Refresh(          IN  LiveFriendsList      *pThis);

BOOL    WINAPI LiveFriendsList_IsReady(          IN  LiveFriendsList      *pThis);

BOOL    WINAPI LiveFriendsList_HasChanged(       IN  LiveFriendsList      *pThis);

DWORD   WINAPI LiveFriendsList_Count(            IN  LiveFriendsList      *pThis);

HRESULT WINAPI LiveFriendsList_GetFriendByIndex( IN  LiveFriendsList      *pThis,
                                                 IN  DWORD                 FriendIndex,
                                                 OUT XONLINE_FRIEND       *pFriend);

HRESULT WINAPI LiveFriendsList_GetFriendByXUID(  IN  LiveFriendsList      *pThis,
                                                 IN  XUID                  FriendXUID,
                                                 OUT XONLINE_FRIEND       *pFriend);

HRESULT WINAPI LiveFriendsList_Remove(           IN  LiveFriendsList      *pThis,
                                                 IN  const XONLINE_FRIEND *pFriend);

HRESULT WINAPI LiveFriendsList_Request(          IN  LiveFriendsList      *pThis,
                                                 IN  XUID                  UserXUID);


//
// ILiveEngine interface
//    - Main interface exposed by the live engine
//

#ifdef __cplusplus

struct LiveEngine
{
    ULONG   WINAPI AddRef();

    ULONG   WINAPI Release();

    HRESULT WINAPI DoWork(                OUT DWORD                    *pDoWorkFlags);

    HRESULT WINAPI SetUIPlugin(           IN  ITitleUIPlugin           *pUIPlugin);

    HRESULT WINAPI SetAudioPlugin(        IN  ITitleAudioPlugin        *pAudioPlugin);

    HRESULT WINAPI EnableFeature(         IN  UIX_FEATURE               FeatureID);

    HRESULT WINAPI StartFeature(          IN  UIX_FEATURE               FeatureID,
                                          IN  const VOID               *pFeatureParams);

    HRESULT WINAPI Render(                IN  IDirect3DSurface8        *pSurface);

    HRESULT WINAPI SetInput(              IN  DWORD                     Port,
                                          IN  const XINPUT_STATE       *pInputState);

    HRESULT WINAPI GetFriendsForUser(     IN  DWORD                     Port,
                                          OUT ILiveFriendsList        **ppFriendsList);

    HRESULT WINAPI NotificationSetState(  IN  DWORD                     Port,
                                          IN  DWORD                     StateFlags,
                                          IN  XNKID                     SessionID,
                                          IN  DWORD                     StateDataSize,
                                          IN  const PVOID               pStateData);

    HRESULT WINAPI GetNotifications(      IN  DWORD                     Port,
                                          IN  UIX_NOTIFICATION_PURPOSE  Purpose,
                                          OUT DWORD                    *pNotifications);

    HRESULT WINAPI SetProperty(           IN  UIX_PROPERTY_TYPE         Property,
                                          IN  DWORD                     Value);

    HRESULT WINAPI GetProperty(           IN  UIX_PROPERTY_TYPE         Property,
                                          OUT DWORD                    *pValue);

    HRESULT WINAPI GetExitInfo(           OUT UIX_EXIT_INFO            *pExitInfo);

    HRESULT WINAPI Reboot(                IN  DWORD                     Context);

    HRESULT WINAPI LogOff(                VOID);

    HRESULT WINAPI GetFeatureInterface(   IN  UIX_FEATURE               FeatureID,
                                          IN  const VOID*               pParam,
                                          OUT VOID**                    ppFeatureInterface);

    HRESULT WINAPI GetSelectionInfo(      OUT UIX_SELECTION_INFO       *pSelectionInfo);

    HRESULT WINAPI EndFeature(            VOID);

    HRESULT WINAPI ClearInput(            VOID);
    
    HRESULT WINAPI UseVoiceMail(          IN UIX_VOICE_MAIL_ENTRY_POINT VoiceMailEntryPoint);
};

#endif __cplusplus

ULONG   WINAPI LiveEngine_AddRef(                IN  LiveEngine               *pThis);

ULONG   WINAPI LiveEngine_Release(               IN  LiveEngine               *pThis);

HRESULT WINAPI LiveEngine_DoWork(                IN  LiveEngine               *pThis,
                                                 OUT DWORD                    *pDoWorkFlags);

HRESULT WINAPI LiveEngine_SetUIPlugin(           IN  LiveEngine               *pThis,
                                                 IN  ITitleUIPlugin           *pUIPlugin);

HRESULT WINAPI LiveEngine_SetAudioPlugin(        IN  LiveEngine               *pThis,
                                                 IN  ITitleAudioPlugin        *pAudioPlugin);

HRESULT WINAPI LiveEngine_EnableFeature(         IN  LiveEngine               *pThis,
                                                 IN  UIX_FEATURE               FeatureID);

HRESULT WINAPI LiveEngine_StartFeature(          IN  LiveEngine               *pThis,
                                                 IN  UIX_FEATURE               FeatureID,
                                                 IN  const VOID               *pFeatureParams);

HRESULT WINAPI LiveEngine_Render(                IN  LiveEngine               *pThis,
                                                 IN  IDirect3DSurface8        *pSurface);

HRESULT WINAPI LiveEngine_SetInput(              IN  LiveEngine               *pThis,
                                                 IN  DWORD                     Port,
                                                 IN  const XINPUT_STATE       *pInputState);

HRESULT WINAPI LiveEngine_GetFriendsForUser(     IN  LiveEngine               *pThis,
                                                 IN  DWORD                     Port,
                                                 OUT ILiveFriendsList        **ppFriendsList);

HRESULT WINAPI LiveEngine_NotificationSetState(  IN  LiveEngine               *pThis,
                                                 IN  DWORD                     Port,
                                                 IN  DWORD                     StateFlags,
                                                 IN  XNKID                     SessionID,
                                                 IN  DWORD                     StateDataSize,
                                                 IN  const PVOID               pStateData);

HRESULT WINAPI LiveEngine_GetNotifications(      IN  LiveEngine               *pThis,
                                                 IN  DWORD                     Port,
                                                 IN  UIX_NOTIFICATION_PURPOSE  Purpose,
                                                 OUT DWORD                    *pNotifications);

HRESULT WINAPI LiveEngine_SetProperty(           IN  LiveEngine               *pThis,
                                                 IN  UIX_PROPERTY_TYPE         Property,
                                                 IN  DWORD                     Value);

HRESULT WINAPI LiveEngine_GetProperty(           IN  LiveEngine               *pThis,
                                                 IN  UIX_PROPERTY_TYPE         Property,
                                                 OUT DWORD                    *pValue);

HRESULT WINAPI LiveEngine_GetExitInfo(           IN  LiveEngine               *pThis,
                                                 OUT UIX_EXIT_INFO            *pExitInfo);

HRESULT WINAPI LiveEngine_Reboot(                IN  LiveEngine               *pThis,
                                                 IN  DWORD                     Context);

HRESULT WINAPI LiveEngine_LogOff(                IN  LiveEngine               *pThis);

HRESULT WINAPI LiveEngine_GetFeatureInterface(   IN  LiveEngine               *pThis,
                                                 IN  UIX_FEATURE               FeatureID,
                                                 IN  const VOID*               pParam,
                                                 OUT VOID**                    ppFeatureInterface);

HRESULT WINAPI LiveEngine_GetSelectionInfo(      IN  LiveEngine               *pThis,
                                                 OUT UIX_SELECTION_INFO       *pSelectionInfo);

HRESULT WINAPI LiveEngine_EndFeature(            IN  LiveEngine               *pThis);

HRESULT WINAPI LiveEngine_ClearInput(            IN  LiveEngine               *pThis);

HRESULT WINAPI LiveEngine_UseVoiceMail(          IN  LiveEngine               *pThis,
                                                 IN  UIX_VOICE_MAIL_ENTRY_POINT VoiceMailEntryPoint);

//-----------------------------------------------------------------------------
// UIX functions
//-----------------------------------------------------------------------------

XBOXAPI
HRESULT
WINAPI
UIXCreateLiveEngine(
    IN  LPCSTR                          pSkinFileName,
    IN  DWORD                           LanguageID,
    OUT ILiveEngine                   **ppEngine
    );

XBOXAPI
HRESULT
WINAPI
UIXCreateUIPlugin(
    IN  ITitleFontRenderer             *pFont,
    OUT ITitleUIPlugin                **ppUIPlugin
    );

XBOXAPI
HRESULT
WINAPI
UIXCreateAudioPlugin(
    IN  VOID                           *pXactEngine,
    IN  VOID                           *pXactSoundBank,
    OUT ITitleAudioPlugin             **ppAudioPlugin
    );

//-----------------------------------------------------------------------------
//  Compatibility wrappers for the interfaces
//-----------------------------------------------------------------------------

//
// IPluginSupport wrappers
//

UIXINLINE ULONG   WINAPI IPluginSupport_AddRef(PluginSupport *pThis)
                         { return PluginSupport_AddRef(pThis); }
UIXINLINE ULONG   WINAPI IPluginSupport_Release(PluginSupport *pThis)
                         { return PluginSupport_Release(pThis); }
UIXINLINE HRESULT WINAPI IPluginSupport_GetString(PluginSupport *pThis, DWORD StringResID, LPCWSTR *ppString)
                         { return PluginSupport_GetString(pThis, StringResID, ppString);}
UIXINLINE HRESULT WINAPI IPluginSupport_GetLayout( PluginSupport *pThis, DWORD ScreenResID, DWORD ObjectResID, UIX_SKIN_LAYOUT_INFO **ppLayout)
                         { return PluginSupport_GetLayout( pThis, ScreenResID, ObjectResID, ppLayout); }
UIXINLINE HRESULT WINAPI IPluginSupport_GetImage(PluginSupport *pThis, DWORD ImageResID, IDirect3DTexture8 **ppTexture)
                         { return PluginSupport_GetImage(pThis, ImageResID, ppTexture);}
UIXINLINE HRESULT WINAPI IPluginSupport_GetScreenImage( PluginSupport *pThis, DWORD ScreenResID, DWORD ImageResID, IDirect3DTexture8 **ppTexture)
                         { return PluginSupport_GetScreenImage(pThis, ScreenResID, ImageResID, ppTexture);}
UIXINLINE HRESULT WINAPI IPluginSupport_GetWordLength( PluginSupport *pThis, LPCWSTR pString, DWORD *pWordLength)
                         { return PluginSupport_GetWordLength( pThis, pString, pWordLength); }

#ifdef __cplusplus

UIXINLINE ULONG   WINAPI IPluginSupport::AddRef()
                         { return PluginSupport_AddRef(this); }
UIXINLINE ULONG   WINAPI IPluginSupport::Release()
                         { return PluginSupport_Release(this); }
UIXINLINE HRESULT WINAPI IPluginSupport::GetString(DWORD StringResID, LPCWSTR *ppString)
                         { return PluginSupport_GetString(this, StringResID, ppString);}
UIXINLINE HRESULT WINAPI IPluginSupport::GetLayout(DWORD ScreenResID, DWORD ObjectResID, UIX_SKIN_LAYOUT_INFO **ppLayout)
                         { return PluginSupport_GetLayout( this, ScreenResID, ObjectResID, ppLayout); }
UIXINLINE HRESULT WINAPI IPluginSupport::GetImage(DWORD ImageResID, IDirect3DTexture8 **ppTexture)
                         { return PluginSupport_GetImage(this, ImageResID, ppTexture);}
UIXINLINE HRESULT WINAPI IPluginSupport::GetScreenImage(DWORD ScreenResID, DWORD ImageResID, IDirect3DTexture8 **ppTexture)
                         { return PluginSupport_GetScreenImage(this, ScreenResID, ImageResID, ppTexture);}
UIXINLINE HRESULT WINAPI IPluginSupport::GetWordLength( LPCWSTR pString, DWORD *pWordLength)
                         { return PluginSupport_GetWordLength( this,  pString, pWordLength); }

#endif __cplusplus

//
// Compatibility wrappers - ILiveFriendsList
//

UIXINLINE ULONG   WINAPI ILiveFriendsList_AddRef(LiveFriendsList *pThis)
                         { return LiveFriendsList_AddRef(pThis); }
UIXINLINE ULONG   WINAPI ILiveFriendsList_Release(LiveFriendsList *pThis)
                         { return LiveFriendsList_Release(pThis); }
UIXINLINE HRESULT WINAPI ILiveFriendsList_Refresh(LiveFriendsList *pThis)
                         { return LiveFriendsList_Refresh(pThis); }
UIXINLINE BOOL    WINAPI ILiveFriendsList_IsReady(LiveFriendsList *pThis)
                         { return LiveFriendsList_IsReady(pThis); }
UIXINLINE BOOL    WINAPI ILiveFriendsList_HasChanged(LiveFriendsList *pThis)
                         { return LiveFriendsList_HasChanged(pThis); }
UIXINLINE DWORD   WINAPI ILiveFriendsList_Count(LiveFriendsList *pThis)
                         { return LiveFriendsList_Count(pThis); }
UIXINLINE HRESULT WINAPI ILiveFriendsList_GetFriendByIndex(LiveFriendsList *pThis, DWORD FriendIndex, XONLINE_FRIEND *pFriend)
                         { return LiveFriendsList_GetFriendByIndex(pThis, FriendIndex, pFriend); }
UIXINLINE HRESULT WINAPI ILiveFriendsList_GetFriendByXUID(LiveFriendsList *pThis, XUID FriendXUID, XONLINE_FRIEND *pFriend)
                         { return LiveFriendsList_GetFriendByXUID(pThis, FriendXUID, pFriend); }
UIXINLINE HRESULT WINAPI ILiveFriendsList_Remove(LiveFriendsList *pThis, const XONLINE_FRIEND *pFriend)
                         { return LiveFriendsList_Remove(pThis, pFriend); }
UIXINLINE HRESULT WINAPI ILiveFriendsList_Request(LiveFriendsList *pThis, XUID UserXUID)
                         { return LiveFriendsList_Request(pThis, UserXUID); }

#ifdef __cplusplus

UIXINLINE ULONG   WINAPI ILiveFriendsList::AddRef()
                         { return LiveFriendsList_AddRef(this); }
UIXINLINE ULONG   WINAPI ILiveFriendsList::Release()
                         { return LiveFriendsList_Release(this); }
UIXINLINE HRESULT WINAPI ILiveFriendsList::Refresh()
                         { return LiveFriendsList_Refresh(this); }
UIXINLINE BOOL    WINAPI ILiveFriendsList::IsReady()
                         { return LiveFriendsList_IsReady(this); }
UIXINLINE BOOL    WINAPI ILiveFriendsList::HasChanged()
                         { return LiveFriendsList_HasChanged(this); }
UIXINLINE DWORD   WINAPI ILiveFriendsList::Count()
                         { return LiveFriendsList_Count(this); }
UIXINLINE HRESULT WINAPI ILiveFriendsList::GetFriendByIndex(DWORD FriendIndex, XONLINE_FRIEND *pFriend)
                         { return LiveFriendsList_GetFriendByIndex(this, FriendIndex, pFriend); }
UIXINLINE HRESULT WINAPI ILiveFriendsList::GetFriendByXUID(XUID FriendXUID, XONLINE_FRIEND *pFriend)
                         { return LiveFriendsList_GetFriendByXUID(this, FriendXUID, pFriend); }
UIXINLINE HRESULT WINAPI ILiveFriendsList::Remove(const XONLINE_FRIEND *pFriend)
                         { return LiveFriendsList_Remove(this, pFriend); }
UIXINLINE HRESULT WINAPI ILiveFriendsList::Request(XUID UserXUID)
                         { return LiveFriendsList_Request(this, UserXUID); }

#endif __cplusplus

//
// Compatibility wrappers - ILiveEngine
//

UIXINLINE ULONG   WINAPI ILiveEngine_AddRef(LiveEngine *pThis)
                         { return LiveEngine_AddRef(pThis); }
UIXINLINE ULONG   WINAPI ILiveEngine_Release(LiveEngine *pThis)
                         { return LiveEngine_Release(pThis); }
UIXINLINE HRESULT WINAPI ILiveEngine_DoWork(LiveEngine *pThis, DWORD *pDoWorkFlags)
                         { return LiveEngine_DoWork(pThis, pDoWorkFlags); }
UIXINLINE HRESULT WINAPI ILiveEngine_SetUIPlugin(LiveEngine *pThis, ITitleUIPlugin *pUIPlugin)
                         { return LiveEngine_SetUIPlugin(pThis, pUIPlugin); }
UIXINLINE HRESULT WINAPI ILiveEngine_SetAudioPlugin(LiveEngine *pThis, ITitleAudioPlugin *pAudioPlugin)
                         { return LiveEngine_SetAudioPlugin(pThis, pAudioPlugin); }
UIXINLINE HRESULT WINAPI ILiveEngine_EnableFeature(LiveEngine *pThis, UIX_FEATURE FeatureID)
                         { return LiveEngine_EnableFeature(pThis, FeatureID); }
UIXINLINE HRESULT WINAPI ILiveEngine_StartFeature(LiveEngine *pThis, UIX_FEATURE FeatureID, const VOID *pFeatureParams)
                         { return LiveEngine_StartFeature(pThis, FeatureID, pFeatureParams); }
UIXINLINE HRESULT WINAPI ILiveEngine_Render(LiveEngine *pThis, IDirect3DSurface8 *pSurface)
                         { return LiveEngine_Render(pThis, pSurface); }
UIXINLINE HRESULT WINAPI ILiveEngine_SetInput(LiveEngine *pThis, DWORD Port, const XINPUT_STATE *pInputState)
                         { return LiveEngine_SetInput(pThis, Port, pInputState); }
UIXINLINE HRESULT WINAPI ILiveEngine_GetFriendsForUser(LiveEngine *pThis, DWORD Port, ILiveFriendsList **ppFriendsList)
                         { return LiveEngine_GetFriendsForUser(pThis, Port, ppFriendsList); }
UIXINLINE HRESULT WINAPI ILiveEngine_NotificationSetState(LiveEngine *pThis, DWORD Port, DWORD StateFlags, XNKID SessionID, DWORD StateDataSize, const PVOID pStateData)
                         { return LiveEngine_NotificationSetState(pThis, Port, StateFlags, SessionID, StateDataSize, pStateData); }
UIXINLINE HRESULT WINAPI ILiveEngine_GetNotifications(LiveEngine *pThis, DWORD Port, UIX_NOTIFICATION_PURPOSE Purpose, DWORD *pNotifications)
                         { return LiveEngine_GetNotifications(pThis, Port, Purpose, pNotifications); }
UIXINLINE HRESULT WINAPI ILiveEngine_SetProperty(LiveEngine *pThis, UIX_PROPERTY_TYPE Property, DWORD Value)
                         { return LiveEngine_SetProperty(pThis, Property, Value); }
UIXINLINE HRESULT WINAPI ILiveEngine_GetProperty(LiveEngine *pThis, UIX_PROPERTY_TYPE Property, DWORD* pValue)
                         { return LiveEngine_GetProperty(pThis, Property, pValue); }
UIXINLINE HRESULT WINAPI ILiveEngine_GetExitInfo(LiveEngine* pThis, UIX_EXIT_INFO* pExitInfo)
                         { return LiveEngine_GetExitInfo(pThis, pExitInfo); }
UIXINLINE HRESULT WINAPI ILiveEngine_Reboot(LiveEngine *pThis, DWORD Context)
                         { return LiveEngine_Reboot(pThis, Context); }
UIXINLINE HRESULT WINAPI ILiveEngine_LogOff(LiveEngine *pThis)
                         { return LiveEngine_LogOff(pThis); }
UIXINLINE HRESULT WINAPI ILiveEngine_GetFeatureInterface(LiveEngine* pThis, UIX_FEATURE FeatureID, const VOID* pParam, VOID** ppFeatureInterface)
                         { return LiveEngine_GetFeatureInterface(pThis, FeatureID, pParam, ppFeatureInterface); }
UIXINLINE HRESULT WINAPI ILiveEngine_GetSelectionInfo(LiveEngine* pThis, UIX_SELECTION_INFO* pSelectionInfo)
                         { return LiveEngine_GetSelectionInfo(pThis, pSelectionInfo); }
UIXINLINE HRESULT WINAPI ILiveEngine_EndFeature(LiveEngine *pThis)
                         { return LiveEngine_EndFeature(pThis); }
UIXINLINE HRESULT WINAPI ILiveEngine_ClearInput(LiveEngine *pThis)
                         { return LiveEngine_ClearInput(pThis); }
UIXINLINE HRESULT WINAPI ILiveEngine_UseVoiceMail(LiveEngine *pThis, UIX_VOICE_MAIL_ENTRY_POINT VoiceMailEntryPoint)
                         {return LiveEngine_UseVoiceMail(pThis, VoiceMailEntryPoint); }


#ifdef __cplusplus

UIXINLINE ULONG   WINAPI ILiveEngine::AddRef()
                         { return LiveEngine_AddRef(this); }
UIXINLINE ULONG   WINAPI ILiveEngine::Release()
                         { return LiveEngine_Release(this); }
UIXINLINE HRESULT WINAPI ILiveEngine::DoWork(DWORD *pDoWorkFlags)
                         { return LiveEngine_DoWork(this, pDoWorkFlags); }
UIXINLINE HRESULT WINAPI ILiveEngine::SetUIPlugin(ITitleUIPlugin *pUIPlugin)
                         { return LiveEngine_SetUIPlugin(this, pUIPlugin); }
UIXINLINE HRESULT WINAPI ILiveEngine::SetAudioPlugin(ITitleAudioPlugin *pAudioPlugin)
                         { return LiveEngine_SetAudioPlugin(this, pAudioPlugin); }
UIXINLINE HRESULT WINAPI ILiveEngine::EnableFeature(UIX_FEATURE FeatureID)
                         { return LiveEngine_EnableFeature(this, FeatureID); }
UIXINLINE HRESULT WINAPI ILiveEngine::StartFeature(UIX_FEATURE FeatureID, const VOID *pFeatureParams)
                         { return LiveEngine_StartFeature(this, FeatureID, pFeatureParams); }
UIXINLINE HRESULT WINAPI ILiveEngine::Render(IDirect3DSurface8 *pSurface)
                         { return LiveEngine_Render(this, pSurface); }
UIXINLINE HRESULT WINAPI ILiveEngine::SetInput(DWORD Port, const XINPUT_STATE *pInputState)
                         { return LiveEngine_SetInput(this, Port, pInputState); }
UIXINLINE HRESULT WINAPI ILiveEngine::GetFriendsForUser(DWORD Port, ILiveFriendsList **ppFriendsList)
                         { return LiveEngine_GetFriendsForUser(this, Port, ppFriendsList); }
UIXINLINE HRESULT WINAPI ILiveEngine::NotificationSetState(DWORD Port, DWORD StateFlags, XNKID SessionID, DWORD StateDataSize, const PVOID pStateData)
                         { return LiveEngine_NotificationSetState(this, Port, StateFlags, SessionID, StateDataSize, pStateData); }
UIXINLINE HRESULT WINAPI ILiveEngine::GetNotifications(DWORD Port, UIX_NOTIFICATION_PURPOSE Purpose, DWORD *pNotifications)
                         { return LiveEngine_GetNotifications(this, Port, Purpose, pNotifications); }
UIXINLINE HRESULT WINAPI ILiveEngine::SetProperty(UIX_PROPERTY_TYPE Property, DWORD Value)
                         { return LiveEngine_SetProperty(this, Property, Value); }
UIXINLINE HRESULT WINAPI ILiveEngine::GetProperty(UIX_PROPERTY_TYPE Property, DWORD* pValue)
                         { return LiveEngine_GetProperty(this, Property, pValue); }
UIXINLINE HRESULT WINAPI ILiveEngine::GetExitInfo(UIX_EXIT_INFO* pExitInfo)
                         { return LiveEngine_GetExitInfo(this, pExitInfo); }
UIXINLINE HRESULT WINAPI ILiveEngine::Reboot(DWORD Context)
                         { return LiveEngine_Reboot(this, Context); }
UIXINLINE HRESULT WINAPI ILiveEngine::LogOff()
                         { return LiveEngine_LogOff(this); }
UIXINLINE HRESULT WINAPI ILiveEngine::GetFeatureInterface(UIX_FEATURE FeatureID, const VOID* pParam, VOID** ppFeatureInterface)
                         { return LiveEngine_GetFeatureInterface(this, FeatureID, pParam, ppFeatureInterface); }
UIXINLINE HRESULT WINAPI ILiveEngine::GetSelectionInfo(UIX_SELECTION_INFO* pSelectionInfo)
                         { return LiveEngine_GetSelectionInfo(this, pSelectionInfo); }
UIXINLINE HRESULT WINAPI ILiveEngine::EndFeature()
                         { return LiveEngine_EndFeature(this); }
UIXINLINE HRESULT WINAPI ILiveEngine::ClearInput()
                         { return LiveEngine_ClearInput(this); }
UIXINLINE HRESULT WINAPI ILiveEngine::UseVoiceMail(UIX_VOICE_MAIL_ENTRY_POINT VoiceMailEntryPoint)
                         {return LiveEngine_UseVoiceMail(this, VoiceMailEntryPoint); }
                         

#endif __cplusplus

//
// Compatibility wrappers - End
//

#endif _XBOX


#ifdef __cplusplus
}
#endif

#pragma warning( pop )

#endif // __UIX__
