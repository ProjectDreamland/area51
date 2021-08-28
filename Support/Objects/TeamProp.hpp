#ifndef TEAMPROP_HPP
#define TEAMPROP_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Objects\Circuit.hpp"

#include "Obj_mgr\obj_mgr.hpp"
#include "Objects\Render\RigidInst.hpp"
#include "Objects\PlaySurface.hpp"

//=========================================================================
// CLASS
//=========================================================================



class team_prop : public play_surface
{
public:
    CREATE_RTTI( team_prop, play_surface, object )

    //=========================================================================

                                team_prop       ( void );
                               ~team_prop       ( void );

                    void        SetL2W          ( const matrix4& L2W );


    virtual         s32         GetMaterial     ( void ) const { return MAT_TYPE_NULL; }

    virtual			void	    OnEnumProp		( prop_enum& rList );           
    virtual			xbool	    OnProperty		( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         s32         OnValidateProperties( xstring& ErrorMsg );
#endif

    virtual const object_desc&  GetTypeDesc     ( void ) const;                 
    static  const object_desc&  GetObjectType   ( void );       

                    void        SetGeom         ( char* pString );

                    void        SetCircuit      ( s32 Circuit ) { m_Circuit.SetCircuit( Circuit ); }

protected:   
    //=========================================================================

    virtual void                OnRender		    ( void );                       
    virtual void                OnAdvanceLogic	    ( f32     DelaTime );           
    virtual void                OnInit              ( void );                       

    // These store the possible alignments.
    enum Alignments
    {
        FRIENDLY_NONE  = 0x00000000,
        FRIENDLY_ALPHA = 0x00000001,
        FRIENDLY_OMEGA = 0x00000002,
        FRIENDLY_ALL   = 0xFFFFFFFF,
    };
    
    // This is just to make life easier in keeping track of what to 
    // render as for the player, and in loading the properties.
    enum RenderAs
    {
        FRIEND_TO_NONE        = 0,
        FRIEND_TO_ENEMY_ALPHA = 1,
        FRIEND_TO_TEAM_ALPHA  = 2,
        FRIEND_TO_ALL         = 3,
        FRIEND_TO_ENEMY_OMEGA = 4,
        FRIEND_TO_TEAM_OMEGA  = 5,
        
        NUM_FRIEND_TO_OPTIONS = 6,
    };

    circuit                         m_Circuit;

protected:
    u32         m_OldState;
    u32         m_NewState;
    f32         m_TransitionValue;
};

//=========================================================================
// END
//=========================================================================
#endif
