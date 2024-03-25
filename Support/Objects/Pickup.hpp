///////////////////////////////////////////////////////////////////////////////
//
//  Pickup.hpp
//
//
///////////////////////////////////////////////////////////////////////////////


#ifndef _PICKUP_HPP
#define _PICKUP_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "x_types.hpp"
#include "Render\RigidGeom.hpp"
#include "Render\Render.hpp"
#include "Objects\Render\RigidInst.hpp"
#include "MiscUtils\SimpleUtils.hpp"
//nclude "Hud\Focus_Inst.hpp"
#include "Inventory/Inventory2.hpp"
#include "NetProjectile.hpp"

//=========================================================================
// CLASS
//=========================================================================

class actor;

//=========================================================================

class pickup : public net_proj
{
protected:
    enum state
    {
        STATE_IDLE,             // -> DECAYING, WAIT_RESPAWN
        STATE_DECAYING,         // -> FADE_OUT
        STATE_FADE_OUT,         // -> DESTROY
        STATE_WAIT_RESPAWN,     // -> RESPAWNING
        STATE_RESPAWNING,       // -> IDLE,
        STATE_DESTROY,          // Terminal
            STATE_FIRST = STATE_IDLE,
            STATE_LAST  = STATE_DESTROY,
    };

    enum
    {
        DIRTY_STATE    = 0x00010000,
        DIRTY_REQUEST  = 0x00020000,
        DIRTY_ASSIGN   = 0x00040000,
    }; 

public:

    CREATE_RTTI( pickup, net_proj, object )
    
                                pickup              ( void );
                               ~pickup              ( void );

static          pickup*         CreatePickup        ( guid              OriginGuid,
                                                      s32               OriginNetSlot,
                                                      inven_item        Item,
                                                      f32               Amount,
                                                      f32               DecayTime,
                                                      const vector3&    Position,
                                                      const radian3&    Orientation,
                                                      const vector3&    Velocity,
                                                      s32               Zone1,
                                                      s32               Zone2 );

virtual         xbool           IsActive            ( void ) { return m_bIsActive; }
virtual         void            OnActivate          ( xbool Flag );
virtual         bbox            GetLocalBBox        ( void ) const;
virtual         void            OnEnumProp          ( prop_enum& rList );
virtual         xbool           OnProperty          ( prop_query& rPropQuery );

virtual         void            OnRender            ( void );
virtual         void            OnColCheck          ( void );   
                rigid_inst&     GetRigidInst        ( void ) { return( m_Inst ); }
     
virtual         void            OnMove              ( const vector3& NewPos   );      
virtual         void            OnTransform         ( const matrix4& L2W      );    

virtual         render_inst*    GetRenderInstPtr    ( void ) { return &m_Inst; }

virtual const   object_desc&    GetTypeDesc         ( void ) const;
static  const   object_desc&    GetObjectType       ( void );

                inven_item      GetItem             ( void ) const { return m_Item; }
                f32             GetAmount           ( void ) const { return m_Amount; }

#ifndef X_EDITOR
virtual         void            net_AcceptUpdate    ( const bitstream& BS );
virtual         void            net_ProvideUpdate   (       bitstream& BS, u32& DirtyBits );
virtual         u32             net_GetUpdateMask   ( s32 TargetClient ) const;
#endif

                void            PlayerRequest       ( s32 PlayerIndex );
                xbool           GetTakeable         ( void ) { return m_bTakeable; }

protected:

virtual         void            OnInit              ( void );
virtual         void            OnKill              ( void );
virtual         void            OnAdvanceLogic      ( f32 DeltaTime );      
virtual         void            OnColNotify         ( object& Object );

                void            ProcessTake         ( actor& Actor );
                void            SetupRigidGeom      ( const char* pGeomName = NULL );
                xbool           ShouldHidePickup    ( void );

                const char*     GetSound            ( void );
                

protected:
                inven_item      m_Item;                 // Item to pickup (defined in inventory.hpp)
                f32             m_Amount;               // Amount to pickup

                state           m_State;                // State machine for pickup
                xbool           m_IsDynamic;            // Is a dynamically created pickup

                f32             m_RespawnTime;          // Time it takes for pickup to respawn
                f32             m_DecayTime;            // Time until pickup decays
                f32             m_Timer;                // Timer for next respawn
                rigid_inst      m_Inst;                 // Render Instance
                rhandle<char>   m_hAudioPackage;        // Audio resource
                rhandle<char>   m_hParticleEffect;      // Effect resource
                guid            m_FXGuid;               // FX

                s32             m_PlayerIndex;          // Player touching/receiving pickup

                xbool           m_bIsActive;            // is this item activated?
                xbool           m_bHideWhileInactive;   // do we hide this pickup if it's inactive?
                xbool           m_bHasBeenPickedup;     // has someone picked this up? (resets to false after respawn)
                xbool           m_bTakeable;            // Can this currently be picked up?

                s32             m_MinPlayers;
                s32             m_MaxPlayers;

                xbool           m_bSpins;
                matrix4         m_GeomL2W;
                radian          m_Rotation;
};

//=========================================================================
#endif
//=========================================================================