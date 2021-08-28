//=========================================================================
//
//  VehicleObject.hpp
//
//=========================================================================

#ifndef VEHICLE_OBJECT_HPP
#define VEHICLE_OBJECT_HPP 

//=========================================================================
// INCLUDES
//=========================================================================

#include "Objects\object.hpp"
#include "Objects\Render\RigidInst.hpp"
#include "Animation\AnimData.hpp"
#include "Entropy.hpp"


//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================


//=========================================================================
// CLASSES
//=========================================================================

class vehicle_object : public object
{
// Real time type information
public:
    CREATE_RTTI( vehicle_object, object, object )

    virtual const object_desc&  GetTypeDesc     ( void ) const ;
    virtual const object_desc&  GetObjectType   ( void ) ;

//=========================================================================
// Friends of the family
//=========================================================================

//=========================================================================
public:

    struct input
    {
        f32     m_Steer;        // -1 .. 1
        f32     m_Accel;        //  0 .. 1
        f32     m_Brake;        //  0 .. 1
        f32     m_CameraYaw;    // -1 .. 1
        f32     m_CameraPitch;  // -1 .. 1
        f32     m_CameraZoom;   // -1 .. 1
        xbool   m_bToggleCamera;
        xbool   m_bReset;

        void    Clear(void) {x_memset(this,0,sizeof(input));}
    };

    struct camera
    {
        vector3 m_Focus;
        radian  m_FocusYaw;
        radian  m_Pitch;
        radian  m_Yaw;
        f32     m_Dist;

        void    Init        ( void);

        void    Update      ( const vector3&  Focus, 
                                    radian    FocusYaw,
                                    radian    DeltaPitch,
                                    radian    DeltaYaw,
                                    f32       DeltaZoom,
                                    f32       DeltaTime );

        void    ComputeView ( view& View );
    };

//=========================================================================
// Class functions
//=========================================================================
public:
                    vehicle_object() ;
    virtual         ~vehicle_object() ;


//=========================================================================
// Inherited virtual functions from base class
//=========================================================================
public:

    virtual void    OnInit          ( void ) ;
    virtual void    OnKill          ( void ) ;
    virtual void    OnAdvanceLogic  ( f32 DeltaTime ) ;      
    virtual void    OnRender        ( void ) ;      
    virtual       bbox          GetLocalBBox    ( void ) const;      
    virtual       s32           GetMaterial     ( void ) const;

    virtual geom*        GetGeomPtr        ( void ) { return m_RigidInst.GetSkinGeom(); }
    virtual render_inst* GetRenderInstPtr  ( void ) { return &m_RigidInst; }

//=========================================================================
// Functions
//=========================================================================
public:
    rigid_inst              m_RigidInst ;           // Render instance
    rhandle<anim_group>     m_hAnimGroup ;          // Animation group handle
    input                   m_Input;
    camera                  m_Camera;
    f32                     m_DeltaTime;
    xbool                   m_bFirstPersonCamera;
    vector3                 m_FirstPersonOffset;


    virtual void  ComputeView   ( view& View );
   
//=========================================================================
// Data
//=========================================================================
protected:
            void    UpdateInput     ( void );
            void    UpdateCamera    ( f32 DeltaTime );


protected:
    virtual void  VehicleInit   ( void );
    virtual void  VehicleKill   ( void );
    virtual void  VehicleReset  ( void );
    virtual void  VehicleUpdate ( f32 DeltaTime );
    virtual void  VehicleRender ( void );


//=========================================================================
// Editor
//=========================================================================
protected:

    // Enum functions
    virtual void        OnEnumProp      ( prop_enum&    List ) ;
    virtual xbool       OnProperty      ( prop_query&   I    ) ;
} ;

//=========================================================================

#endif//__COP_CAR_HPP__
