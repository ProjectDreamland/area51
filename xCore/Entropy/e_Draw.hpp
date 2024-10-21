//==============================================================================
//
//  e_Draw.hpp
//
//==============================================================================

#ifndef E_DRAW_HPP
#define E_DRAW_HPP

//==============================================================================
//
//  UVs in 3D are parametric.  UVs in 2D are pixels.
//
//  The index -1 can be used as a "separator" in index lists for strips.  
//  (A terminating -1 is optional.)
//  
//  The functions draw_Sprite and draw_SpriteUV are shortcuts for the primitive 
//  functions.  A compatible mode must be in effect.
//
//  Functions draw_SetL2W and draw_SetTexture could ASSERT that you are not
//  within draw_Begin and draw_End.
//
//  Some sort of clipping needs to be done.
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "e_view.hpp"
#include "x_bitmap.hpp"
#include "x_color.hpp"

//==============================================================================
//  DEFINITIONS
//==============================================================================

// Values to be used in the Flags parameter for function draw_Begin().

#define DRAW_2D             ( 1 <<  0 )     // Default: 3D
#define DRAW_TEXTURED       ( 1 <<  1 )     // Default: Not textured
#define DRAW_USE_ALPHA      ( 1 <<  2 )     // Default: Alpha disabled
#define DRAW_WIRE_FRAME     ( 1 <<  3 )     // Default: Solid/filled
#define DRAW_NO_ZBUFFER     ( 1 <<  4 )     // Default: Z-Buffer
#define DRAW_NO_ZWRITE      ( 1 <<  5 )     // Default: Z-Write enabled
#define DRAW_2D_KEEP_Z      ( 1 <<  6 )     // Default: all on same Z plane close to camera
#define DRAW_BLEND_ADD      ( 1 <<  7 )     // Default: Multiplicative (alpha) ADD and SUB mutually exclusive
#define DRAW_BLEND_SUB      ( 1 <<  8 )     // Default: Multiplicative (alpha) ADD and SUB mutually exclusive
#define DRAW_U_CLAMP        ( 1 <<  9 )     // Default: WRAP
#define DRAW_V_CLAMP        ( 1 << 10 )     // Default: WRAP
#define DRAW_KEEP_STATES	( 1 << 11 )		// Default: Off (doesn't set any render/texture-stage states)
#define DRAW_XBOX_NO_BEGIN  ( 1 << 12 )     // Default: Off
#define DRAW_XBOX_WRITE_A   ( 1 << 13 )     // Default: Off (writes into the frame-buffer alpha channel)

#define DRAW_CULL_NONE      ( 1 << 29 )     // Default: Cull CW
#define DRAW_CUSTOM_MODE    ( 1 << 30 )     // Default: Off
#define DRAW_HAS_NORMAL		( 1 << 31 )     // Default: has no normal

#define DRAW_UV_CLAMP       (DRAW_U_CLAMP | DRAW_V_CLAMP)

//==============================================================================
//  TYPES
//==============================================================================

enum draw_primitive
{
    DRAW_POINTS,     
    DRAW_LINES,      
    DRAW_LINE_STRIPS,      
    DRAW_TRIANGLES,  
    DRAW_TRIANGLE_STRIPS,  
    DRAW_QUADS,      
    DRAW_RECTS,
    DRAW_SPRITES,
};

//==============================================================================
//  FUNCTIONS
//==============================================================================

void    draw_Begin              ( draw_primitive Primitive, u32 Flags = 0 );
void    draw_End                ( void );
void    draw_ResetAfterException( void );

void    draw_SetL2W             ( const matrix4& L2W );
void    draw_ClearL2W           ( void );
void    draw_SetTexture         ( const xbitmap& Bitmap );
void    draw_SetTexture         ( void );
void    draw_DisableBilinear    ( void );
void    draw_EnableBilinear     ( void );
void    draw_SetZBias           ( s32 Bias );

#ifdef TARGET_XBOX
void    draw_DisableSatCompensation ( void );
void    draw_EnableSatCompensation  ( void );
#endif

void    draw_UV                 ( const vector2& UV );
void    draw_UV                 ( f32 U, f32 V );
                        
void    draw_Color              ( const xcolor & Color );
void    draw_Color              ( f32 R, f32 G, f32 B, f32 A = 1.0f );
                        
void    draw_Vertex             ( const vector3& Vertex );
void    draw_Vertex             ( f32 X, f32 Y, f32 Z );

void    draw_UVs                ( const vector2* pUVs,    s32 Count, s32 Stride = sizeof(vector2) );
void    draw_Colors             ( const xcolor*  pColors, s32 Count, s32 Stride = sizeof(xcolor ) );
void    draw_Verts              ( const vector3* pVerts,  s32 Count, s32 Stride = sizeof(vector3) );
                        
void    draw_Index              ( s32 Index );
void    draw_Execute            ( const s16* pIndices, s32 NIndices );

// Primitive must be DRAW_TRIANGLES
void    draw_OrientedQuad   (const vector3& Pos0,
                             const vector3& Pos1,
                             const vector2& UV0,
                             const vector2& UV1,
                             const xcolor &  Color0,
                             const xcolor &  Color1,
                                   f32      Radius );

// trapezoidal version
void    draw_OrientedQuad   (const vector3& Pos0,
                             const vector3& Pos1,
                             const vector2& UV0,
                             const vector2& UV1,
                             const xcolor &  Color0,
                             const xcolor &  Color1,
                                   f32      Radius0,
                                   f32      Radius1 );

// connected sequence
void    draw_OrientedStrand (const vector3* pPosData,
                                   s32      NumPts,
                             const vector2& UV0,
                             const vector2& UV1,
                             const xcolor &  Color0,
                             const xcolor &  Color1,
                                   f32      Radius );

// Primitive must be DRAW_SPRITES.
void    draw_Sprite     ( const vector3& Position,  // Hot spot (2D Left-Top), (3D Center)
                          const vector2& WH,        // (2D pixel W&H), (3D World W&H)
                          const xcolor &  Color );   // 

// Primitive must be DRAW_SPRITES.
void    draw_SpriteUV   ( const vector3& Position,  // Hot spot (2D Left-Top), (3D Center)
                          const vector2& WH,        // (2D pixel W&H), (3D World W&H)
                          const vector2& UV0,       // Upper Left   UV  [0.0 - 1.0]
                          const vector2& UV1,       // Bottom Right UV  [0.0 - 1.0]
                          const xcolor &  Color );   // 

// Primitive must be DRAW_SPRITES, 3D only.
void    draw_SpriteUV   ( const vector3& Position,  // Hot spot (3D Center)
                          const vector2& WH,        // (3D World W&H)
                          const vector2& UV0,       // Upper Left   UV  [0.0 - 1.0]
                          const vector2& UV1,       // Bottom Right UV  [0.0 - 1.0]
                          const xcolor &  Color,     //
                                radian   Rotate );  // 


//
// These are draw implementations of different debug objects
//

#if !defined( CONFIG_RETAIL ) || defined( TARGET_PC )

// Single call to render a single line
void    draw_Line       ( const vector3& P0,
                          const vector3& P1,
                                xcolor   Color = XCOLOR_WHITE );

// Renders a wire bbox 
void    draw_BBox       ( const bbox&    BBox,
                                xcolor   Color = XCOLOR_WHITE );

// Renders a volume given an two end points
void    draw_Volume     ( const vector3& P0, 
                          const vector3& P1, 
                                f32      Width,
                                f32      Height,
                                xcolor   Color = XCOLOR_WHITE );

// Renders a volume given a BBox
void    draw_Volume     ( const bbox&    BBox, 
                                xcolor   Color = XCOLOR_WHITE );

// Renders a wire sphere
void    draw_Sphere     ( const vector3& Pos,
                                f32      Radius,
                                xcolor   Color = XCOLOR_WHITE );

// Renders a wire or solid ngon
void    draw_NGon       ( const vector3* pPoint,
                                s32      NPoints,
                                xcolor   Color = XCOLOR_WHITE,
                                xbool    DoWire = TRUE );

// Renders always-visible solid rect on screen showing where point is in world
void    draw_Marker     ( const vector3& Pos,
                                xcolor   Color = XCOLOR_WHITE );

// Renders always-visible solid rect on screen showing where point is in world
void    draw_Point      ( const vector3& Pos,
                                xcolor   Color = XCOLOR_WHITE,
                                s32      Size  = 2 );

// Renders a wire pyramid extending along the edges of the frustum
void    draw_Frustum    ( const view&    View,
                                xcolor   Color = XCOLOR_RED,
                                f32      Dist = 100.0f );

// Renders a wire axis where RGB=XYZ
void    draw_Axis       (       f32      Size = 1.0f );

// Renders a wire grid starting at corner and stretching the lengths of the edges
void    draw_Grid       ( const vector3& Corner,
                          const vector3& Edge1,
                          const vector3& Edge2,
                                xcolor   Color = XCOLOR_WHITE, 
                                s32      NSubdivisions = 8 );

// Renders a text message centered on a 3D point if visible
void    draw_Label      ( const vector3& Pos,
                                xcolor   Color,
                          const char*    pFormatStr, ... );

// Single call to render a single rect
void    draw_Rect       ( const rect&    Rect,
                          xcolor         Color = XCOLOR_WHITE,
                          xbool          DoWire = TRUE);

#endif // !defined( CONFIG_RETAIL

// Single call to render a single rect
void    draw_Rect       ( const irect&   Rect,
                          xcolor         Color = XCOLOR_WHITE,
                          xbool          DoWire = TRUE);

// Single call to render a single rect
void    draw_GouraudRect( const irect&   Rect,
                          const xcolor & c1,
                          const xcolor & c2,
                          const xcolor & c3,
                          const xcolor & c4,
                          xbool          DoWire = TRUE,
                          xbool          DoAdditive = FALSE);


// Single call to set the z buffer to a value (1.0f = far clip, 0 = near clip)
void    draw_SetZBuffer   ( const irect& Rect, f32 Z ) ;

// Single call to clear z buffer to far clip
void    draw_ClearZBuffer ( const irect& Rect ) ;

// Single call to fill z buffer to near clip
void    draw_FillZBuffer  ( const irect& Rect ) ;


//==============================================================================
#endif // E_DRAW_HPP
//==============================================================================




