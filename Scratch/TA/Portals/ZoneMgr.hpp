#ifndef ZONE_MGR_HPP
#define ZONE_MGR_HPP

//=========================================================================
// INCLUDES
//=========================================================================
#include "Entropy.hpp"

//=========================================================================
// TYPES
//=========================================================================

class zone_mgr
{
public:

    //---------------------------------------------------------------------
    enum portal_flags
    {
        PFLAGS_DISABLE = (1<<0),
    };

    //---------------------------------------------------------------------
    typedef u8 zone_id;

    //---------------------------------------------------------------------
    struct zone
    {
        bbox        BBox;               // BBox containing the hold zone
        s32         nPortals;           // How many portals does this zone has
        s32         iPortal2Portal;     // start index to the array of indices to portals
    };
    
    //---------------------------------------------------------------------
    struct portal
    {
        bbox        BBox;               // BBox 
        plane       Plane;              // Plane of the actual portal.
        zone_id     iZone[2];           // The two zones that this portal connects
        vector3     Edges[4];           // A portal is always a square.
        u32         Flags;              // Flags about the portal.
        xbool       bStack;             // Flags that represents whether this portal is in the recursive stack
    };

    //---------------------------------------------------------------------
    struct frustum
    {
        s32         nPlanes;            // Number of planes incluring far and near
        plane       Plane[16];          // array of planes

        s32         nEdges;             // Number of edges ones all has been cliped. This is more for debuging
        vector3     Edges[16];          // Array of the edges
        zone_id     iZone;              // Which zone does this frustum belongs 
    };

public:

                zone_mgr        ( void );
               ~zone_mgr        ( void );

    void        Clean           ( void );
    void        AddStart        ( s32 nZones, s32 nPortals );
    void        AddZone         ( bbox& BBox, s32 ZoneID );
    void        AddPortal       ( bbox& BBox, vector3* pEdges, s32 ZoneA, s32 ZoneB );
    void        AddEnd          ( void );

    void        PortalWalk      ( const view& View, s32 iZone );
    void        Render          ( void );

protected:

    enum
    {
        MAX_FRUSTUMS = 32
    };


protected:

    xbool       ComputeFrustum  ( frustum&        NewFrustum, 
                                  const frustum&  CurrentFrustum, 
                                  const vector3&  EyePosition, 
                                  const portal&   Portal ) const;    
    void        PortalWalk      ( frustum* pFrustum, const frustum& ParentFrustum );

protected:

    // Data-Base variables
    s32         m_nPortals;
    portal*     m_pPortal;

    s32         m_nZones;
    zone*       m_pZone;

    s32         m_nZone2Portal;
    s32*        m_pZone2Portal;

    // Add mode variables
    xbool       m_bAddMode;
    s32         m_MaxPortals;

    // Portal Walk variables
    s32         m_nFrustums;
    frustum     m_Frustum[ MAX_FRUSTUMS ];
    vector3     m_EyePosition;

    //
    // This is an array of bits nZones by nZones big. It tells whether a 
    // zone is visible from another zone.
    //
    u8*         m_pVisivilityBits;
};

//=========================================================================

extern zone_mgr g_ZoneMgr;

//=========================================================================
// END
//=========================================================================
#endif