#ifndef CHECK_POINT_MGR_HPP
#define CHECK_POINT_MGR_HPP

#include "Inventory\Inventory2.hpp"
#include "Objects\Player.hpp"

#define MAX_LEVELS      (18)
#define MAX_CHECKPOINTS (6)

struct ammo_counts
{
    s32     Amount;
    s32     CurrentClip;
};

struct check_point
{
    xbool       bIsValid;
    s32         TableName;
    s32         TitleName;
    guid        RespawnGUID;
    guid        AdvanceGUID;
    s32         CurrWeapon;
    s32         PrevWeapon;
    s32         NextWeapon;
    xbool       MutantMelee;
    xbool       MutantPrimary;
    xbool       MutantSecondary;
    f32         Mutagen;
    f32         Health;
    f32         MaxHealth;
    f32         Inventory[INVEN_COUNT];
    ammo_counts Ammo[INVEN_NUM_WEAPONS*2];

    void Init( void ) 
    {
        bIsValid        = FALSE;
        TableName       = -1;
        TitleName       = -1;
        RespawnGUID     = 0;
        AdvanceGUID     = 0;
        CurrWeapon      = 0;
        PrevWeapon      = 0;
        NextWeapon      = 0;
        MutantMelee     = FALSE;
        MutantPrimary   = FALSE;
        MutantSecondary = FALSE;
        Mutagen         = 0.0f;
        Health          = 0.0f;
        MaxHealth       = 0.0f;
        for( s32 i=0 ; i<INVEN_COUNT ; i++ )
            Inventory[i] = 0.0f;
        for( s32 i=0 ; i<INVEN_NUM_WEAPONS*2 ; i++ )
        {
            Ammo[i].Amount      = 0;
            Ammo[i].CurrentClip = 0;
        }
    };
};

struct level_check_points
{
    s32             MapID;                  // Unique Map ID.
    s32             nValidCheckPoints;      // Number of valid check points.
    s32             iCurrentCheckPoint;     // Current check point.
    check_point     CheckPoints[ MAX_CHECKPOINTS ];

    void Init( s32 ID )
    {
        MapID              = ID;
        nValidCheckPoints  = 0;
        iCurrentCheckPoint = -1;
        for( s32 i=0 ; i<MAX_CHECKPOINTS ; i++ )
            CheckPoints[i].Init();
    }
};

class check_point_mgr 
{
public:

                        check_point_mgr         ( void )                {}
                       ~check_point_mgr         ( void )                {}

    void                Init                    ( s32           MapID ) { m_Level.Init( MapID ); }
    void                Kill                    ( void )                {}
    s32                 GetCheckPointIndex      ( void );
    void                SetCheckPointIndex      ( s32           CheckPoint );
    xbool               SetCheckPoint           ( guid          RespawnwGUID,
                                                  guid          DebugAdvanceGUID,
                                                  s32           TableName,
                                                  s32           TitleName );
    s32                 GetMapID                ( void );
    s32                 Read                    ( bitstream&    in );
    s32                 Write                   ( bitstream&    out );
    xbool               Restore                 ( xbool         bIsDebugAdvance );
    void                Reinit                  ( s32           MapID );


    level_check_points  m_Level;
};

extern check_point_mgr g_CheckPointMgr;

#endif // CHECK_POINT_MGR_HPP
