//==============================================================================
//
//  NetGhost.hpp
//
//  This object only exists in a networked game. 
//
//  It is not controlled by a human or AI - it's just a puppet that
//  updates itself based on the network updates it receives from the server.
//  It does not perform collision with the world, it just blends to the incoming
//  positions and orientations it receives.
//
//==============================================================================

#ifndef NET_GHOST_HPP
#define NET_GHOST_HPP

#if defined(X_EDITOR)
#error NetGhost.hpp should not be included in the editor
#endif          

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Objects\Actor\Actor.hpp"
#include "Objects\Render\SkinInst.hpp"
#include "Objects\NewWeapon.hpp"
#include "NetworkMgr\Blender.hpp"
                                
//==============================================================================
//  DEFINES
//==============================================================================

//==============================================================================
//  TYPES
//==============================================================================

//==============================================================================
// CLASSES
//==============================================================================

class net_ghost : public actor
{
//------------------------------------------------------------------------------
// CLASS INFO
//------------------------------------------------------------------------------
public:
        CREATE_RTTI( net_ghost, actor, object )

virtual const   object_desc&    GetTypeDesc             ( void ) const;
static  const   object_desc&    GetObjectType           ( void );


//------------------------------------------------------------------------------
//  STRUCTURES
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
//  FUNCTIONS
//------------------------------------------------------------------------------

                                net_ghost                   ( void );
virtual                        ~net_ghost                   ( void );
                                                        
protected:

    // Object virtual functions
    virtual s32             GetMaterial                 ( void ) const { return MAT_TYPE_FLESH; }
    virtual void            OnInit                      ( void );
    virtual void            OnKill                      ( void );
    virtual void            OnPain                      ( const pain& Pain );
    virtual void            OnDeath                     ( void );
    virtual void            OnAdvanceLogic              ( f32 DeltaTime );
    virtual void            OnRender                    ( void );
    virtual void            OnRenderTransparent         ( void );
    virtual void            OnEvent                     ( const event& Event );
            void            EmitMeleePain               ( void );

public:

    // Actor virtual functions
    virtual xbool           IsNetGhost                  ( void )    { return TRUE; }
    virtual void            OnSpawn                     ( void );
    virtual actor::eHitType OverrideFlinchType          ( actor::eHitType hitType );
    virtual const char*     GetLogicalName              ( void ) { return "PLAYER"; }

            s32             GetNetSlot                  ( void )    { return m_NetSlot; }

//------------------------------------------------------------------------------
//  NETWORK FUNCTIONS
//------------------------------------------------------------------------------

public:

    virtual void            net_Activate        ( void );
    virtual void            net_AcceptUpdate    ( const update& Update );
    virtual void            net_ProvideUpdate   (       update& Update, u32& DirtyBits );
    virtual void            net_Logic           ( f32 DeltaTime );
    virtual xbool           net_EquipWeapon2    ( inven_item WeaponItem );

//------------------------------------------------------------------------------
//  DATA
//------------------------------------------------------------------------------

protected:


//------------------------------------------------------------------------------
//  NETWORKING STUFF
//------------------------------------------------------------------------------
protected:

    struct locale
    {
        // Functions
        locale();

        // Data
        vector3 Position;
        radian  Yaw;
        radian  Pitch;
        f32     Lean;
    };

    struct net
    {
        // Functions
        net( void );

        // Data
        s32             LocoType;

        locale          Actual;
        locale          Render;
                        
        blender         BlendPosX;
        blender         BlendPosY;
        blender         BlendPosZ;
        blender         BlendPitch;
        blender         BlendYaw;
        blender         BlendLean;

        s32             FrameDelay; // Frames of logic since last update.
        s32             Frames[2];  // Last 2 values of FrameDelay.

        vector3         WayPoint[2];
        xbool           DoTeleport;
        xbool           DoWayPoint;
    };

    //------------------------------------------------------------------
    
    net     m_Net;  // <<---- All net_ghost specific networking variables!

    static f32 m_BlindLogic;
    static s32 m_NInstances;

//------------------------------------------------------------------------------

};

//==============================================================================
#endif // #ifndef NET_GHOST_HPP
//==============================================================================