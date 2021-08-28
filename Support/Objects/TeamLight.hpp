#ifndef TEAMLIGHT_HPP
#define TEAMLIGHT_HPP

//=========================================================================
// INCLUDES
//=========================================================================

#include "Circuit.hpp"

#include "Obj_mgr\obj_mgr.hpp"
#include "Objects\LightObject.hpp"
#include "Objects\Render\RigidInst.hpp"

//=========================================================================
// CLASS
//=========================================================================

class team_light : public object
{
public:
    CREATE_RTTI( team_light, object, object )

     //=========================================================================

    team_light                                  ( void );
   ~team_light                                  ( void );

    virtual s32                 GetMaterial     ( void ) const { return MAT_TYPE_NULL; }


    virtual			void	    OnEnumProp		( prop_enum& rList );           
    virtual			xbool	    OnProperty		( prop_query& rPropQuery );     

    virtual         bbox        GetLocalBBox    ( void ) const;                 

    virtual const object_desc&  GetTypeDesc     ( void ) const;                 
    static  const object_desc&  GetObjectType   ( void );                       

                    circuit&    GetCircuit      ( void ) { return m_Circuit; }
protected:   
    //=========================================================================

    virtual void                OnRender		( void );                                  
    virtual void                OnAdvanceLogic	( f32     DelaTime );           
    virtual void                OnInit          ( void );                       
                   

#ifndef X_RETAIL
    virtual void                OnDebugRender   ( void );                        
#endif // X_RETAIL

    struct state_vals
    {
        xcolor m_Color;
    };

protected:
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
        FRIEND_TO_ALL,
        FRIEND_TO_TEAM,
        FRIEND_TO_ENEMY,
        FRIEND_TO_NONE
    };

    bbox        m_RenderBBox; // The rendering bbox.

    f32         m_Radius;
    f32         m_Intensity;

    u32         m_OldState;
    u32         m_NewState;
    f32         m_TransitionValue;
    
    state_vals* m_States[ 4 ];

    state_vals  m_Friend;
    state_vals  m_Foe;
    state_vals  m_FriendAll;
    state_vals  m_FoeAll;

    circuit     m_Circuit;
};

//=========================================================================
// END
//=========================================================================
#endif
