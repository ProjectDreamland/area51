//=========================================================================
//
//  VehicleObject.cpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "VehicleObject.hpp"
#include "Obj_mgr\obj_mgr.hpp"
#include "Entropy.hpp"

//=========================================================================
// DEFINES
//=========================================================================


//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct vehicle_object_desc : public object_desc
{
        vehicle_object_desc( void ) : object_desc( 
            object::TYPE_VEHICLE, 
            "Vehicle",
            "VEHICLE", 
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_COLLIDABLE         | 
            object::ATTR_RENDERABLE         | 
            object::ATTR_SPACIAL_ENTRY      |
            object::ATTR_DAMAGEABLE         |
            object::ATTR_LIVING,
            FLAGS_GENERIC_EDITOR_CREATE | FLAGS_IS_DYNAMIC )   { }

    virtual object* Create( void ) { return new vehicle_object; }

#ifdef X_EDITOR
    s32 OnEditorRender( object& Object ) const
    {
        (void)Object;
        //if( Object.GetAttrBits() & object::ATTR_EDITOR_SELECTED )
        //    Object.OnDebugRender();

        return -1;
    }
#endif // X_EDITOR

} s_vehicle_object_desc;

//=============================================================================

const object_desc& vehicle_object::GetTypeDesc( void ) const
{
    return s_vehicle_object_desc;
}

//=============================================================================

const object_desc& vehicle_object::GetObjectType( void )
{
    return s_vehicle_object_desc;
}

//=========================================================================
// CAMERA
//=========================================================================

void vehicle_object::camera::Init( void )
{
    m_Focus.Zero();
    m_FocusYaw = 0;
    m_Pitch = -R_20;
    m_Yaw   = R_0;
    m_Dist  = 600.0f;
}

//=========================================================================

void vehicle_object::camera::Update( const vector3&  Focus, 
                            radian    FocusYaw,
                            radian    DeltaPitch,
                            radian    DeltaYaw,
                            f32       DeltaZoom,
                            f32       DeltaTime
                            )
{
    m_Focus     = Focus;
    m_FocusYaw  = FocusYaw;

    m_Yaw     += DeltaYaw * DeltaTime * R_90;
    //if( m_Yaw < -R_80) m_Yaw = -R_80;
    //if( m_Yaw > +R_80) m_Yaw = +R_80;

    m_Pitch   += DeltaPitch * DeltaTime * R_90;
    if( m_Pitch > +R_10) m_Pitch = R_10;
    if( m_Pitch < -R_80) m_Pitch = -R_80;

    m_Dist += DeltaZoom * DeltaTime * 300.0f;
    if( m_Dist < 100.0f ) m_Dist = 100.0f;
    if( m_Dist > 2000.0f) m_Dist = 2000.0f;
}

//=========================================================================

void vehicle_object::camera::ComputeView( view& View )
{
    View.OrbitPoint( m_Focus, m_Dist, m_Pitch, m_Yaw+R_180 );
}

//=========================================================================
// CAR
//=========================================================================

// Constructor
vehicle_object::vehicle_object() 
{
}

//=========================================================================

// Destructor
vehicle_object::~vehicle_object()
{
}

//=========================================================================
// INHERITED VIRTUAL FUNCTIONS FROM BASE CLASS
//=========================================================================

void vehicle_object::OnInit( void )
{
    m_Camera.Init();
    VehicleInit();
    m_DeltaTime = 0;
    m_bFirstPersonCamera = FALSE;
    m_FirstPersonOffset = vector3(30,150,-75);
}

//=============================================================================

void vehicle_object::OnKill( void )
{
    VehicleKill();
}

//=============================================================================

void vehicle_object::UpdateInput( void )
{
    m_Input.Clear();

    // Steering
    m_Input.m_Steer = 0;//-input_GetValue(INPUT_PS2_STICK_LEFT_X) ;
    //m_Input.m_Steer = -input_GetValue(INPUT_PS2_STICK_LEFT_X) ;

    // Accel
    m_Input.m_Accel = input_GetValue(INPUT_PS2_STICK_LEFT_Y) ;
    //m_Input.m_Accel = -input_GetValue(INPUT_PS2_STICK_RIGHT_X) ;

    // Braking
    {
        m_Input.m_Brake = input_GetValue( INPUT_PS2_BTN_CROSS );
    }

    // Camera inputs
    {
/*
        if( input_IsPressed( INPUT_PS2_BTN_L_RIGHT ) )
            m_Input.m_CameraYaw =  1.0f;
        else
        if( input_IsPressed( INPUT_PS2_BTN_L_LEFT ) )
            m_Input.m_CameraYaw = -1.0f;

        if( input_IsPressed( INPUT_PS2_BTN_L_UP ) )
            m_Input.m_CameraPitch = -1.0f;
        else
        if( input_IsPressed( INPUT_PS2_BTN_L_DOWN ) )
            m_Input.m_CameraPitch =  1.0f;
*/
        m_Input.m_CameraYaw = -input_GetValue(INPUT_PS2_STICK_RIGHT_X);
        if( m_Input.m_CameraYaw > 0 )
            m_Input.m_CameraYaw = +m_Input.m_CameraYaw*m_Input.m_CameraYaw;
        else
            m_Input.m_CameraYaw = -m_Input.m_CameraYaw*m_Input.m_CameraYaw;

        m_Input.m_CameraPitch = -input_GetValue(INPUT_PS2_STICK_RIGHT_Y);

        m_Input.m_CameraZoom = 0;
        if( input_IsPressed(INPUT_PS2_BTN_TRIANGLE ))
            m_Input.m_CameraZoom = -1.0f;
        else
        if( input_IsPressed(INPUT_PS2_BTN_CIRCLE ))
            m_Input.m_CameraZoom = +1.0f;
    }

    m_Input.m_bToggleCamera = input_WasPressed( INPUT_PS2_BTN_SQUARE );

    m_Input.m_bReset = input_WasPressed( INPUT_PS2_BTN_SELECT );
}

//=============================================================================

void vehicle_object::UpdateCamera( f32 DeltaTime )
{
    m_Camera.Update( GetPosition() + vector3(0,125,0), 
                     GetL2W().GetRotation().Yaw,
                     m_Input.m_CameraPitch,
                     m_Input.m_CameraYaw,
                     m_Input.m_CameraZoom,
                     DeltaTime );
}

//=============================================================================

void vehicle_object::OnAdvanceLogic( f32 DeltaTime )
{
    UpdateInput();

    //if( m_Input.m_bToggleCamera )
    //    m_bFirstPersonCamera = !m_bFirstPersonCamera;

    if( m_Input.m_bReset )
        VehicleReset();

//    vector3 OldPosition = GetPosition();

    m_DeltaTime += DeltaTime;


xtimer Timer;
Timer.Start();

    f32 MinTimeStep = 0.020f;
    f32 MaxTimeStep = 0.100f;
    while( m_DeltaTime > MinTimeStep )
    {
        f32 TimeStep = MIN(m_DeltaTime,MaxTimeStep);
        m_DeltaTime -= TimeStep;
        VehicleUpdate( TimeStep );
        UpdateCamera( TimeStep );
    }

Timer.Stop();

//    vector3 NewPosition = GetPosition();
//    f32     CMPerSec = (NewPosition - OldPosition).Length() / DeltaTime;
//    f32     MPH = CMPerSec / ((12.0f*2.54f*5280.0f)/3600.0f);

    //x_DebugMsg("SPEED: %5.2f mph\n", MPH );
//    x_DebugMsg("LogicTime: %5.2f\n",Timer.ReadMs() );
}

//=============================================================================
// FUNCTIONS
//=============================================================================

//=========================================================================

void vehicle_object::VehicleInit( void )
{
}

//=========================================================================

void vehicle_object::VehicleKill( void )
{
}

//=========================================================================

void vehicle_object::VehicleUpdate( f32 DeltaTime )
{
    (void)DeltaTime;
}

//=========================================================================

void vehicle_object::VehicleReset( void )
{
    m_bFirstPersonCamera = FALSE;
    m_Camera.Init();
    m_Camera.m_Focus = GetPosition();
    m_Camera.m_FocusYaw = GetL2W().GetRotation().Yaw;
    m_Camera.m_Yaw = m_Camera.m_FocusYaw;
}

//=========================================================================

void vehicle_object::VehicleRender( void )
{
    draw_BBox( GetBBox() ); 
}

//=========================================================================
// EDITOR FUNCTIONS
//=========================================================================

//=============================================================================
// Enum functions
//=============================================================================

void vehicle_object::OnEnumProp ( prop_enum& List )
{
    object::OnEnumProp( List );

    // Header
    List.AddHeader(     "Vehicle",  
                        "Vehicle is the base class for all vehicles in the game" 
                        ) ;

    // Geometry
    m_RigidInst.OnEnumProp(List) ;

    // Animation
    List.AddExternal("RenderInst\\Anim", "Resource\0anim", "Resource File", PROP_TYPE_MUST_ENUM) ;
}

//=============================================================================

xbool vehicle_object::OnProperty ( prop_query& I )
{
    // Call base class
    if (object::OnProperty(I))
        return TRUE ;

    // Geometry
    if (m_RigidInst.OnProperty(I))
        return TRUE ;

    // Animation
    if (I.IsVar( "RenderInst\\Anim"))
    {
        if (I.IsRead())
            I.SetVarExternal(m_hAnimGroup.GetName(), RESOURCE_NAME_SIZE) ;
        else
        {
            // Anim changed?
            if( I.GetVarExternal()[0] )
            {
                const char* pAnimFile = I.GetVarExternal() ;
                m_hAnimGroup.SetName(pAnimFile) ;
                if (m_hAnimGroup.GetPointer())
                {
                    // Initialize ai?
                    const char* pGeomFile = m_RigidInst.GetRigidGeomName() ;
                    if ((pGeomFile) && (pAnimFile))
                        OnInit() ;
                }
            }
        }
        return TRUE ;
    }

    return FALSE;
}

//=============================================================================

void vehicle_object::OnRender( void )
{
    VehicleRender();
}

//=============================================================================

bbox vehicle_object::GetLocalBBox( void ) const
{
    bbox BBox;
    BBox.Min = vector3(-100,-100,-100);
    BBox.Max = vector3(+100,+100,+100);
    return BBox;
}

//=============================================================================

s32 vehicle_object::GetMaterial( void ) const
{
    return 0;
}

//=============================================================================

void vehicle_object::ComputeView( view& View )
{
    if( m_bFirstPersonCamera )
    {
        vector3 Eye = GetL2W() * m_FirstPersonOffset;
        View.SetPosition( Eye );
        View.SetRotation( GetL2W().GetQuaternion() );
    }
    else
    {
        m_Camera.ComputeView( View );
    }
}

//=============================================================================
