#ifndef	__COKE_CAN_HPP__
#define __COKE_CAN_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "Obj_mgr\obj_mgr.hpp"
#include "Objects\Render\SkinInst.hpp"
#include "Characters\FloorProperties.hpp"

//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================
class actor;
struct coke_can_tweaks;


//=========================================================================
// CLASS
//=========================================================================

class coke_can : public object
{
//=========================================================================
// Structures
//=========================================================================
private:
    struct particle
    {
        // Members
        vector3 m_BindPos;     // Position of particle when binded to geometry
        vector3 m_Pos;         // Current world position of particle
        vector3 m_LastPos;     // World position after last advance logic
        vector3 m_LastCollPos; // Last collision free position

        // Initialization
        particle() :
            m_Pos(0,0,0),
            m_LastPos(0,0,0),
            m_LastCollPos(0,0,0)
        {
        }

        // Velocity functions
        vector3 GetVelocity( void ) const { return m_Pos - m_LastPos; }
        void    SetVelocity( const vector3& Vel ) { m_LastPos = m_Pos - Vel; }
    };

//=========================================================================
// Class info
//=========================================================================
public:

    CREATE_RTTI( coke_can, object, object )
    virtual const object_desc&  GetTypeDesc     ( void ) const;    
    static  const object_desc&  GetObjectType   ( void );


//=========================================================================
// Public functions
//=========================================================================
public:
                        coke_can               ( void );
    virtual            ~coke_can               ( void );

    virtual bbox        GetLocalBBox		    ( void ) const;
            bbox        GetGeomBBox             ( void ) const;
    virtual s32         GetMaterial				( void ) const { return MAT_TYPE_SOLID_METAL; }
    virtual void        OnRender                ( void );
    virtual const char* GetLogicalName          ( void )   { return "COKE_CAN"; }

#ifndef X_RETAIL
    virtual void        OnColRender             ( xbool bRenderHigh );
#endif // X_RETAIL

    virtual void        OnAdvanceLogic          ( f32 DeltaTime );  
    virtual void        OnPain                  ( const pain& Pain );    
    virtual void        OnColCheck              ( void );
    virtual void        OnMove                  ( const vector3& NewPos );        
    virtual void        OnTransform             ( const matrix4& L2W    ); 
    virtual void        OnColNotify             ( object& Object );

#ifdef X_EDITOR
    virtual s32         OnValidateProperties    ( xstring& ErrorMsg );
#endif // X_EDITOR

//=========================================================================
// Private functions
//=========================================================================
protected:

            // Misc
            const coke_can_tweaks& GetProfile( void );

            // Physics functions
            void    InitPhysics                 ( void );
            f32     GetEnergy                   ( void );
            void    UpdateL2W                   ( void );
            void    Integrate                   ( f32 DeltaTime );
            void    ApplyEqualDistConstraint    ( particle& ParticleA, particle& ParticleB, f32 EqualDist );
            f32     ApplyMinDistConstraint      ( particle& ParticleA, particle& ParticleB, f32 MinDist, 
                                                  f32 TotalInvMass, f32 InvMassA, f32 InvMassB );
            void    ApplyDistConstraints        ( void );
            xbool   ApplyCylinderConstraint     ( const vector3& Bottom, const vector3& Top, f32 Radius, vector3& CollNorm );
            void    ApplyCollConstraints        ( void );
            void    ApplyCanConstraints         ( coke_can& CokeCan );
            void    ApplyCanConstraints         ( void );
            void    ApplyActorConstraints       ( void );
            void    ApplyConstraints            ( void );
            void    ApplyDamping                ( void );

//=========================================================================
// Public functions
//=========================================================================
public:
            void    ApplyActorConstraints       ( actor& Actor );




//=========================================================================
// Editor functions
//=========================================================================
protected:
    
    virtual void    OnEnumProp      ( prop_enum&    List );
    virtual xbool   OnProperty      ( prop_query&   I    );

//=========================================================================
// Profiles
//=========================================================================
public:
    enum {
            PROFILE_CAN         = 0,
            PROFILE_BARREL      = 1,
            
            PROFILE_COUNT
         };

//=========================================================================
// Data
//=========================================================================
protected:  

    // Flags
    u32                 m_bInitialized : 1;         // TRUE if initialized
    u32                 m_bOnGround    : 1;         // TRUE if lying on the ground

    // Physics    
    s32                 m_ActiveCount;              // Forces physics to update
    particle            m_Particles[2];             // Particles
    f32                 m_ParticleRadius;           // Radius of particles
    f32                 m_ParticleDist;             // Constraint distance
    radian              m_Roll;                     // Roll of can
    radian              m_RollSpeed;                // Roll speed of can
    f32                 m_DeltaTime;                // Accumulated delta time
    s32                 m_iMajorAxis;               // Longest axis of can
    vector3             m_MinInitVel;               // Min initial velocity
    vector3             m_MaxInitVel;               // Max initial velocity
    s32                 m_RollAudioID;              // Can rolling audio id
    s32                 m_iProfile;                 // Which physics profile to use

    // Rendering
    skin_inst           m_SkinInst;                 // Skinned inst
    floor_properties    m_FloorProperties;          // Floor tracking class
};

//=========================================================================


//=========================================================================
// END
//=========================================================================
#endif
