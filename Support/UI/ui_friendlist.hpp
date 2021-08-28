//==============================================================================
//  
//  ui_friendlist.hpp
//  
//==============================================================================

#ifndef UI_FRIEND_LIST_HPP
#define UI_FRIEND_LIST_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#include "x_math.hpp"
#endif

#include "ui\ui_listbox.hpp"

//==============================================================================
//  ui_friendlist
//==============================================================================

enum
{
    ICON_FRIEND,
    ICON_FRIEND_REQ_SENT,
    ICON_FRIEND_REQ_RCVD,
    ICON_INVITE_SENT,
    ICON_INVITE_RCVD,
    ICON_VOICE_ON,
    ICON_VOICE_MUTED,
    ICON_VOICE_THRU_TV,
    ICON_VOICE_SPEAKING,
    NUM_PRESENCE_ICONS,
};

// IMPORTANT NOTE! (JP)
//
// The following flags are used to provide extra voice information in a
// buddy so that the players and friends list can be XBox TCR compliant.
// These flags will be OR'd on top of the buddy_info.Flags, so to avoid
// conflicts with the existing flags, they use the most significant bits.

enum
{
    FRIENDLIST_IS_VOICE_ALLOWED       = (1 << 28),
    FRIENDLIST_IS_VOICE_CAPABLE       = (1 << 29),
    FRIENDLIST_IS_TALKING             = (1 << 30),
    FRIENDLIST_IS_MUTED               = (1 << 31),
    FRIENDLIST_MASK                   = (FRIENDLIST_IS_VOICE_ALLOWED    |
                                         FRIENDLIST_IS_VOICE_CAPABLE    |
                                         FRIENDLIST_IS_TALKING          |
                                         FRIENDLIST_IS_MUTED)
};

extern ui_win* ui_friendlist_factory( s32 UserID, ui_manager* pManager, const irect& Position, ui_win* pParent, s32 Flags );

class ui_friendlist : public ui_listbox
{
public:

    // define some LB item flags
    enum
    {
        FLAG_ITEM_NORMAL            = 0x00000001,                   // Normal friend/player item
        FLAG_ITEM_RECENT_PLAYER     = 0x00000002,                   // Recent player item
        FLAG_ITEM_SEPARATOR         = 0x00000004,                   // Separator item
    };

                    ui_friendlist             ( void );
    virtual        ~ui_friendlist             ( void );

    virtual void    Render                  ( s32 ox=0, s32 oy=0 );

    void            RenderString            ( irect r, u32 Flags, const xcolor& c1, const xcolor& c2, const char* pString );
    void            RenderString            ( irect r, u32 Flags, const xcolor& c1, const xcolor& c2, const xwchar* pString );
    void            RenderTitle             ( irect r, u32 Flags, const xwchar* pString );
    virtual void    RenderItem              ( irect r, const item& Item, const xcolor& c1, const xcolor& c2 );

    void            Configure               ( xbool IsFriendsList );

private:
    s32             m_IconID[NUM_PRESENCE_ICONS];
    xbool           m_IsFriendsList;
};

//==============================================================================
#endif // UI_FRIEND_LIST_HPP
//==============================================================================
