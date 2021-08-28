//=========================================================================
//
//  ANIMPLAYER.HPP
//
//=========================================================================
#ifndef ANIM_PLAYER_HPP
#define ANIM_PLAYER_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_files.hpp"
#include "x_math.hpp"
#include "x_bitstream.hpp"
#include "animdata.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "AnimTrack.hpp"
#include "SMemMatrixCache.hpp"
#include "BasePlayer.hpp"

//=========================================================================
// CLASS SIMPLE_ANIM_PLAYER
//=========================================================================

class simple_anim_player : public base_player
{

//-------------------------------------------------------------------------
public:

                                simple_anim_player     ( void );
    virtual                     ~simple_anim_player     ( void );

    void                        SetAnimGroup        ( const anim_group::handle& hGroup );    
    void                        SetSimpleAnim       ( s32 iAnim );
    void                        SetSimpleAnim       ( const char* pAnimName );
    void                        SetAnim             ( s32 iAnim, xbool IsLooping=TRUE );
    void                        SetAnim             ( const char* pAnimName, xbool IsLooping=TRUE );
    void                        SetStopAtEnd        ( xbool StopAtEnd );
    void                        SetLooping          ( xbool Looping );
    xbool                       IsLooping           ( void ) const { return m_IsLooping; }
    void                        SetLoopDelay        ( f32   LoopDelay );
                                
    void                        Advance             ( f32       nSeconds );
    const vector3&              GetRootNodePos      ( void ) const;
                                
    const matrix4&              GetL2W              ( void ) const { return m_L2W; }
    void                        SetL2W              ( const matrix4& L2W );

    void                        SetTrackController  ( s32 iTrackController, track_controller* pTrackController );
    track_controller*           GetTrackController  ( s32 iTrackController );

private:                        
    f32                         ComputeInterpFrame  ( void ) ;
    void                        GetInterpKeys       ( anim_key* pKey ) ;
    
    xbool                       IsCachedL2WValid    ( xbool bApplyTheBindPose );
                                                      

    void                        DirtyCachedL2W      ( void ) ;
    const matrix4*              UpdateCachedL2W     ( xbool             bApplyTheBindPose );

public:
    // Returns the cached L2W matrix for that bone
    const matrix4*              GetBoneL2Ws         ( xbool bApplyTheBindPose = TRUE ) ;

    const matrix4*              GetBoneL2W          ( s32 iBone, xbool bApplyTheBindPose = TRUE );
                                
    const vector3               GetBoneBindPosition ( s32 iBone ) ;             // Bone bind position

    void                        SetFrame            ( f32 Frame );
    void                        SetFrame            ( s32 iFrame );
    void                        SetFrameParametric  ( f32 Frame );
    void                        SetTime             ( f32 Time );
    xbool                       IsAtEnd             ( void ) const;
                                
    f32                         GetFrame            ( void ) const;
    f32                         GetPrevFrame        ( void ) const;
    s32                         GetCycle            ( void ) const;
    s32                         GetPrevCycle        ( void ) const;
    f32                         GetRate             ( void ) const;
    void                        SetRate             ( f32 Rate );
    f32                         GetFrameParametric  ( void );
    s32                         GetNFrames          ( void ) const;
    s32                         GetAnimIndex        ( void ) const;
    s32                         GetBoneIndex        ( const char* pBoneName, xbool bFindAnywhere = FALSE );
    s32                         GetNBones           ( void ) ;
    s32                         GetNAnims           ( void ) ;
    s32                         GetAnimIndex        ( const char* pAnimName ) ;
    f32                         GetTimeLeft         ( void );
    
    virtual s32                 GetNEvents          ( void );
    virtual xbool               IsEventActive       ( s32 iEvent );
    virtual const anim_event&   GetEvent            ( s32 iEvent );
                                
    s32                         GetCollisionBone    ( void );
                                
    f32                         GetStartSpeed       ( f32 Time = 1.0f );
    f32                         GetEndSpeed         ( f32 Time = 1.0f );
                                
    xbool                       AnimDone            ( void ) const;
                                
    const anim_info&            GetAnimInfo         ( void );   // Warning - leaves lock
    
    s32                         GetEventIndex       ( const char* pEventName );
    virtual vector3             GetEventPosition    ( s32 iEvent );
    virtual radian3             GetEventRotation    ( s32 iEvent );
    virtual vector3             GetEventPosition    ( const anim_event& Event );
    virtual radian3             GetEventRotation    ( const anim_event& Event );
    virtual const char*         GetAnimName         ( void ) { return GetAnimInfo().GetName(); }

    const anim_group*           GetAnimGroup        ( void );
    const anim_group::handle    GetAnimGroupHandle  ( void ) { return m_hGroup; }

    // Returns the combined local space bbox of all the current playing animations
    bbox                        ComputeBBox         ( void ) const;

//-------------------------------------------------------------------------
private:

    anim_group::handle  m_hGroup;                       // Handle for group anim
    s32                 m_iAnim;                        // Index of anim we are playing
    s32                 m_nFrames;                      // # of frames in animation
    f32                 m_Frame;                        // Frame number
    s32                 m_Cycle;                        // Cycle number
    f32                 m_PrevFrame;                    // Previous Frame number
    s32                 m_PrevCycle;                    // Previous Cycle number
    f32                 m_Rate;                         // Playback rate (defaults to 1)
    matrix4             m_L2W PS2_ALIGNMENT(16) ;       // Local -> world matrix
    xbool               m_IsLooping;                    // Loop flag
    xbool               m_StopAtEnd;                    // Stop anim after 1 cycle flag
    xbool               m_bAtEnd;                       // TRUE if anim has played 1 cycle
    vector3             m_RootNodePos;                  // Position of root node
    f32                 m_LoopDelay;                    // Loop delay when reaching end of anim
    
    smem_matrix_cache   m_CachedL2W ;                   // Cached matrices
    const anim_group*   m_pCachedGroup ;                // Anim group currently cached
    s32                 m_iCachedAnim ;                 // Index of animation that cached
    f32                 m_CachedFrame ;                 // Current frame that is cached
    xbool               m_bCachedApplyBind ;            // Apply bind cached value
    track_controller*   m_pTrackController[ 4 ];        // Pointer to last used track controllers        

} PS2_ALIGNMENT(16) ;

//=========================================================================
// INLINES
//=========================================================================

inline
const anim_group* simple_anim_player::GetAnimGroup( void )
{
    return m_hGroup.GetPointer() ;
}

//=========================================================================

inline
const anim_info& simple_anim_player::GetAnimInfo( void ) 
{
    // Lookup group
    const anim_group* pGroup = GetAnimGroup();
    ASSERT( pGroup );

    // Valid anim?
    ASSERT( m_iAnim >= 0 );
    ASSERT( m_iAnim < pGroup->GetNAnims() );

    // Lookup anim
    const anim_info& AnimInfo = pGroup->GetAnimInfo( m_iAnim );
    return AnimInfo;
}

//=========================================================================

inline
const vector3& simple_anim_player::GetRootNodePos  ( void ) const
{
    return m_RootNodePos;
}

//=========================================================================

inline
void simple_anim_player::SetFrameParametric( f32 Frame )
{
    // Range check
    if (Frame < 0)
        Frame = 0 ;
    else
        if (Frame > 1)
            Frame = 1 ;

    // NOTE: The -2 is because the last 2 frames are the same - they are used
    //       to calculate a correct delta for the positions eg. looping run anim
    //       DO NOT CHANGE THIS. If you use -1, then an incorrect delta gets calculated
    //       upon the loop of the animation, resulting in a position pop!

    // Set
    m_Frame = Frame * ((f32)m_nFrames-2) ;
}

//=========================================================================

inline
f32 simple_anim_player::GetFrameParametric( void )
{
    // NOTE: The -2 is because the last 2 frames are the same - they are used
    //       to calculate a correct delta for the positions eg. looping run anim
    //       DO NOT CHANGE THIS. If you use -1, then an incorrect delta gets calculated
    //       upon the loop of the animation, resulting in a position pop!

    f32 Frame = m_Frame / (m_nFrames-2) ;

    // Range check
    if (Frame < 0)
        Frame = 0 ;
    else
        if (Frame > 1)
            Frame = 1 ;

    return Frame ;
}

//=========================================================================

inline
xbool simple_anim_player::IsAtEnd( void ) const
{
    return ( (m_Cycle>0) || (m_Frame >= (m_nFrames-2)) );
}

//=========================================================================

inline
void simple_anim_player::SetFrame( f32 Frame )
{
    ASSERT( Frame >= 0 );
    ASSERT( Frame < m_nFrames );
    
    m_Frame = Frame;
    m_Cycle = 0;
    m_bAtEnd = FALSE;
}

//=========================================================================

inline
void simple_anim_player::SetFrame( s32 iFrame )
{
    ASSERT( iFrame >= 0 );
    ASSERT( iFrame < m_nFrames );
    
    m_Frame = (f32)iFrame;
    m_Cycle = 0;
    m_bAtEnd = FALSE;
}

//=========================================================================

inline
void simple_anim_player::SetTime( f32 Time )
{
    // No anim?
    if( m_iAnim == -1 )
        return;
        
    // Lookup anim info
    const anim_info& AnimInfo = GetAnimInfo();
    
    // Compute frame from time
    f32 Frame = Time * AnimInfo.GetFPS();
    
    // Peg at end (-2 is because the last 2 frames are the same from the exporter)
    f32 EndFrame = (f32)( AnimInfo.GetNFrames() - 2 );
    if( Frame > EndFrame )
        Frame = EndFrame;
    
    // Peg at beginning
    if( Frame < 0.0f )
        Frame = 0.0f;

    // Record so events fire off correctly
    m_PrevFrame = m_Frame;

    // Set it
    SetFrame( Frame );        
}

//=========================================================================

inline
f32 simple_anim_player::GetFrame( void ) const
{
    return m_Frame;
}

//=========================================================================

inline
f32 simple_anim_player::GetPrevFrame( void ) const
{
    return m_PrevFrame;
}

//=========================================================================

inline
s32 simple_anim_player::GetCycle( void ) const
{
    return m_Cycle;
}

//=========================================================================

inline
s32 simple_anim_player::GetPrevCycle( void ) const
{
    return m_PrevCycle;
}

//=========================================================================

inline
f32 simple_anim_player::GetRate( void ) const
{
    return m_Rate;
}

//=========================================================================

inline
void simple_anim_player::SetRate( f32 Rate )
{
    m_Rate = Rate;
}

//=========================================================================

inline
s32 simple_anim_player::GetNFrames( void ) const
{
    return m_nFrames;
}

//=========================================================================

inline
s32 simple_anim_player::GetAnimIndex( void ) const
{
    return m_iAnim;
}

//=========================================================================

inline
s32 simple_anim_player::GetNEvents( void )
{
    return GetAnimInfo().GetNEvents();
}

//=========================================================================

inline
xbool simple_anim_player::IsEventActive( s32 iEvent )
{
    return GetAnimInfo().IsEventActive( iEvent, m_Frame, m_PrevFrame);
}

//=========================================================================

inline
const anim_event& simple_anim_player::GetEvent( s32 iEvent )
{
    return GetAnimInfo().GetEvent( iEvent );
}

//=========================================================================



//=========================================================================
#endif // END ANIM_PLAYER_HPP
//=========================================================================
