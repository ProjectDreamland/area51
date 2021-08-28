//==============================================================================
//
//  NetPlayer.hpp
//
//==============================================================================

#ifndef NETPLAYER_HPP
#define NETPLAYER_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_math.hpp"
#include "NetObj.hpp"
#include "NetObjMgr.hpp"
#include "MoveMgr.hpp"

//==============================================================================
//  DEFINES
//==============================================================================

extern s32 FramesToBlend;

//==============================================================================
//  blender
//==============================================================================

class blender
{
    f32     m_DeltaToRestart;
    f32     m_DeltaToPop;
    f32     m_Current;
    f32     m_Target;
    f32     m_Delta;
    s32     m_Frames;
    s32     m_FramesToBlend;
    xbool   m_IsAngular;

public:
    blender()
    {
        m_DeltaToRestart    = 0.0f;
        m_DeltaToPop        = 1.0f;
        m_Current           = 0.0f;
        m_Target            = 0.0f;
        m_Delta             = 0.0f;
        m_Frames            = 0;
        m_FramesToBlend     = 6;
        m_IsAngular         = FALSE;
    }

    void Init( f32 DeltaToRestart, f32 DeltaToPop, s32 FramesToBlend, xbool IsAngular )
    {
        m_DeltaToRestart    = DeltaToRestart;
        m_DeltaToPop        = DeltaToPop;
        m_FramesToBlend     = FramesToBlend;
        m_IsAngular         = IsAngular;
    }

    void SetTarget( f32 Target )
    {
        f32 Delta       = Target - m_Target;
        f32 AbsDelta    = (Delta >= 0.0f) ? Delta : -Delta;

        // Check for wrapping on angles
        if( m_IsAngular )
        {
            if( AbsDelta > PI )
            {
                if( Delta >= 0.0f )
                {
                    m_Target   += PI*2;
                    m_Current  += PI*2;
                }
                else
                {
                    m_Target   -= PI*2;
                    m_Current  -= PI*2;
                }

                Delta       = Target - m_Target;
                AbsDelta    = (Delta > 0.0f) ? Delta : -Delta;
            }
        }

        // Accept the new target but don't reset the number of blend frames
        if( AbsDelta < m_DeltaToRestart )
        {
            m_Frames    = MAX(1,m_Frames);
            m_Target    = Target;
            m_Delta     = (m_Target - m_Current) / m_Frames;
        }

        // Accept the new target and reset the number of blend frames
        else if( AbsDelta < m_DeltaToPop )
        {
            m_Frames    = m_FramesToBlend;
            m_Target    = Target;
            m_Delta     = (m_Target - m_Current) / m_Frames;
        }

        // Just pop to the new location and reset the number of blend frames
        else
        {
            m_Frames    = m_FramesToBlend;
            m_Target    = Target;
            m_Current   = Target;
            m_Delta     = 0.0f;
        }
    }

    f32 BlendLogic( void )
    {
        if( m_Frames == 0 )
            return m_Current;

        m_Frames--;
        m_Current += m_Delta;
        return m_Current;
    }
};

//==============================================================================
//  TYPES
//==============================================================================

class net_player : public netobj
{

//------------------------------------------------------------------------------
//  Private types
//------------------------------------------------------------------------------

protected:

    struct locale
    {
        vector3     Pos;
        radian      Yaw;
        radian      Pitch;
    };

    enum mode
    {
        CONTROL_LOCAL       = 0x01,
        CONTROL_REMOTE      = 0x02,
        ON_SERVER           = 0x04,
        ON_CLIENT           = 0x08,

        LOCAL_ON_SERVER     = CONTROL_LOCAL  | ON_SERVER,
        LOCAL_ON_CLIENT     = CONTROL_LOCAL  | ON_CLIENT,
        GHOST_ON_SERVER     = CONTROL_REMOTE | ON_SERVER,
        GHOST_ON_CLIENT     = CONTROL_REMOTE | ON_CLIENT,
    };

    enum dirty_bits
    {
        POSITION_BIT        = 0x00000001,
        ORIENTATION_BIT     = 0x00000002,
        WEAPON_BIT          = 0x00000004,
        HEALTH_BIT          = 0x00000008,
        ALIVE_BIT           = 0x00000010,
        CROUCH_BIT          = 0x00000100,
        JUMP_BIT            = 0x00000200,
        RELOAD_BIT          = 0x00000400,
        FIRE_BIT            = 0x00000800,
        TOSS_BIT            = 0x00001000,
        PAIN_BIT            = 0x00002000,
        RESPAWN_BIT         = 0x00004000,
        MELEE_BIT           = 0x00008000,
        AMMO_BIT            = 0x00010000,
//      ACTIVATE_BIT        = 0x80000000,
//      DEACTIVATE_BIT      = 0x40000000,
//      DELTA_DATA_BIT      = 0x20000000,
    };

    // this structure is to track not-so-vital stats for the local player
    //  there are several stat-tracking structures in the code, but none
    //  work completely, and since we're 1 day from deadline, I don't have
    //  time to figure out how to make other people code work.  Let the
    //  hacking begin. Kills will be read from gamemgr.player_score.
    //  This struct is NOT transmitted over the network. - AY
    struct local_player_stats
    {
        s32 hitsTaken;
        s32 shotsFired;
        s32 hits;
        s32 headHits;
        s32 torsoHits;
        s32 groinHits;
        s32 legHits[2]; // left, right
        s32 armHits[2]; // left, right
    };

    struct buffer
    {
        u32         ReadBits;           // collection of dirty_bits
        u32         WriteBits;          // collection of dirty_bits
        vector3     Position;           // POSITION_BIT
        radian      Pitch;              // ORIENTATION_BIT
        radian      Yaw;                // ORIENTATION_BIT
        s32         Health;             // HEALTH_BIT
        s32         LifeSeq;            // HEALTH_BIT ?
        s32         PainDirBits;        // HEALTH_BIT
        xbool       Alive;              // ALIVE_BIT
        s32         Weapon;             // WEAPON_BIT
        s32         DeltaData[6];       // DELTA_DATA_BIT
        xbool       Crouch;             // CROUCH_BIT
        s32         Jump;               // JUMP_BIT
        s32         Melee;              // MELEE_BIT
        s32         Reload;             // RELOAD_BIT
        s32         Fire;               // FIRE_BIT
        vector3     FirePos;            // FIRE_BIT
        vector3     FireVel;            // FIRE_BIT
        f32         FireStrength;       // FIRE_BIT
        s32         FireWeaponIndex;    // FIRE_BIT
        s32         AmmoClip[4];        // AMMO_BIT
        s32         AmmoReserve[4];     // AMMO_BIT
        s32         Toss;               // TOSS_BIT
        vector3     TossPos;            // TOSS_BIT
        vector3     TossVel;            // TOSS_BIT
        f32         TossStrength;       // TOSS_BIT
        s32         Respawn;            // RESPAWN_BIT
        net_pain    Pain[32];           // PAIN_BIT (read)
        s32         ClientPain;         // PAIN_BIT (write)
        s32         ClientPainType;     // PAIN_BIT (write)
        s32         ClientPainDirBits;  // PAIN_BIT (write)
        s32         ClientPainOrigin;   // PAIN_BIT (write)
    };

//------------------------------------------------------------------------------
//  Public functions
//------------------------------------------------------------------------------

public:

virtual void        Activate        ( void );
virtual void        Deactivate      ( void );
                    
virtual void        CreateGameObj   ( void );
virtual void        DestroyGameObj  ( void );
                    
virtual void        OnAcceptUpdate  ( const bitstream& BitStream );
virtual void        OnProvideUpdate (       bitstream& BitStream, 
                                            u32&       DirtyBits, 
                                            s32        Client,
                                      const delta*     pDelta );

virtual void        GetClearDelta   ( delta* pDeltaData );

virtual void        Logic           ( void );

virtual void        DebugRender     ( void ) const;

        void        ExchangeValues  ( buffer& Buffer );

        void        ApplyMove       ( const move& Move );
        void        ApplyPain       ( s32 Pain, s32 Type, s32 DirBits, s32 Origin );

        void        BlendUpdatePos          ( void );
        void        BlendUpdateOrientation  ( void );
        void        BlendLogicPos           ( void );
        void        BlendLogicOrientation   ( void );

        xbool       IsLocal         ( void );
        xbool       IsGhost         ( void ); 
        s32         GetHealth       ( void ) { return m_Health; }

        local_player_stats &GetLocalStats()     { return m_LocalStats; }

        void        SetMute         ( s32 MouthIndex, xbool Mute );
        xbool       GetMute         ( s32 MouthIndex );

        void        SetTeam         ( s32 Team );
        void        ForceDeath      ( void );
        void        DesireTeamChange( void );

        void        SetSkins        ( s32 Primary, s32 Alternate );    

//------------------------------------------------------------------------------
//  Private functions
//------------------------------------------------------------------------------

protected:

//------------------------------------------------------------------------------
//  Private variables
//------------------------------------------------------------------------------

protected:

        mode        m_Mode;    
                    
        locale      m_Actual;
        locale      m_Render;
//      locale      m_BlendDelta;   // Delta locale per frame for blending.
//      s32         m_BlendFrames;

        blender     m_BlendPosX;
        blender     m_BlendPosY;
        blender     m_BlendPosZ;
        blender     m_BlendPitch;
        blender     m_BlendYaw;

        local_player_stats m_LocalStats;

        xbool       m_FreshActivate;
        xbool       m_ChangingTeam; // TRUE means desire team change.
        s32         m_DesiredTeam;    

        s32         m_Weapon;
        s32         m_NGrenades;
        s32         m_Health;
        s32         m_LifeSeq;
        xbool       m_Alive;
        xbool       m_Crouch;
        s32         m_Jump;  
        s32         m_Melee;
        s32         m_Reload;
        s32         m_Fire;
        vector3     m_FirePos;
        vector3     m_FireVel;
        f32         m_FireStrength;
        s32         m_FireWeaponIndex;
        s32         m_AmmoClip[4];
        s32         m_AmmoReserve[4];
        s32         m_Toss;
        vector3     m_TossPos;
        vector3     m_TossVel;
        f32         m_TossStrength;
        s32         m_Respawn;

        s32         m_Pain;
        s32         m_PainType;
        s32         m_PainDirBits;
        s32         m_PainOrigin;

        s32         m_DeltaData[6];
                    // 0 : 1st weapon
                    // 1 : 2nd weapon
                    // 2 : 3rd weapon
                    // 3 : 4th weapon
                    // 4 : grenades
                    // 5 : health

        move_mgr*   m_pMoveMgr;

        u32         m_MuteBits;

        s32         m_PrimarySkin;
        s32         m_AlternateSkin;
};

//==============================================================================
#endif // NETPLAYER_HPP
//==============================================================================
