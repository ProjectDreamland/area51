//=========================================================================
//
//  OCCLUDERMGR.HPP
//
//=========================================================================
//
// This class accepts a set of convex ngons as occluders.  Each occluder
// must be convex and have no more than MAX_OCCLUDER_POINTS points.  After
// making multiple calls to AddOccluder at init time the set is ready.
// At the beginning of each render loop you need to call SetView
// with the current camera.  The class will compute the set of 
// planes that contain the region blocked by the occluders.  You can then
// call the Is???Occluded functions to evaluate if something is wholly
// contained within the blocked region of any of the occluders.
//
// If you do not call SetView the occluders will use the last camera
// available.  This allows you to render and cull from a frozen view
// position and direction.
//
// It is in your best interest to make the largest occluders you can since
// most of the objects need to be culled without occluder fusion.  
//
//=========================================================================
#ifndef OCCLUDERMGR_HPP
#define OCCLUDERMGR_HPP

#include "x_math.hpp"
#include "x_time.hpp"
#include "e_View.hpp"
 
//=========================================================================

class occluder_mgr
{
public:
            occluder_mgr( void );
            ~occluder_mgr( void );
    
    void    Init                    ( void );
    void    Kill                    ( void );
    
    void    GatherOccluders  ( void );

    void    RenderAllOccluders		( void );
    void    RenderUsableOccluders	( void );
    void    RenderFrustums			( void );

    void    UseOccluders            ( xbool OnOff );
    void    SetView                 ( const view& View );
    xbool   IsBBoxOccluded          ( const bbox& BBox );
    xbool   IsPointOccluded         ( const vector3& Point );

    void    DirtyOccluders          ( void );
    
//-------------------------------------------------------------------------
private:

    void    Clear					( void );
    void    AddOccluder				( const vector3* pPoint, s32 nPoints );

//-------------------------------------------------------------------------
private:

    #define MAX_OCCLUDERS           32
    #define MAX_USABLE_OCCLUDERS    8
    #define MAX_OCCLUDER_POINTS     4
    #define MAX_OCCLUDER_PLANES     (MAX_OCCLUDER_POINTS+1)

    struct occluder
    {
        plane   Plane;
        s32     nPoints;
        vector3 Point[ MAX_OCCLUDER_POINTS ];
        plane   ViewPlane[ MAX_OCCLUDER_PLANES ];
        s32     BBoxMinIndex[ MAX_OCCLUDER_PLANES*3 ];
        s32     BBoxMaxIndex[ MAX_OCCLUDER_PLANES*3 ];
        vector3 Center;
        f32     Area;
        f32     Score;
		bbox	BBox;
        xbool   bPrepared;
        xbool   bActive;
    };

    occluder    m_Occluder[MAX_OCCLUDERS];
    s32         m_nOccluders;
    xbool       m_UseOccluders;

    vector3     m_EyePos;
    plane       m_EyePlane;
    vector3     m_EyeX;
    vector3     m_EyeY;
    vector3     m_EyeZ;

    s32         m_nUsableOccluders;
    s32         m_UsableOccluderIndex[MAX_OCCLUDERS];
    s32         m_HintDelaySeq;

    xbool       m_bDirtyOccluders;

    struct stats
    {
        s32     nBBoxOccludedTrue;
        s32     nBBoxOccludedFalse;
        f32     BBoxOccludeTime;
        f32     SetViewTime;
    };
public:
    stats       m_Stats;


//-------------------------------------------------------------------------
private:
    xbool   IsBBoxCompletelyInsidePlanes( const occluder& O, const bbox& BBox );
};

//=========================================================================

extern occluder_mgr g_OccluderMgr;

//=========================================================================
#endif
//=========================================================================
