//=========================================================================
//
//  God.hpp
//
//=========================================================================

#ifndef __GOD_HPP__
#define __GOD_HPP__ 

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\Obj_mgr.hpp"
#include "AStar.hpp"
#include "Characters\AlertPackage.hpp"
#include "Navigation\ng_connection2.hpp"
#include "TriggerEX\Actions\action_music_intensity.hpp"


const s32 k_NumTargettingData = 32;
const s32 k_MaxMeleeingPlayer = 2;

//=========================================================================
// CLASSES
//=========================================================================

// God class - takes care of telling all characters what to do
class god : public object
{
public:

    //=========================================================================
// Real time type information
//=========================================================================
public:
    CREATE_RTTI( god, object, object )

    virtual const object_desc&  GetTypeDesc     ( void ) const;
    static  const object_desc&  GetObjectType   ( void );

//=========================================================================
// Defines
//=========================================================================
public:

//=========================================================================
// Structs
//=========================================================================

    struct TargettingData
    {
        TargettingData(guid targetter, guid targetGuid, f32 distanceSqr );
        TargettingData() { Clear(); }        
        void Clear();

        guid m_Targetter;
        guid m_TargetGuid;
        f32  m_DistanceSqr;
    };

//=========================================================================
// Class functions
//=========================================================================
public:
                    god() ;
    virtual         ~god() ;
    
//=========================================================================
// Inherited virtual functions from base class
//=========================================================================

    virtual void    OnActivate              ( xbool Flag );            
    virtual void    OnKill                  ( void );   
    virtual void    OnAdvanceLogic          ( f32 DeltaTime );

#ifndef X_RETAIL
    virtual void    OnDebugRender           ( void ) ;
#endif // X_RETAIL

    virtual s32     GetMaterial             ( void ) const ;
    virtual bbox    GetLocalBBox            ( void ) const;

            xbool   RequestPathWithEdges    ( object*               pRequestObject, 
                                              const vector3&        vDestination, 
                                              path_find_struct&     rPathStruct, 
                                              s32                   nMaxEdgeListSize,
                                              const pathing_hints*  pPathingHints = NULL,
                                              xbool                 bDestInSameGrid = TRUE );

            xbool   RequestRetreatPath      ( object*               pRequestObject, 
                                              const vector3&        vRetreatFrom,
                                              f32                   DesiredMinDistance,
                                              path_find_struct&     rPathStruct, 
                                              s32                   nMaxEdgeListSize,
                                              const pathing_hints*  pPathingHints = NULL );
            
            s32     PlayAlertSound          ( const char* pObjectName, const char* pAction, s32 State, guid ObjGuid,
                                              s16 ZoneID, vector3& Pos );


            xbool   IsActiveZone            ( u16 ZoneID );
            f32     GetMinDistanceToPlayersThroughZones( const vector3& position, u16 ZoneID  );
            xbool   GetCanMeleePlayer       ( guid requestNPC );
            
            void    AddTargettingData       ( TargettingData newTargetData );           // adds data to the list of targetting data
            s32     GetNumTargettingCloser  ( TargettingData newTargetData );           // this tells us the number targeting our target unless we are the closest, then it returns 0.


//=========================================================================
// Data
//=========================================================================
    astar_path_finder   m_AStarPathFinder;          // pathfinder
protected:    
    TargettingData      m_CurrentTargettingData[k_NumTargettingData];
    TargettingData      m_LastTickTargettingData[k_NumTargettingData];

    s32                 m_ActiveThinkID ;           // Current ID of character that can think
    guid                m_MeleeingPlayerGuids[k_MaxMeleeingPlayer];       // guid of the NPC currently meleeing the player.   
    xtick               m_SoundTimer;
    xtick               m_GrenadeTimer;
    f32                 m_TimeDeltaToTalk;      
    s32                 m_LastTalkState;

//=========================================================================
// Editor
//=========================================================================
protected:
    virtual void    OnEnumProp      ( prop_enum&    List ) ;
    virtual xbool   OnProperty      ( prop_query&   I    ) ;
} ;


//=========================================================================

#endif//__GOD_HPP__
