//==============================================================================
//
//  StickBone.hpp
//
//==============================================================================

#ifndef __STICK_BONE_HPP__
#define __STICK_BONE_HPP__

//==============================================================================
// INCLUDES
//==============================================================================
#include "x_files.hpp"
#include "x_math.hpp"


//==============================================================================
// CLASSES
//==============================================================================

// Class that represents a stick bone
struct stick_bone
{
// Data
public:
    matrix4     m_L2W ;             // Local->World matrix
    vector3     m_Start;            // Start of bone
    vector3     m_End;              // End of bone
    const char* m_pName ;           // Name of stick bone
    xcolor      m_Color ;           // Debug color of stick bone

// Functions
public:
    // Constructor
         stick_bone() ;

    // Initialization
    void Init( const char* pName, xcolor Color ) ;

    // Updates stick bones axis and position. (normalizes axis)
    void Update( const vector3& BoneAxisX, 
                 const vector3& BoneAxisY, 
                 const vector3& BoneAxisZ, 
                 const vector3& BoneStart, 
                 const vector3& BoneEnd ) ;

    // Returns local->world matrix
    matrix4& GetL2W( void ) ;
    
#ifdef X_DEBUG
    
    // Renders the stick bone
    void Render( void ) ;

#endif
    
} PS2_ALIGNMENT(16) ;

//==============================================================================

inline
void stick_bone::Update( const vector3& BoneAxisX, 
                         const vector3& BoneAxisY, 
                         const vector3& BoneAxisZ, 
                         const vector3& BoneStart, 
                         const vector3& BoneEnd )
{
    // Setup rotation
    vector3 AxisX = BoneAxisX ;
    vector3 AxisY = BoneAxisY ;
    vector3 AxisZ = BoneAxisZ ;
    AxisX.Normalize() ;
    AxisY.Normalize() ;
    AxisZ.Normalize() ;
    m_L2W.SetColumns( AxisX, AxisY, AxisZ ) ;

    // Setup position
    // (See ragdoll.cpp optimization where the BindT = 0.0f)
    m_L2W.SetTranslation( BoneStart );

    // Keep end points
    m_Start = BoneStart ;
    m_End   = BoneEnd ;
}

//==============================================================================

inline
matrix4&  stick_bone::GetL2W( void )
{
    return m_L2W ;
}

//==============================================================================

#endif  // #ifndef __STICK_BONE_HPP__
