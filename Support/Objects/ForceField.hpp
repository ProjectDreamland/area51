//=========================================================================
//
// ForceField.hpp
//
//=========================================================================

#ifndef FORCEFIELD_HPP
#define FORCEFIELD_HPP

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

class force_field : public object
{
public:
    CREATE_RTTI( force_field, object, object )

        //=========================================================================

    enum field_states
    {
        STATE_OPEN,
        STATE_CLOSING,
        STATE_CLOSED,
    };

                                    force_field         ( void );
                                   ~force_field         ( void );

    virtual         s32             GetMaterial         ( void ) const { return MAT_TYPE_ENERGY_FIELD; }

    virtual			void	        OnEnumProp		    ( prop_enum& rList );           
    virtual			xbool	        OnProperty		    ( prop_query& rPropQuery );

                    void            OnColCheck          ( void );
                    void            OnPolyCacheGather   ( void );
                    void            CreateVertices      ( void );

                    circuit&        GetCircuit          ( void ) { return m_Circuit; }
                    void            Open                ( void ) { m_bOn = FALSE; }
                    
    virtual const object_desc&      GetTypeDesc         ( void ) const;                 
    static  const object_desc&      GetObjectType       ( void );      

                    field_states    GetState            ( void );

                    bbox            GetLocalBBox        ( void ) const;
                    void            OnProjectileImpact  ( vector3& Point );

                    void                OnPain          ( const pain& Pain );

protected:   
    //=========================================================================

    virtual void                OnRender		    ( void ) {};
    virtual void                OnRenderTransparent ( void );

    virtual void                OnAdvanceLogic	    ( f32     DelaTime );           
    virtual void                OnInit              ( void );                       

    // These store the possible alignments.
    enum alignments
    {
        FRIENDLY_NONE  = 0x00000000,
        FRIENDLY_ALPHA = 0x00000001,
        FRIENDLY_OMEGA = 0x00000002,
        FRIENDLY_ALL   = 0xFFFFFFFF,
    };

    // This is just to make life easier in keeping track of what to 
    // render as for the player, and in loading the properties.
    enum render_as
    {
        FRIEND_TO_NONE  = 0,
        FRIEND_TO_ENEMY = 1,
        FRIEND_TO_TEAM  = 2,
        FRIEND_TO_ALL   = 3
    };

    circuit      m_Circuit;
    xbool        m_bInitialized;

protected:
    u32          m_OldState;
    u32          m_NewState;
    f32          m_TransitionValue;

    f32          m_Width;
    f32          m_Height;

    vector2*     m_pVertices;
    vector2*     m_pSeekingVertices;

    f32          m_ScrollFactor;

    s32          m_NumRows;
    s32          m_NumCols;

    xbool        m_bOn;
    f32          m_PercentageOn;
    field_states m_LastState;    

    f32          m_Brightness;
    f32          m_Phase;
    f32          m_Hit;

    xbool        m_bActiveWhenFriendlyAll;

    static rhandle<xbitmap>     m_ForceTexture;
    static rhandle<xbitmap>     m_ForceCloudTexture;
};

//=========================================================================
// END
//=========================================================================
#endif
