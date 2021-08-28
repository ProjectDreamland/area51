//==============================================================================
//
//  Main.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "Ode\Ode.h"


//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================
// MAIN
//==============================================================================
//==============================================================================
//==============================================================================
//==============================================================================


//==============================================================================
//  STORAGE
//==============================================================================

view            View2;
view            View;
random          R;
s32             ScreenW;
s32             ScreenH;
#pragma warning( disable : 4355 ) // warning 'this' used in base member initializer list

#ifdef TARGET_PC
    const char*     DataPath = "";
#endif

#ifdef TARGET_GCN
    const char*     DataPath = "";
#endif

void ResetRagdoll( void ) ;


//==============================================================================
// ODE DEFINITION STRUCTURES
//==============================================================================

struct box_def
{
    const   char*   pName ;         // Name
            f32     PX,PY,PZ ;      // Position
            f32     RX,RY,RZ ;      // Rotation
            f32     SX,SY,SZ ;      // Size
            xbool   bCollision ;    // Collision
} ;

struct sphere_def
{
    const   char*   pName ;         // Name
            f32     PX,PY,PZ ;      // Position
            f32     Radius ;        // Radius
            xbool   bCollision ;    // Collision
} ;

struct capped_cylinder_def
{
    const   char*   pName ;         // Name
            f32     PX,PY,PZ ;      // Position
            f32     RX,RY,RZ ;      // Rotation
            f32     R,H ;           // Radius, Height
            xbool   bCollision ;    // Collision
} ;

struct hinge_joint_def
{
    const   char*   pName ;                 // Name
            f32     PX,PY,PZ ;              // Position
    const   char*   pObjectA ;              // ObjectA
    const   char*   pObjectB ;              // ObjectB
            f32     AX,AY,AZ ;              // Axis
            f32     MinAngle, MaxAngle ;    // Forward/backward rotation limit
} ;

struct universal_joint_def
{
    const   char*   pName ;                 // Name
            f32     PX,PY,PZ ;              // Position
    const   char*   pObjectA ;              // ObjectA
    const   char*   pObjectB ;              // ObjectB
            f32     AX1,AY1,AZ1 ;           // Axis
            f32     AX2,AY2,AZ2 ;           // Axis
} ;


//==============================================================================
// MAX EXPORTED DATA
//==============================================================================

box_def BoxDefs[] =
{
    { "ODE Box L Foot",
      16.4975f, -2.49552e-007f, 5.70909f,
      90.0f, 0.0f, 9.0f,
      15.4f, 11.5f, 28.0f,
      true},

    { "ODE Box L Calf",
      14.0912f, 5.46248f, 34.8972f,
      96.9524f, -6.05586f, 8.32943f,
      8.0f, 50.0f, 8.0f,
      true},

    { "Box L Thigh",
      10.1657f, -3.64372e-006f, 83.3587f,
      97.0f, -5.0f, 0.0f,
      8.0f, 47.7f, 8.0f,
      true},

    { "ODE Box R Calf",
      -14.5899f, 5.46248f, 34.8972f,
      -83.0476f, 6.05586f, -8.32943f,
      8.0f, 50.0f, 8.0f,
      true},

    { "ODE Box R Foot",
      -16.9961f, -2.42393e-007f, 5.70909f,
      -90.0f, 0.0f, -9.0f,
      15.4f, 11.5f, 28.0f,
      true},

    { "ODE Box R Thigh",
      -10.6643f, -3.10205e-006f, 83.3587f,
      -83.0f, 5.0f, 2.14259e-007f,
      8.0f, 47.7f, 8.0f,
      true},

    { "Box Hips",
      -0.0769519f, -2.63419f, 113.538f,
      90.0f, 0.0f, 0.0f,
      21.7f, 36.4f, 11.7f,
      false},

    { "Box Spine3",
      0.0287967f, -6.43179e-006f, 147.142f,
      90.0f, 0.0f, 0.0f,
      25.641f, 28.0817f, 10.1648f,
      false},

    { "Box Neck",
      -0.0313177f, -7.11085e-006f, 162.677f,
      90.0f, 0.0f, 0.0f,
      5.63354f, 5.30977f, 10.7491f,
      false},

    { "Box L UpperArm",
      29.3372f, -6.89114e-006f, 157.651f,
      90.0f, 5.0f, 0.0f,
      43.867f, 6.41025f, 9.34066f,
      true},

    { "Box L LowerArm",
      64.4445f, -6.75013e-006f, 154.425f,
      90.0f, 5.0f, 0.0f,
      26.867f, 6.41025f, 9.34066f,
      true},

    { "Box L Hand",
      89.1314f, -6.65314e-006f, 152.206f,
      90.0f, 5.0f, 0.0f,
      22.267f, 6.41025f, 15.9407f,
      true},

    { "Box R LowerArm",
      -64.2609f, -6.74995e-006f, 154.403f,
      -90.0f, -5.0f, 0.0f,
      26.867f, 6.41025f, 9.34066f,
      true},

    { "Box R Hand",
      -88.9479f, -6.65296e-006f, 152.184f,
      -90.0f, -5.0f, 0.0f,
      22.267f, 6.41025f, 15.9407f,
      true},

    { "Box R UpperArm",
      -29.1537f, -6.89096e-006f, 157.629f,
      -90.0f, -5.0f, 0.0f,
      43.867f, 6.41025f, 9.34066f,
      true},

} ;
sphere_def SphereDefs[] =
{
    { "Sphere L Hip",
      9.25321f, -4.42625e-006f, 101.261f,
      4.0f,
      true},

    { "Sphere R Hip",
      -8.8041f, -4.42625e-006f, 101.261f,
      4.0f,
      true},

    { "Sphere Head	",
      0.0219707f, -7.78008e-006f, 177.988f,
      14.3736f,
      true},

    { "Sphere L Clavicle	",
      12.8279f, -6.98114e-006f, 159.71f,
      2.65568f,
      false},

    { "Sphere R Clavicle",
      -12.7262f, -6.98114e-006f, 159.71f,
      2.65568f,
      false},

} ;
hinge_joint_def HingeJointDefs[] =
{
    { "ODE HingeJoint L Ankle	",
      16.2822f, 8.78896f, 10.4791f,
      "ODE Box L Calf", "ODE Box L Foot",
      0.984808f, 0.173648f, 0.0f,
      -65.0f, 5.0f},

    { "HingeJoint L Knee	",
      11.7872f, 3.3779f, 59.3031f,
      "Box L Thigh", "ODE Box L Calf",
      0.994522f, 0.104528f, 0.0f,
      -145.0f, 0.0f},

    { "ODE HingeJoint R Ankle",
      -16.7808f, 8.78896f, 10.4791f,
      "ODE Box R Calf", "ODE Box R Foot",
      0.984808f, -0.173648f, 0.0f,
      -65.0f, 5.0f},

    { "HingeJoint R Knee",
      -12.2858f, 3.3779f, 59.3031f,
      "ODE Box R Thigh", "ODE Box R Calf",
      0.994522f, -0.104528f, 0.0f,
      -145.0f, 0.0f},

    { "HingeJoint L Hip FB",
      1.92126f, -4.43288e-006f, 101.412f,
      "Box Hips", "Sphere L Hip",
      1.0f, 0.0f, 0.0f,
      -1.0f, 60.0f},

    { "HingeJoint L Hip FB",
      -2.14984f, -4.43288e-006f, 101.412f,
      "Box Hips", "Sphere R Hip",
      1.0f, 0.0f, 0.0f,
      -1.0f, 60.0f},

    { "HingeJoint L Hip S",
      9.38352f, -4.42876e-006f, 101.318f,
      "Sphere L Hip", "Box L Thigh",
      0.0f, -1.0f, 0.0f,
      -30.0f, 5.0f},

    { "HingeJoint R Hip S",
      -8.88524f, -4.42876e-006f, 101.318f,
      "Sphere R Hip", "ODE Box R Thigh",
      0.0f, -1.0f, 0.0f,
      -5.0f, 30.0f},

    { "HingeJoint Back04",
      -0.129573f, -5.78181e-006f, 132.272f,
      "Box Spine3", "Box Hips",
      1.0f, 0.0f, 0.0f,
      -1.0f, 1.0f},

    { "HingeJoint Neck",
      -0.129573f, -6.9556e-006f, 159.126f,
      "Box Neck", "Box Spine3",
      1.0f, 0.0f, 0.0f,
      -10.0f, 25.0f},

    { "HingeJoint Head	",
      0.539998f, -7.24973e-006f, 165.854f,
      "Sphere Head	", "Box Neck",
      1.0f, 0.0f, 0.0f,
      -45.0f, 45.0f},

    { "HingeJoint L Clavicle FB	",
      12.8508f, -6.97814e-006f, 159.641f,
      "Sphere L Clavicle	", "Box Spine3",
      1.0f, 0.0f, 0.0f,
      -75.0f, 75.0f},

    { "HingeJoint R Clavicle FB	01",
      -12.6656f, -6.97814e-006f, 159.641f,
      "Sphere R Clavicle", "Box Spine3",
      1.0f, 0.0f, 0.0f,
      -75.0f, 75.0f},

    { "HingeJoint L Clavicle S",
      12.7749f, -6.97733e-006f, 159.623f,
      "Box L UpperArm", "Sphere L Clavicle	",
      0.0f, -1.0f, 0.0f,
      -150.0f, 150.0f},

    { "HingeJoint L Elbow",
      51.0092f, -6.80083e-006f, 155.585f,
      "Box L LowerArm", "Box L UpperArm",
      0.0f, -1.0f, 0.0f,
      -125.0f, 1.0f},

    { "HingeJoint L Wrist",
      78.0721f, -6.69227e-006f, 153.101f,
      "Box L LowerArm", "Box L Hand",
      0.0f, -1.0f, 0.0f,
      -75.0f, 75.0f},

    { "HingeJoint R Clavicle S",
      -12.5914f, -6.97715e-006f, 159.601f,
      "Box R UpperArm", "Sphere R Clavicle",
      0.0f, 1.0f, 0.0f,
      -150.0f, 150.0f},

    { "HingeJoint R Elbow",
      -50.8257f, -6.80065e-006f, 155.563f,
      "Box R LowerArm", "Box R UpperArm",
      0.0f, 1.0f, 0.0f,
      -125.0f, 1.0f},

    { "HingeJoint R Wrist",
      -77.8886f, -6.69209e-006f, 153.079f,
      "Box R LowerArm", "Box R Hand",
      0.0f, 1.0f, 0.0f,
      -75.0f, 75.0f},

} ;


//==============================================================================
// RUNTIME DATA
//==============================================================================

struct body
{
    const char* m_pName ;
    xbool       m_bCollision ;
    vector3     m_Pos ;
    radian3     m_Rot ;
    dBodyID     m_BodyID ;
    dGeomID     m_GeomID ;
} ;

struct joint
{
    const char* m_pName ;
    dJointID    m_JointID ;
} ;

body        s_Bodys[100] ;
s32         s_NBodys= 0 ;

joint       s_Joints[100] ;
s32         s_NJoints ;

dWorldID    s_WorldID ;

dBodyID     s_PlaneBodyID ;
dGeomID     s_PlaneGeomID ;
            
dSpaceID    s_SpaceID ;

dJointGroupID s_ContactJointGroupID ;
s32           s_NContactJoints = 0 ;

xtimer      s_SimTimer ;
xtimer      s_CollTimer ;


//==============================================================================
// UTIL
//==============================================================================

dBodyID FindBodyID( const char* pName )
{
    for (s32 i = 0 ; i < s_NBodys ; i++)
    {
        if (x_stricmp(s_Bodys[i].m_pName, pName) == 0)
            return s_Bodys[i].m_BodyID ;
    }

    return 0 ;
}


#define DMATRIX3(_R_,_i_,_j_) _R_[(_i_)*4+(_j_)]

//=========================================================================

void SetBodyL2W( dBodyID BodyID, const matrix4& L2W )
{
    // Reset position
    dBodySetPosition(BodyID, L2W(3,0), L2W(3,1), L2W(3,2)) ;

    // Reset rotation
    dMatrix3 Rot ;
    dRSetIdentity(Rot) ;
    DMATRIX3(Rot, 0,0) = L2W(0,0) ;
    DMATRIX3(Rot, 0,1) = L2W(1,0) ;
    DMATRIX3(Rot, 0,2) = L2W(2,0) ;
    DMATRIX3(Rot, 1,0) = L2W(0,1) ;
    DMATRIX3(Rot, 1,1) = L2W(1,1) ;
    DMATRIX3(Rot, 1,2) = L2W(2,1) ;
    DMATRIX3(Rot, 2,0) = L2W(0,2) ;
    DMATRIX3(Rot, 2,1) = L2W(1,2) ;
    DMATRIX3(Rot, 2,2) = L2W(2,2) ;
    dBodySetRotation(BodyID, Rot) ;
}

//=========================================================================

matrix4 GetBodyL2W( dBodyID BodyID )
{ 
    // Get info
    const dReal* Pos = dBodyGetPosition(BodyID) ;
    const dReal* Rot = dBodyGetRotation(BodyID) ;

    // Setup Local->world
    matrix4 L2W ;
    L2W.Identity() ;
    L2W.SetTranslation(vector3(Pos[0], Pos[1], Pos[2])) ;

    L2W(0,0) = DMATRIX3(Rot, 0,0) ;
    L2W(1,0) = DMATRIX3(Rot, 0,1) ;
    L2W(2,0) = DMATRIX3(Rot, 0,2) ;
    L2W(0,1) = DMATRIX3(Rot, 1,0) ;
    L2W(1,1) = DMATRIX3(Rot, 1,1) ;
    L2W(2,1) = DMATRIX3(Rot, 1,2) ;
    L2W(0,2) = DMATRIX3(Rot, 2,0) ;
    L2W(1,2) = DMATRIX3(Rot, 2,1) ;
    L2W(2,2) = DMATRIX3(Rot, 2,2) ;

    return L2W ;
}

//==============================================================================

void ConvertMaxToEng( matrix4& L2W )
{
    // Convert from max space to engine space
    matrix4 MaxToEng ;
    MaxToEng.Identity() ;
    MaxToEng.RotateX(DEG_TO_RAD(-90)) ;
    MaxToEng.RotateY(DEG_TO_RAD(180)) ;
    L2W = MaxToEng * L2W ;
}

//==============================================================================

void ConvertMaxToEng( vector3& V )
{
    // Convert from max space to engine space
    matrix4 MaxToEng ;
    MaxToEng.Identity() ;
    MaxToEng.RotateX(DEG_TO_RAD(-90)) ;
    MaxToEng.RotateY(DEG_TO_RAD(180)) ;
    V = MaxToEng * V ;
}


//==============================================================================
// FUNCTIONS
//==============================================================================

void MoveRagdoll( void )
{
    // Move all bodys
    for (s32 i = 0 ; i < s_NBodys ; i++)
    {
        // Lookup body info
        body&       Body = s_Bodys[i] ;
        matrix4 L2W = GetBodyL2W(Body.m_BodyID) ;
        L2W.Translate(vector3(0,140,0)) ;
        SetBodyL2W(Body.m_BodyID, L2W) ;
    }
}

/*
// damp the rotational motion of body 0 a bit

void dampRotationalMotion (dReal kd)
{
  const dReal *w = dBodyGetAngularVel (body[0]);
  dBodyAddTorque (body[0],-kd*w[0],-kd*w[1],-kd*w[2]);
}


// add a spring force to keep the bodies together, otherwise they may fly
// apart with some joints.

void addSpringForce (dReal ks)
{
  const dReal *p1 = dBodyGetPosition (body[0]);
  const dReal *p2 = dBodyGetPosition (body[1]);
  dBodyAddForce (body[0],ks*(p2[0]-p1[0]),ks*(p2[1]-p1[1]),ks*(p2[2]-p1[2]));
  dBodyAddForce (body[1],ks*(p1[0]-p2[0]),ks*(p1[1]-p2[1]),ks*(p1[2]-p2[2]));
}
*/

//=========================================================================

void Initialize( void )
{
    s32 i ;

    eng_Init();

    View.SetXFOV( R_60 );
    f32 D = 0.5f ;

    View.SetPosition( vector3(0, 250*D, -700*D) );
    View.LookAtPoint( vector3(  0,  0,  0) );
    View.SetZLimits ( 0.1f, 10000.0f );

    View2 = View;

    eng_GetRes( ScreenW, ScreenH );


    // Create ODE bits

    // Create world
    s_WorldID = dWorldCreate() ;
    
    dWorldSetGravity(s_WorldID, 0, -9.81f*10, 0) ;
    dWorldSetERP(s_WorldID, 0.5f) ;
    dWorldSetCFM(s_WorldID, 0.0000001f) ;

    // Create space
    s_SpaceID = dSimpleSpaceCreate(0) ;

    // Create joint groupd
    s_ContactJointGroupID = dJointGroupCreate(0) ;

    // Create box bodys
    for (i = 0 ; i < (sizeof(BoxDefs) / sizeof(BoxDefs[0])) ; i++)
    {
        // Lookup definition
        box_def& Def = BoxDefs[i] ;
        ASSERT(FindBodyID(Def.pName) == 0) ;

        // Create new body
        body& Body = s_Bodys[s_NBodys++] ;
        Body.m_pName        = Def.pName ;
        Body.m_bCollision   = Def.bCollision ;
        Body.m_Pos          = vector3(Def.PX, Def.PY, Def.PZ) ;
        Body.m_Rot          = radian3(Def.RX, Def.RY, Def.RZ) ;
        Body.m_BodyID       = dBodyCreate(s_WorldID) ;
        Body.m_GeomID       = dCreateBox(s_SpaceID, Def.SX, Def.SY, Def.SZ) ;
        dGeomSetData(Body.m_GeomID, &Body) ;

        // Connect body and geom
        dGeomSetBody(Body.m_GeomID, Body.m_BodyID) ;

        // Set mass of body
        dMass Mass ;
        dMassSetBox(&Mass,
                    1.0f,           // density,
                    Def.SX, Def.SY, Def.SZ) ;   // Size
        //dMassSetBox(&Mass,
                    //Def.SX * Def.SY * Def.SZ,   // density,
                    //1,1,1) ;                                // Size
        dBodySetMass(Body.m_BodyID, &Mass) ;

        dBodySetFiniteRotationMode(Body.m_BodyID, 1) ;
    }


    // Create sphere bodys
    for (i = 0 ; i < (sizeof(SphereDefs) / sizeof(SphereDefs[0])) ; i++)
    {
        // Lookup definition
        sphere_def& Def = SphereDefs[i] ;
        ASSERT(FindBodyID(Def.pName) == 0) ;

        // Create new body
        body& Body = s_Bodys[s_NBodys++] ;
        Body.m_pName        = Def.pName ;
        Body.m_bCollision   = Def.bCollision ;
        Body.m_Pos          = vector3(Def.PX, Def.PY, Def.PZ) ;
        Body.m_BodyID       = dBodyCreate(s_WorldID) ;
        Body.m_GeomID       = dCreateSphere(s_SpaceID, Def.Radius) ;
        dGeomSetData(Body.m_GeomID, &Body) ;

        // Connect body and geom
        dGeomSetBody(Body.m_GeomID, Body.m_BodyID) ;

        // Set mass of body
        dMass Mass ;
        dMassSetSphere(&Mass,
                    1.0f,           // density,
                    Def.Radius) ;   // radius
        dBodySetMass(Body.m_BodyID, &Mass) ;
        dBodySetFiniteRotationMode(Body.m_BodyID, 1) ;
    }


    // Reset bodys ready for putting in joints
    ResetRagdoll() ;

    // Create hinge joints
    for (i = 0 ; i < (sizeof(HingeJointDefs) / sizeof(HingeJointDefs[0])) ; i++)
    {
        // Lookup definition
        hinge_joint_def& Def = HingeJointDefs[i] ;

        // Create new joint
        joint& Joint = s_Joints[s_NJoints++] ;

        // Setup joint
        Joint.m_pName = Def.pName ;
        Joint.m_JointID = dJointCreateHinge(s_WorldID, 0) ;
        ASSERT(FindBodyID(Def.pObjectA)) ;
        ASSERT(FindBodyID(Def.pObjectB)) ;
        dJointAttach(Joint.m_JointID, FindBodyID(Def.pObjectA), FindBodyID(Def.pObjectB)) ; 

        vector3 Anchor(Def.PX, Def.PY, Def.PZ) ;
        vector3 Axis  (Def.AX, Def.AY, Def.AZ) ;
        ConvertMaxToEng(Anchor) ;
        ConvertMaxToEng(Axis) ;

        dJointSetHingeAnchor(Joint.m_JointID, Anchor.X, Anchor.Y, Anchor.Z) ; 
        dJointSetHingeAxis  (Joint.m_JointID, Axis.X,   Axis.Y,   Axis.Z) ; 

        dJointSetHingeParam (Joint.m_JointID, dParamLoStop, DEG_TO_RAD(Def.MinAngle)) ; // Forward
        dJointSetHingeParam (Joint.m_JointID, dParamHiStop, DEG_TO_RAD(Def.MaxAngle)) ; // Backward
        
        //dJointSetHingeParam (Joint.m_JointID, dParamFudgeFactor, 1.0f) ;    // Stop force amount
        //dJointSetHingeParam (Joint.m_JointID, dParamBounce, 0.0f) ;    // Stop bouncyness

        //dJointSetHingeParam (Joint.m_JointID, dParamStopERP, 0.8f) ; // Good error correction
        //dJointSetHingeParam (Joint.m_JointID, dParamStopCFM, 0.00001f) ;   // Make hard
    }

    /*
    // Create universal joints
    for (i = 0 ; i < (sizeof(UniversalJointDefs) / sizeof(UniversalJointDefs[0])) ; i++)
    {
        // Lookup definition
        universal_joint_def& Def = UniversalJointDefs[i] ;

        // Create new joint
        joint& Joint = s_Joints[s_NJoints++] ;

        // Setup joint
        Joint.m_pName = Def.pName ;
        Joint.m_JointID = dJointCreateUniversal(s_WorldID, 0) ;
        ASSERT(FindBodyID(Def.pObjectA)) ;
        ASSERT(FindBodyID(Def.pObjectB)) ;
        dJointAttach(Joint.m_JointID, FindBodyID(Def.pObjectA), FindBodyID(Def.pObjectB)) ; 

        vector3 Anchor(Def.PX, Def.PY, Def.PZ) ;
        vector3 Axis1 (Def.AX1, Def.AY1, Def.AZ1) ;
        vector3 Axis2 (Def.AX2, Def.AY2, Def.AZ2) ;
        ConvertMaxToEng(Anchor) ;
        ConvertMaxToEng(Axis1) ;
        ConvertMaxToEng(Axis2) ;

        dJointSetUniversalAnchor(Joint.m_JointID, Anchor.X, Anchor.Y, Anchor.Z) ; 
        dJointSetUniversalAxis1 (Joint.m_JointID, Axis1.X,  Axis1.Y,  Axis1.Z) ; 
        dJointSetUniversalAxis2 (Joint.m_JointID, Axis2.X,  Axis2.Y,  Axis2.Z) ; 

        dJointSetUniversalParam (Joint.m_JointID, dParamLoStop, DEG_TO_RAD(Def.MinAngle1)) ; // Forward
        dJointSetUniversalParam (Joint.m_JointID, dParamHiStop, DEG_TO_RAD(Def.MaxAngle1)) ; // Backward
        
        dJointSetUniversalParam (Joint.m_JointID, dParamLoStop2, DEG_TO_RAD(Def.MinAngle2)) ; // Forward
        dJointSetUniversalParam (Joint.m_JointID, dParamHiStop2, DEG_TO_RAD(Def.MaxAngle2)) ; // Backward
        
        //dJointSetUniversalParam (Joint.m_JointID, dParamFudgeFactor, 1.0f) ;    // Stop force amount
        //dJointSetUniversalParam (Joint.m_JointID, dParamBounce, 0.0f) ;    // Stop bouncyness

        //dJointSetUniversalParam (Joint.m_JointID, dParamStopERP, 0.8f) ; // Good error correction
        //dJointSetUniversalParam (Joint.m_JointID, dParamStopCFM, 0.00001f) ;   // Make hard
    }
    */

    // Create plane geom
    s_PlaneGeomID = dCreatePlane(s_SpaceID, 0,1,0,   0) ;

    MoveRagdoll() ;
}

//=========================================================================

void ResetRagdoll( void )
{
    // Reset all bodys
    for (s32 i = 0 ; i < s_NBodys ; i++)
    {
        // Lookup body info
        body&       Body = s_Bodys[i] ;

        // Create L2W
        matrix4 L2W, RotX, RotY, RotZ ;
        L2W.Identity() ;
        RotX.Identity() ;
        RotY.Identity() ;
        RotZ.Identity() ;
        RotX.RotateX(DEG_TO_RAD(Body.m_Rot.Pitch)) ;
        RotY.RotateY(DEG_TO_RAD(Body.m_Rot.Yaw)) ;
        RotZ.RotateZ(DEG_TO_RAD(Body.m_Rot.Roll)) ;
        L2W = RotZ * RotY * RotX ;
        L2W.SetTranslation(Body.m_Pos) ;
        ConvertMaxToEng(L2W) ;

        // Set L2W
        SetBodyL2W(Body.m_BodyID, L2W) ;

        // Reset velocities
        dBodySetLinearVel(Body.m_BodyID, 0,0,0) ;
        dBodySetAngularVel(Body.m_BodyID, 0,0,0) ;
    }
}

//=========================================================================

void Shutdown( void )
{
    dSpaceDestroy(s_SpaceID) ;
    dWorldDestroy(s_WorldID);
    dCloseODE() ;
}

//=========================================================================

#ifdef TARGET_PC
#define INPUT_RESET INPUT_KBD_SPACE
#define INPUT_U     INPUT_KBD_R
#define INPUT_D     INPUT_KBD_F
#define INPUT_F     INPUT_KBD_W
#define INPUT_B     INPUT_KBD_S
#define INPUT_L     INPUT_KBD_A
#define INPUT_R     INPUT_KBD_D

#define INPUT_PITCH_SIGN    1
#define INPUT_PITCH INPUT_MOUSE_Y_REL

#define INPUT_YAW_SIGN  1
#define INPUT_YAW       INPUT_MOUSE_X_REL

#endif

#ifdef TARGET_PS2
#define INPUT_RESET INPUT_PS2_BTN_START     
#define INPUT_U     INPUT_PS2_BTN_R1
#define INPUT_D     INPUT_PS2_BTN_R2
#define INPUT_F     INPUT_PS2_BTN_L_UP
#define INPUT_B     INPUT_PS2_BTN_L_DOWN
#define INPUT_L     INPUT_PS2_BTN_L_LEFT
#define INPUT_R     INPUT_PS2_BTN_L_RIGHT

#define INPUT_PITCH_SIGN    10
#define INPUT_PITCH         INPUT_PS2_STICK_LEFT_Y

#define INPUT_YAW_SIGN      10
#define INPUT_YAW           INPUT_PS2_STICK_LEFT_X

#endif

xbool HandleInput( f32 DeltaTime )
{
#ifdef TARGET_PC
    while( input_UpdateState() )
#else
    input_UpdateState() ;
#endif
    {
        // Move view using keyboard and mouse
        // WASD - forward, left, back, right
        // RF   - up, down
        // MouseRightButton - 4X speed

        radian Pitch;
        radian Yaw;
        f32    S = 300.0f * DeltaTime ;
        f32    R = 0.005f ;

        if (input_IsPressed(INPUT_KBD_ESCAPE  ))  return( FALSE );

        if (input_IsPressed(INPUT_MOUSE_BTN_L ))  S *= 4.0f;
        if (input_IsPressed(INPUT_F           ))  View.Translate( vector3( 0, 0, S), view::VIEW );
        if (input_IsPressed(INPUT_B           ))  View.Translate( vector3( 0, 0,-S), view::VIEW );
        if (input_IsPressed(INPUT_L           ))  View.Translate( vector3( S, 0, 0), view::VIEW );
        if (input_IsPressed(INPUT_R           ))  View.Translate( vector3(-S, 0, 0), view::VIEW );
        if (input_IsPressed(INPUT_U           ))  View.Translate( vector3( 0, S, 0), view::VIEW );
        if (input_IsPressed(INPUT_D           ))  View.Translate( vector3( 0,-S, 0), view::VIEW );

        View.GetPitchYaw( Pitch, Yaw );       
        Pitch += INPUT_PITCH_SIGN * input_GetValue( INPUT_PITCH ) * R;
        Yaw   -= INPUT_YAW_SIGN   * input_GetValue( INPUT_YAW ) * R;
        View.SetRotation( radian3(Pitch,Yaw,0) );

        if( input_IsPressed( INPUT_MSG_EXIT ) )
            return( FALSE );

        if (input_IsPressed(INPUT_RESET))
        {
            ResetRagdoll() ;
            MoveRagdoll() ;
        }
    }

    return( TRUE );
}

//==============================================================================

void draw_Quad( const vector3& A, 
                const vector3& B, 
                const vector3& C, 
                const vector3& D,
                xcolor Color, const vector3& LightDir )
{
    vector3 N  = (C - A).Cross(B - A) ;
    N.Normalize() ;
    f32 Dot = N.Dot(LightDir) * 0.7f ;
    if (Dot < 0)
        Dot = 0 ;
    Dot += 0.25f ;
    if (Dot > 1)
        Dot = 1 ;

    Color.R = (u8)((f32)Color.R * Dot) ;
    Color.G = (u8)((f32)Color.G * Dot) ;
    Color.B = (u8)((f32)Color.B * Dot) ;

    draw_Color( Color );
    draw_Vertex(A) ;
    draw_Vertex(B) ;
    draw_Vertex(C) ;
    
    draw_Vertex(C) ;
    draw_Vertex(D) ;
    draw_Vertex(A) ;
}

void draw_SolidBBox( const bbox&    BBox,
                      xcolor        Color,
                      const matrix4& L2W )
{
    vector3 P[8];
    P[0].X = BBox.Min.X;    P[0].Y = BBox.Min.Y;    P[0].Z = BBox.Min.Z;
    P[1].X = BBox.Min.X;    P[1].Y = BBox.Min.Y;    P[1].Z = BBox.Max.Z;
    P[2].X = BBox.Min.X;    P[2].Y = BBox.Max.Y;    P[2].Z = BBox.Min.Z;
    P[3].X = BBox.Min.X;    P[3].Y = BBox.Max.Y;    P[3].Z = BBox.Max.Z;
    P[4].X = BBox.Max.X;    P[4].Y = BBox.Min.Y;    P[4].Z = BBox.Min.Z;
    P[5].X = BBox.Max.X;    P[5].Y = BBox.Min.Y;    P[5].Z = BBox.Max.Z;
    P[6].X = BBox.Max.X;    P[6].Y = BBox.Max.Y;    P[6].Z = BBox.Min.Z;
    P[7].X = BBox.Max.X;    P[7].Y = BBox.Max.Y;    P[7].Z = BBox.Max.Z;

    // Setup light direction in local space
    matrix4 W2L ;
    W2L = L2W ;
    W2L.InvertSRT() ;
    vector3 LightDir = W2L.RotateVector(vector3(0,0,1)) ;
    LightDir.Normalize() ;

    draw_SetL2W(L2W) ;
    draw_Begin( DRAW_TRIANGLES, DRAW_USE_ALPHA );

    // Draw bbox
    draw_Quad(P[0], P[2], P[6], P[4], Color, LightDir) ; // F
    draw_Quad(P[1], P[5], P[7], P[3], Color, LightDir) ; // B
    draw_Quad(P[4], P[6], P[7], P[5], Color, LightDir) ; // R
    draw_Quad(P[0], P[1], P[3], P[2], Color, LightDir) ; // L
    draw_Quad(P[2], P[3], P[7], P[6], Color, LightDir) ; // T
    draw_Quad(P[0], P[4], P[5], P[1], Color, LightDir) ; // B
    
    draw_End();
}

//==============================================================================

void Render( void )
{
    s32 i ;

    //==---------------------------------------------------
    //  GRID, BOX AND MARKER
    //==---------------------------------------------------
    eng_Begin( "GridBoxMarker" );
    {
        draw_ClearL2W();
        draw_Grid( vector3(  -5000,   0,    -5000), 
                   vector3(10000,  0,    0), 
                   vector3(  0,   0, 10000), 
                   xcolor (  0,128,  0), 32 );
    }

    // Renders a wire axis where RGB=XYZ
    draw_Axis(50) ;

    eng_End();

    //==---------------------------------------------------
    //  Render rag doll
    //==---------------------------------------------------
    if( 1 )
    {
        eng_Begin( "Ragdoll" );

        // Draw bodys
        for (i = 0 ; i < s_NBodys ; i++)
        {
            // Lookup body
            body& Body = s_Bodys[i] ;

            // Get info
            matrix4 L2W = GetBodyL2W(Body.m_BodyID) ;

            // Draw body
            switch(dGeomGetClass(Body.m_GeomID))
            {
                case dBoxClass:
                {
                    dVector3    Size ;
                    dGeomBoxGetLengths(Body.m_GeomID, Size) ;
                    bbox        BBox ;
                    BBox.Min = vector3(-Size[0]/2, -Size[1]/2, -Size[2]/2) ;
                    BBox.Max = vector3(+Size[0]/2, +Size[1]/2, +Size[2]/2) ;
                    draw_SolidBBox(BBox, XCOLOR_WHITE, L2W) ;
                }
                break ;

                case dSphereClass:
                {
                    f32 Radius = dGeomSphereGetRadius(Body.m_GeomID) ;
                    draw_SetL2W(L2W) ;
                    draw_Sphere(vector3(0,0,0), Radius, XCOLOR_WHITE) ;
                }
                break ;
            }
        }

        // Draw joints
        draw_ClearL2W() ;
        for (i = 0 ; i < s_NJoints ; i++)
        {
            // Lookup joint
            joint& Joint = s_Joints[i] ;
            switch(dJointGetType(Joint.m_JointID))
            {
                case dJointTypeHinge:
                {
                    // Draw joint
                    dVector3 JP, JA ;
                    dJointGetHingeAnchor(Joint.m_JointID, JP) ;
                    dJointGetHingeAxis (Joint.m_JointID, JA) ;
                    vector3 P(JP[0], JP[1], JP[2]) ;
                    vector3 A(JA[0], JA[1], JA[2]) ;
                    draw_Sphere(P, 1, XCOLOR_RED) ;
                    draw_Line(P - (A*15), P + (A*15), XCOLOR_GREEN) ;
                    //radian A = dJointGetHingeAngle(Joint.m_JointID) ;
                    //draw_Label(vector3(P[0], P[1], P[2]), XCOLOR_WHITE, "%f", RAD_TO_DEG(A)) ;
                }
                break ;
            }                
        }

        eng_End();
    }
}

//==============================================================================

static void odeNearCallback(void *data, dGeomID Geom1, dGeomID Geom2)
{
    // Get bodies
    dBodyID Body1 = dGeomGetBody(Geom1) ;
    dBodyID Body2 = dGeomGetBody(Geom2) ;

    // Do not collide if bodies are connected by a joint
    if ( (Body1) && (Body2) && (dAreConnected(Body1, Body2)) )
        return ;

    // Geom1?
    if (Geom1) 
    {
        // Skip if no collision required
        body* pBody = (body*)dGeomGetData(Geom1) ;
        if ((pBody) && (!pBody->m_bCollision))
            return ;
    }

    // Geom2?
    if (Geom2) 
    {
        // Skip if no collision required
        body* pBody = (body*)dGeomGetData(Geom2) ;
        if ((pBody) && (!pBody->m_bCollision))
            return ;
    }

    // Any collision between bodies?
    dContactGeom Contacts[10] ;
    s32 NContacts = dCollide(Geom1, Geom2, 10, Contacts, sizeof(dContactGeom)) ;
    if (NContacts)
    {
        ASSERT(NContacts < 10) ;

        // Create contact joints
        for (s32 i = 0 ; i < NContacts ; i++)
        {
            // Fill in surface contact info
            dContact C ;
            C.surface.mode   = dContactBounce ;
            C.surface.mu     = 0.9f ;
            C.surface.bounce = 0.1f ;
            C.geom           = Contacts[i] ;

            // Create new joint
            dJointID JointID = dJointCreateContact(s_WorldID, s_ContactJointGroupID, &C) ;
            dJointAttach(JointID, Body1, Body2) ;
            s_NContactJoints++ ;
        }                
    }
}

//==============================================================================

void Advance( f32 Seconds )
{
    //Seconds = 1.0f / (60.0f*2) ;

    //
    // Advance the Logic 
    //

    static f32 SlowMo = 1;
    static f32 Pause=1;
    if( input_WasPressed( INPUT_KBD_P ) )
    {
        if( Pause == 1 ) Pause = 0;
        else Pause = 1;
    }

    if( input_WasPressed( INPUT_KBD_O ) )
    {
        if( SlowMo == 1 ) SlowMo = 0.25f;
        else SlowMo = 1;
    }

    f32 SingleStep = 0;
    if( input_WasPressed( INPUT_KBD_L ) )
    {
        SingleStep += 1/60.0f;
    }

//    x_printfxy( 50, 0, "Pause : %f ", Pause );
//    x_printfxy( 50, 1, "Pause : %f ", SlowMo );

    f32 V = Seconds * Pause * SlowMo + SingleStep;


    // Update ode world
    if (V != 0)
    //if (0)
    {
        // Perform collision
        s_CollTimer.Reset() ;
        s_CollTimer.Start() ;
        s_NContactJoints = 0 ;
        dSpaceCollide(s_SpaceID, NULL, &odeNearCallback);
        s_CollTimer.Stop() ;

        // Advance simulation
        s_SimTimer.Reset() ;
        s_SimTimer.Start() ;
        dWorldStep(s_WorldID, V) ;

        // Destroy contact joints
        dJointGroupEmpty(s_ContactJointGroupID) ;
        s_SimTimer.Stop() ;
    }
}


//==============================================================================

void AppMain( s32, char** )
{
    Initialize();
    xtimer Timer;
   
    while( TRUE )
    {
        f32 DeltaTime = Timer.TripSec() ;
        // Cap incase we go in the debugger
        if (DeltaTime > (1.0f / 60.0f))
            DeltaTime = 1.0f / 60.0f ;

        if( !HandleInput(DeltaTime) )
            break;

        eng_MaximizeViewport( View );
        eng_SetView         ( View, 0 );
        eng_ActivateView    ( 0 );

        // Logic
        Advance(DeltaTime) ;

        // Render
        Render();

        // Stats
        x_printfxy(1,1, "SimTime:  %f", s_SimTimer.ReadMs()) ;
        x_printfxy(1,2, "CollTime: %f", s_CollTimer.ReadMs()) ;
        x_printfxy(1,3, "Contacts: %d", s_NContactJoints) ;

        // DONE!
        eng_PageFlip();
    }

    Shutdown();
}

//==============================================================================
